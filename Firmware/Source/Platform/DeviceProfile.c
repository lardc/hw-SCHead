﻿// -----------------------------------------
// Device profile
// ----------------------------------------

// Header
#include "DeviceProfile.h"
//
// Includes
#include "SysConfig.h"
#include "Global.h"
#include "DataTable.h"
#include "Controller.h"
#include "Constraints.h"
#include "ZwNCAN.h"
#include "ZwSCI.h"
#include "BCCIMaster.h"

// Types
//
typedef struct __EPState
{
	Int16U Size;
	Int16U ReadCounter;
        Int16U WriteCounter;
	Int16U LastReadCounter;
	pInt16U pDataCounter;
	pInt16U Data;
} EPState, *pEPState;
//
typedef struct __EPStates
{
	EPState EPs[EP_COUNT];
        EPState WriteEPs[EP_WRITE_COUNT];
} EPStates, *pEPStates;


// Variables
//
SCCI_Interface DEVICE_RS232_Interface;
BCCI_Interface DEVICE_CAN_Interface;
BCCIM_Interface MASTER_DEVICE_CAN_Interface;
//
static SCCI_IOConfig RS232_IOConfig;
static BCCI_IOConfig CAN_IOConfig;
static BCCI_IOConfig CAN_Master_IOConfig;
static xCCI_ServiceConfig X_ServiceConfig;
static EPStates RS232_EPState, CAN_EPState;
static Boolean UnlockedForNVWrite = FALSE;
static xCCI_FUNC_CallbackAction ControllerDispatchFunction;
//
static Boolean* MaskChangesFlag;


// Forward functions
//
static Boolean DEVPROFILE_Validate16(Int16U Address, Int16U Data);
static Boolean DEVPROFILE_DispatchAction(Int16U ActionID, pInt16U UserError);
static void DEVPROFILE_FillWRPartDefault();


// Functions
//
void DEVPROFILE_Init(xCCI_FUNC_CallbackAction SpecializedDispatch, Boolean* MaskChanges)
{
	// Save values
        ControllerDispatchFunction = SpecializedDispatch;
	MaskChangesFlag = MaskChanges;

	// Init interface
	RS232_IOConfig.IO_SendArray16 = &ZwSCI_SendArray16;
	RS232_IOConfig.IO_ReceiveArray16 = &ZwSCI_ReceiveArray16;
	RS232_IOConfig.IO_GetBytesToReceive = &ZwSCI_GetBytesToReceive;
	RS232_IOConfig.IO_ReceiveByte = &ZwSCI_ReceiveChar;
	//
	CAN_IOConfig.IO_SendMessage = &NCAN_SendMessage;
	CAN_IOConfig.IO_SendMessageEx = &NCAN_SendMessageEx;
	CAN_IOConfig.IO_GetMessage = &NCAN_GetMessage;
	CAN_IOConfig.IO_IsMessageReceived = &NCAN_IsMessageReceived;
	CAN_IOConfig.IO_ConfigMailbox = &NCAN_ConfigMailbox;
	//
	CAN_Master_IOConfig.IO_SendMessage = &NCAN_SendMessage;
	CAN_Master_IOConfig.IO_SendMessageEx = &NCAN_SendMessageEx;
	CAN_Master_IOConfig.IO_GetMessage = &NCAN_GetMessage;
	CAN_Master_IOConfig.IO_IsMessageReceived = &NCAN_IsMessageReceived;
	CAN_Master_IOConfig.IO_ConfigMailbox = &NCAN_ConfigMailbox;

	// Init service
	X_ServiceConfig.UserActionCallback = &DEVPROFILE_DispatchAction;
	X_ServiceConfig.ValidateCallback16 = &DEVPROFILE_Validate16;

	// Init interface driver
	SCCI_Init(&DEVICE_RS232_Interface, &RS232_IOConfig, &X_ServiceConfig, (pInt16U)DataTable,
			  DATA_TABLE_SIZE, SCCI_TIMEOUT_TICKS, &RS232_EPState);
	BCCI_Init(&DEVICE_CAN_Interface, &CAN_IOConfig, &X_ServiceConfig, (pInt16U)DataTable,
			DATA_TABLE_SIZE, &CAN_EPState);
	BCCIM_Init(&MASTER_DEVICE_CAN_Interface, &CAN_Master_IOConfig, BCCIM_TIMEOUT_TICKS, &CONTROL_TimeCounter);

	// Set write protection
	SCCI_AddProtectedArea(&DEVICE_RS232_Interface, DATA_TABLE_WP_START, DATA_TABLE_SIZE - 1);
	BCCI_AddProtectedArea(&DEVICE_CAN_Interface, DATA_TABLE_WP_START, DATA_TABLE_SIZE - 1);
}
// ----------------------------------------

void DEVPROFILE_ProcessRequests()
{
  // Handle interface requests
  SCCI_Process(&DEVICE_RS232_Interface, CONTROL_TimeCounter, *MaskChangesFlag);
  // Handle interface requests
  BCCI_Process(&DEVICE_CAN_Interface, *MaskChangesFlag);
}
// ----------------------------------------


void DEVPROFILE_ResetControlSection()
{
	DT_ResetWRPart(&DEVPROFILE_FillWRPartDefault);
}
// ----------------------------------------

Int32U DEVPROFILE_ReadValue32(pInt16U pTable, Int16U Index)
{
	return pTable[Index] | (((Int32U)(pTable[Index + 1])) << 16);
}
// ----------------------------------------

void DEVPROFILE_WriteValue32(pInt16U pTable, Int16U Index, Int32U Data)
{
	pTable[Index] = Data & 0x0000FFFF;
	pTable[Index + 1] = Data >> 16;
}

// ----------------------------------------

static void DEVPROFILE_FillWRPartDefault()
{
	Int16U i;

	// Write default values to data table
	for(i = 0; i < (DATA_TABLE_WP_START - DATA_TABLE_WR_START); ++i)
		DataTable[DATA_TABLE_WR_START + i] = VConstraint[i].Default;
}
// ----------------------------------------

void DEVPROFILE_FillNVPartDefault(void)
{
	Int16U i;

	// Write default values to data table
	for(i = 0; i < DATA_TABLE_NV_SIZE; ++i)
		DataTable[DATA_TABLE_NV_START + i] = NVConstraint[i].Default;
}
// ----------------------------------------

static Boolean DEVPROFILE_Validate16(Int16U Address, Int16U Data)
{
	if(ENABLE_LOCKING && !UnlockedForNVWrite && (Address < DATA_TABLE_WR_START))
		return FALSE;

	if(Address < DATA_TABLE_WR_START)
	{
		if(Data < NVConstraint[Address - DATA_TABLE_NV_START].Min || Data > NVConstraint[Address - DATA_TABLE_NV_START].Max)
			return FALSE;
	}
	else if(Address < DATA_TABLE_WP_START)
	{
		if(Data < VConstraint[Address - DATA_TABLE_WR_START].Min || Data > VConstraint[Address - DATA_TABLE_WR_START].Max)
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

static Boolean DEVPROFILE_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_RESET_FOR_PROGRAMMING:
			{
				volatile pInt16U FlagPointer = (volatile pInt16U)ADDRESS_FLAG_REGISTER;
				*FlagPointer = FLAG_RESET_FOR_PROG;
				break;
			}

		case ACT_SAVE_DT_TO_FLASH:
			{
				DT_SaveNVPartToEPROM();
				break;
			}

		case ACT_RESET_DEVICE:
			{
				volatile pInt16U FlagPointer = (volatile pInt16U)ADDRESS_FLAG_REGISTER;
				*FlagPointer = FLAG_RESET;
				break;
			}

		default:
			return (ControllerDispatchFunction) ? ControllerDispatchFunction(ActionID, UserError) : FALSE;
	}
	return TRUE;
}
// ----------------------------------------

void DEVPROFILE_InitEPService(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas)
{
  Int16U i;

  for(i = 0; i < EP_COUNT; ++i)
  {
    RS232_EPState.EPs[i].Size = Sizes[i];
    RS232_EPState.EPs[i].pDataCounter = Counters[i];
    RS232_EPState.EPs[i].Data = Datas[i];

    CAN_EPState.EPs[i].Size = Sizes[i];
    CAN_EPState.EPs[i].pDataCounter = Counters[i];
    CAN_EPState.EPs[i].Data = Datas[i];

    RS232_EPState.EPs[i].ReadCounter = RS232_EPState.EPs[i].LastReadCounter = 0;
    CAN_EPState.EPs[i].ReadCounter = CAN_EPState.EPs[i].LastReadCounter = 0;

    SCCI_RegisterReadEndpoint16(&DEVICE_RS232_Interface, Indexes[i], &DEVPROFILE_CallbackReadX);
    BCCI_RegisterReadEndpoint16(&DEVICE_CAN_Interface, Indexes[i], &DEVPROFILE_CallbackReadX);
  }
}
// ----------------------------------------

Int16U DEVPROFILE_CallbackReadX(Int16U Endpoint, pInt16U *Buffer, Boolean Streamed,
									   Boolean RepeatLastTransmission, void *EPStateAddress, Int16U MaxNonStreamSize)
{
	Int16U pLen;
	pEPState epState;
	pEPStates epStates = (pEPStates)EPStateAddress;

	// Validate pointer
	if(!epStates)
		return 0;

	// Get endpoint
	epState = &epStates->EPs[Endpoint - 1];

	// Handle transmission repeat
	if(RepeatLastTransmission)
		epState->ReadCounter = epState->LastReadCounter;

	// Write possible content reference
	*Buffer = epState->Data + epState->ReadCounter;

	// Calculate content length
	if(*(epState->pDataCounter) < epState->ReadCounter)
		pLen = 0;
	else
		pLen = *(epState->pDataCounter) - epState->ReadCounter;

	if(!Streamed)
		pLen = (pLen > MaxNonStreamSize) ? MaxNonStreamSize : pLen;

	// Update content state
	epState->LastReadCounter = epState->ReadCounter;
	if(!Streamed)
	{
		if(pLen == 0)
			epState->ReadCounter = 0;
		else
			epState->ReadCounter += pLen;
	}

	return pLen;
}
// ----------------------------------------
void DEVPROFILE_ResetEPReadState()
{
  Int16U i;

  for(i = 0; i < EP_COUNT; ++i)
  {
    RS232_EPState.EPs[i].ReadCounter = 0;
    CAN_EPState.EPs[i].ReadCounter = 0;
    RS232_EPState.EPs[i].LastReadCounter = 0;
    CAN_EPState.EPs[i].LastReadCounter = 0;
  }
}
// ----------------------------------------

void DEVPROFILE_ResetScopes(Int16U ResetPosition)
{
  Int16U i;

  for(i = 0; i < EP_COUNT; ++i)
  {
    *(RS232_EPState.EPs[i].pDataCounter) = ResetPosition;
    *(CAN_EPState.EPs[i].pDataCounter) = ResetPosition;

    MemZero16(RS232_EPState.EPs[i].Data, RS232_EPState.EPs[i].Size);
    MemZero16(CAN_EPState.EPs[i].Data, CAN_EPState.EPs[i].Size);
  }
}
// ----------------------------------------

void DEVPROFILE_InitEPWriteService(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas)
{
  Int16U i;

  for(i = 0; i < EP_WRITE_COUNT; ++i)
  {
    RS232_EPState.WriteEPs[i].Size = Sizes[i];
    RS232_EPState.WriteEPs[i].pDataCounter = Counters[i];
    RS232_EPState.WriteEPs[i].Data = Datas[i];

    CAN_EPState.WriteEPs[i].Size = Sizes[i];
    CAN_EPState.WriteEPs[i].pDataCounter = Counters[i];
    CAN_EPState.WriteEPs[i].Data = Datas[i];

    RS232_EPState.WriteEPs[i].WriteCounter = 0;
    CAN_EPState.WriteEPs[i].WriteCounter = 0;

    SCCI_RegisterWriteEndpoint16(&DEVICE_RS232_Interface, Indexes[i], &DEVPROFILE_CallbackWriteX);
    BCCI_RegisterWriteEndpoint16(&DEVICE_CAN_Interface, Indexes[i], &DEVPROFILE_CallbackWriteX);
  }
}
// ----------------------------------------

Boolean DEVPROFILE_CallbackWriteX(Int16U Endpoint, pInt16U Buffer, Boolean Streamed, Int16U Length, void *EPStateAddress)
{
  pEPState epState;
  pEPStates epStates = (pEPStates)EPStateAddress;

  // Validate pointer
  if(!epStates)
          return FALSE;

  // Get endpoint
  epState = &epStates->WriteEPs[Endpoint - 1];

  // Check for free space
  if (epState->Size < Length + *(epState->pDataCounter))
          return FALSE;
  else
  {
          MemCopy16(Buffer, epState->Data + *(epState->pDataCounter), Length);
          *(epState->pDataCounter) += Length;
          return TRUE;
  }
}
// ----------------------------------------
// No more

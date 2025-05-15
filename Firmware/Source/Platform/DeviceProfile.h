// -----------------------------------------
// Device profile
// ----------------------------------------

#ifndef __DEV_PROFILE_H
#define __DEV_PROFILE_H

// Include
#include "stdinc.h"
//
#include "SCCISlave.h"
#include "BCCISlave.h"
#include "BCCIMaster.h"
#include "Controller.h"

extern BCCIM_Interface MASTER_DEVICE_CAN_Interface;


// Functions
//
void DEVPROFILE_FillNVPartDefault(void);
//
// Initialize device profile engine
void DEVPROFILE_Init(xCCI_FUNC_CallbackAction SpecializedDispatch, Boolean *MaskChanges);
// Initialize endpoint service
void DEVPROFILE_InitEPService(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas);
// Process user interface requests
void DEVPROFILE_ProcessRequests();
// Reset EP counters
void DEVPROFILE_ResetEPReadState();
// Reset user control (WR) section of data table
void DEVPROFILE_ResetControlSection();
// Update diagnostic registers
void DEVPROFILE_UpdateCANDiagStatus();

// Read 32-bit value from data table
Int32U DEVPROFILE_ReadValue32(pInt16U pTable, Int16U Index);
// Write 32-bit value to data table
void DEVPROFILE_WriteValue32(pInt16U pTable, Int16U Index, Int32U Data);
 Int16U DEVPROFILE_CallbackReadX(Int16U Endpoint, pInt16U *Buffer, Boolean Streamed,
									   Boolean RepeatLastTransmission, void *EPStateAddress, Int16U MaxNonStreamSize);

void DEVPROFILE_ResetEPReadState();
void DEVPROFILE_ResetScopes(Int16U ResetPosition);
void DEVPROFILE_InitEPWriteService(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas);
 Boolean DEVPROFILE_CallbackWriteX(Int16U Endpoint, pInt16U Buffer, Boolean Streamed, Int16U Length, void *EPStateAddress);
#endif // __DEV_PROFILE_H

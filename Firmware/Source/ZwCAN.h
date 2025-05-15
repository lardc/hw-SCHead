// -----------------------------------------
// CAN high-level interface functions
// ----------------------------------------

#ifndef __ZW_CAN_H
#define __ZW_CAN_H


// Include
//
#include "stm32f30x.h"
#include "stdinc.h"
#include "LocalDefs.h"
#include "ZwRCC.h"
#include "Global.h"
#include "ZwWWDG.h"
#include "BCCITypes.h"


// Совместимость TrueSTUDIO и IAR
#ifndef CAN1
	#define CAN1 CAN
#endif


// Definitions
//
#define MAILBOXmax					36		// Количество MailBox

// CAN commands ids
#define CAN_ID_W_16					10
#define CAN_ID_W_32					14
#define CAN_ID_R_16					20
#define CAN_ID_R_32					24
#define CAN_ID_CALL					50
#define CAN_ID_ERR					61
#define CAN_ID_WB_16				30
#define CAN_ID_RB_16				40

// Mailbox configuration flags
#define ZW_CAN_RTRAnswer			0x01
#define ZW_CAN_MBProtected			0x02
#define ZW_CAN_UseExtendedID		0x04

// Parameter for ZwCANx_ConfigMailbox
#define ZW_CAN_RECEIVE_MAILBOX		TRUE
#define ZW_CAN_TRANSMIT_MAILBOX		FALSE
#define ZW_CAN_MSG_LENGTH			8
#define ZW_CAN_NO_PRIORITY			0
#define ZW_CAN_STRONG_MATCH			0


// Types
//
typedef struct __CANMailboxItem				// Store information about mailbox
{
	uint32_t MsgID;
	uint32_t Dir;
	uint32_t DataLen;
	uint32_t DWORD1;
	uint32_t DWORD0;
	uint32_t DataReady;
	
} CANMailboxItem;


// Variables
//
extern uint32_t MasterAddress;


// Functions
//
void CAN_DeInit(void);
void CAN_Init(uint32_t SystemClock, uint32_t Baudrate);
void CAN_Init1(uint32_t SystemClock, uint32_t Baudrate);
void CAN_Filter(uint32_t id);
void CAN_Start(void);
void CAN_WaitReady(void);
void ZwCAN_GetMessage(Int16U mBox, pCANMessage Data);
Boolean ZwCAN_IsMessageReceived(Int16U mBox, pBoolean pMessageLost);
Boolean ZwCAN_SendMessageEx(Int16U mBox, pCANMessage Data, Boolean AlterMessageID, Boolean AlterMessageLength);
void ZwCAN_SendMessageEx_Void(Int16U mBox, pCANMessage Data, Boolean AlterMessageID, Boolean AlterMessageLength);
Boolean ZwCAN_SendMessage(Int16U mBox, pCANMessage Data);
void ZwCAN_SendMessage_Void(Int16U mBox, pCANMessage Data);
void ZwCAN_ConfigMailbox(Int16U mBox, Int32U MsgID, Boolean Dir, Int16U DataLen, Int32U Flags,
	Int16U TransmitPriority, Int32U LAM);
void ZwCAN_ConfigMailbox_BCCI(Int16U mBox, Int32U MsgID, Int16U DataLen);
void ZwCAN_Init(Int32U CANx, uint32_t SystemFrequency, uint32_t Baudrate);
void ZwCAN_RecieveData(void);
void ZwCAN_MessageReceivedReset(Int16U mBox);
void ZwCAN_RecieveFlagReset(CAN_TypeDef* CANx);
bool ZwCAN_RecieveCheck(CAN_TypeDef* CANx);
CANMailboxItem ZwCAN_GetMailbox(Int16U mBox);
uint32_t WordSwap(uint32_t Data, uint8_t DataLen);


#endif // __ZW_CAN_H

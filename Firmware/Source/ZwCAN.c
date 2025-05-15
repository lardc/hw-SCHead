// Includes
//
#include "ZwCAN.h"
#include "stm32f30x.h"
#include "BCCIxParams.h"


// Variables
//
CANMailboxItem MailBox[MAILBOXmax];
uint32_t MasterAddress = 0;


// Functions
//
// Сброс модуля CAN
void CAN_DeInit(void)
{
	RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_CAN1RST;
}
//-----------------------------------------------

// Инициализация CAN
void CAN_Init(uint32_t SystemFrequency, uint32_t Baudrate)
{
	static float BitTime, SystemClockTime, tq, tbs1, tbs2;
	static uint32_t TS1, TS2, BRP;

	SystemClockTime = 1.0 / SystemFrequency;
	BitTime = 1.0 / Baudrate;
	tq = BitTime / 10;
	tbs1 = tq * 7;
	tbs2 = tq * 2;

	BRP = (uint16_t)(tq / SystemClockTime - 1);
	TS1 = (uint16_t)(tbs1 / tq - 1);
	TS2 = (uint16_t)(tbs2 / tq - 1);

    // Без автоматической ретрансляции, запрос инициализации
	CAN1->MCR = (/*CAN_MCR_NART |*/ CAN_MCR_INRQ);
    // Разрешение прерывания при приеме
	CAN1->IER = CAN_IER_FMPIE0;

	//Настройка скорости CAN
	CAN1->BTR = 0x0;
	CAN1->BTR |= TS2 << 20; // 4 TS2
	CAN1->BTR |= TS1 << 16; // 9 TS1
	CAN1->BTR |= BRP;       // 3 BRP
}
//-----------------------------------------------

// Инициализация фильтров CAN
void CAN_Filter(uint32_t id)
{
	CAN1->FMR |= CAN_FMR_FINIT;			//Разблокировать инициализацию фильтров
	CAN1->FA1R &= ~(uint32_t)(1);		//Деактивация фильтра 0

	CAN1->FS1R |= (uint32_t)(1);		// 32 битный режим регистра фильтров
	CAN1->FM1R &= ~(uint32_t)(1);		//Режим list
	CAN1->FFA1R &= ~(uint32_t)(1);		//Сообщение сохраняется в FIFO 0

	CAN1->sFilterRegister[0].FR1 = (uint32_t)(id << 21); // ID
	CAN1->sFilterRegister[0].FR2 = (uint32_t)(id << 21); // ID

	CAN1->FA1R |= (uint32_t)(1);		//Активировать фильтр 0

	CAN1->FMR &= ~CAN_FMR_FINIT;    	//Заблокировать инициализацию фильтров
}
//-----------------------------------------------

// Старт модуля CAN
void CAN_Start(void)
{
	CAN1->BTR &= ~(CAN_BTR_SILM | CAN_BTR_LBKM);
	CAN1->MCR &= ~CAN_MCR_INRQ;         //Нормальный режим работы
	while (CAN1->MSR & CAN_MCR_INRQ);   //Ожидание переключения

	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}
//-----------------------------------------------

// Ожидание готовности CAN
void CAN_WaitReady(void)
{
	while (!(CAN1->TSR & CAN_TSR_TME0));
}
//-----------------------------------------------

void ZwCAN_RecieveData(void)
{
	uint32_t MsgID = CAN1->sFIFOMailBox[0].RIR >> 3;

	if (CAN_SLAVE_DEVICE)
	{
		MasterAddress = (MsgID >> MASTER_DEV_ADDR_MPY) & MASTER_ADDRESS_MASK;
		MsgID &= ~(MASTER_ADDRESS_MASK << MASTER_DEV_ADDR_MPY);
	}
	else
	{
		uint32_t MasterAddressTemp = (MsgID >> MASTER_DEV_ADDR_MPY) & MASTER_ADDRESS_MASK;

		if (MasterAddressTemp == MASTER_DEVICE_CAN_ADDRESS)
			MsgID &= MASTER_ID_MASK;	// Данные идут от slave
		else
		{
			MasterAddress = MasterAddressTemp;
			MsgID &= SLAVE_ID_MASK;		// Данные идут от master
		}
	}

	uint8_t MailBoxCNT = 1;
	while ((MsgID != MailBox[MailBoxCNT].MsgID) && (MailBoxCNT < MAILBOXmax))
	{
		MailBoxCNT++;
	}	// Ищем свой маилбокс

	if (MailBoxCNT < MAILBOXmax)
	{
		// Сохраняем младший байт
		MailBox[MailBoxCNT].DWORD0 = WordSwap(CAN1->sFIFOMailBox[0].RDLR, MailBox[MailBoxCNT].DataLen);

		// Сохраняем старший байт
		MailBox[MailBoxCNT].DWORD1 = WordSwap(CAN1->sFIFOMailBox[0].RDHR, MailBox[MailBoxCNT].DataLen);

		MailBox[MailBoxCNT].DataReady = 1;
	}
	else
		MasterAddress = 0;
}
//-----------------------------------------------

void ZwCAN_Init(Int32U CANx, uint32_t SystemFrequency, uint32_t Boudrate)
{
	RCC_CAN_Clk_EN(CANx);
	CAN_DeInit();
	CAN_Init(SystemFrequency, Boudrate);
	CAN_Filter(0);
	CAN_Start();
	CAN_WaitReady();
}
//-----------------------------------------------

void ZwCAN_ConfigMailbox(Int16U mBox, Int32U MsgID, Boolean Dir, Int16U DataLen, Int32U Flags,
	Int16U TransmitPriority, Int32U LAM)
{
	MailBox[mBox].MsgID = MsgID;
	MailBox[mBox].Dir = Dir;
	MailBox[mBox].DataLen = DataLen;
}
//-----------------------------------------------

Boolean ZwCAN_SendMessage(Int16U mBox, pCANMessage Data)
{
	uint32_t TimeOut = 0, lr, hr, tir, len;

	tir = (MailBox[mBox].MsgID | (MasterAddress << MASTER_DEV_ADDR_MPY)) << 3;
	tir |= 1 << 2;
	CAN1->sTxMailBox[0].TIR = tir;

	len = MailBox[mBox].DataLen;
	CAN1->sTxMailBox[0].TDTR = len;

	lr = WordSwap(Data->HIGH.WORD.WORD_0, 2) | WordSwap(Data->HIGH.WORD.WORD_1, 2) << 16;
	hr = WordSwap(Data->LOW.WORD.WORD_2, 2) | WordSwap(Data->LOW.WORD.WORD_3, 2) << 16;
	CAN1->sTxMailBox[0].TDLR = lr;
	CAN1->sTxMailBox[0].TDHR = hr;

	CAN1->sTxMailBox[0].TIR |= 1; // TX

	while ((!(CAN1->TSR & CAN_TSR_TME0)) && (TimeOut < 1000))
		TimeOut++;

	return true;
}
//-----------------------------------------------

Boolean ZwCAN_SendMessageEx(Int16U mBox, pCANMessage Data, Boolean AlterMessageID, Boolean AlterMessageLength)
{
	uint32_t TimeOut = 0;

	CAN1->sTxMailBox[0].TIR = Data->MsgID.all << 3;
	CAN1->sTxMailBox[0].TIR |= 1 << 2; // IDE
	CAN1->sTxMailBox[0].TDTR = Data->DLC; // Data->DLC

	CAN1->sTxMailBox[0].TDLR
		= WordSwap(Data->HIGH.WORD.WORD_0, 2) | WordSwap(Data->HIGH.WORD.WORD_1, 2) << 16;
	CAN1->sTxMailBox[0].TDHR
		= WordSwap(Data->LOW.WORD.WORD_2, 2) | WordSwap(Data->LOW.WORD.WORD_3, 2) << 16;

	CAN1->sTxMailBox[0].TIR |= 1; // TX

	while ((!(CAN1->TSR & CAN_TSR_TXOK0)) && (TimeOut < 1000))
		TimeOut++;

	return true;
}
//-----------------------------------------------

Boolean ZwCAN_IsMessageReceived(Int16U mBox, pBoolean pMessageLost)
{
	return MailBox[mBox].DataReady;
}
//-----------------------------------------------

void ZwCAN_MessageReceivedReset(Int16U mBox)
{
	MailBox[mBox].DataReady = 0;
}
//-----------------------------------------------

CANMailboxItem ZwCAN_GetMailbox(Int16U mBox)
{
	return MailBox[mBox];
}
//-----------------------------------------------

void ZwCAN_GetMessage(Int16U mBox, pCANMessage Data)
{
	Data->DLC = MailBox[mBox].DataLen;
	Data->MsgID.all = MailBox[mBox].MsgID & 0xff;

	switch (Data->MsgID.all)
	{
		case CAN_ID_R_16:
		{
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case (CAN_ID_R_16 + 1):
		{
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case CAN_ID_W_16:
		{
			Data->HIGH.WORD.WORD_0 = (Int16U)((MailBox[mBox].DWORD0) >> 16);
			Data->HIGH.WORD.WORD_1 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case CAN_ID_R_32:
		{
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case CAN_ID_W_32:
		{
			Data->LOW.WORD.WORD_2 = (Int16U)(MailBox[mBox].DWORD1 >> 16);
			Data->LOW.WORD.WORD_3 = (Int16U)(MailBox[mBox].DWORD1);
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0 >> 16);
			Data->HIGH.WORD.WORD_1 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case CAN_ID_CALL:
		{
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case CAN_ID_WB_16:
		{
			Data->LOW.WORD.WORD_2 = (Int16U)(MailBox[mBox].DWORD1 >> 16);
			Data->LOW.WORD.WORD_3 = (Int16U)(MailBox[mBox].DWORD1);
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0 >> 16);
			Data->HIGH.WORD.WORD_1 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
		case CAN_ID_RB_16:
		{
			Data->HIGH.WORD.WORD_1 = (Int16U)(MailBox[mBox].DWORD0 >> 16);
			Data->HIGH.WORD.WORD_0 = (Int16U)(MailBox[mBox].DWORD0);
			break;
		}
	}
}
//-----------------------------------------------

void ZwCAN_RecieveFlagReset(CAN_TypeDef* CANx)
{
	CANx->RF0R |= CAN_RF0R_RFOM0;
}
//-----------------------------------------------

bool ZwCAN_RecieveCheck(CAN_TypeDef* CANx)
{
	return (CANx->RF0R & CAN_RF0R_FMP0);
}
//-----------------------------------------------

// Word swap
uint32_t WordSwap(uint32_t Data, uint8_t DataLen)
{
	uint32_t result = 0;

	switch (DataLen)
	{
		case 2:
		{
			result |= (Data & 0x000000ff) << 8;
			result |= (Data & 0x0000ff00) >> 8;
			break;
		}
		case 3:
		{
			result |= (Data & 0x000000ff) << 16;
			result |= (Data & 0x00ff0000) >> 16;
			break;
		}
		case 4:
		{
			result |= (Data & 0x000000ff) << 24;
			result |= (Data & 0x0000ff00) << 8;
			result |= (Data & 0x00ff0000) >> 8;
			result |= (Data & 0xff000000) >> 24;
			break;
		}
	}
	return result;
}
//-----------------------------------------------

void ZwCAN_SendMessage_Void(Int16U mBox, pCANMessage Data)
{
    ZwCAN_SendMessage(mBox, Data);
}
//-----------------------------------------------
void ZwCAN_SendMessageEx_Void(Int16U mBox, pCANMessage Data, Boolean AlterMessageID, Boolean AlterMessageLength)
{
    ZwCAN_SendMessageEx(mBox, Data, AlterMessageID, AlterMessageLength);
}
//-----------------------------------------------
void ZwCAN_ConfigMailbox_BCCI(Int16U mBox, Int32U MsgID, Int16U DataLen)
{
	ZwCAN_ConfigMailbox(mBox, MsgID, ZW_CAN_RECEIVE_MAILBOX, DataLen, 0, 0, 0);
}
//-----------------------------------------------

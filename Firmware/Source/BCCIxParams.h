// -----------------------------------------
// Parameters of BCCI interface
// ----------------------------------------

#ifndef __BCCI_PARAMS_H
#define __BCCI_PARAMS_H

#include "xCCIParams.h"

// Constants
//
#define CAN_FUNC_MASK				0xFFFFFC00

// Конфигурация Slave
//
#define CAN_SLAVE_NID				8				// Node ID устройства
#define CAN_SLAVE_NID_MASK			0x0003FC00		// Маска Slave node ID
#define CAN_SLAVE_NID_MPY			10				// Смещение адреса Slave

// Конфигурация Master
//
#define CAN_MASTER_NID				8				// Node ID устройства
#define CAN_MASTER_NID_MASK			0x03FC0000		// Маска Master node ID
#define CAN_MASTER_NID_MPY			6				// Смещение адреса Master

// CAN фильтр
//
#define CAN_SLAVE_FILTER_ID			(CAN_SLAVE_NID << CAN_SLAVE_NID_MPY)
#define CAN_MASTER_FILTER_ID		(CAN_MASTER_NID << CAN_MASTER_NID_MPY)


//Параметры для старой библиотеки CAN
#define CAN_ACCEPTANCE_MASK			0xFFFFFC00

#define CAN_SLAVE_DEVICE			FALSE
//
#define MASTER_DEVICE_CAN_ADDRESS	        8
#define MASTER_DEV_ADDR_MPY			6
//
#define SLAVE_DEVICE_CAN_ADDRESS	        8
#define SLAVE_DEV_ADDR_MPY			10

#define MASTER_ID_MASK				0x03FF
#define SLAVE_ID_MASK				0xFC3F	//0x003F
#define MASTER_ADDRESS_MASK			0x000F

#endif // __BCCI_PARAMS_H

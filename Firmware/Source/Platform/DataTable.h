﻿// -----------------------------------------
// Device data table
// ----------------------------------------

#ifndef __DATA_TABLE_H
#define __DATA_TABLE_H

// Include
#include "stdinc.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"
#include "DeviceProfile.h"


// Constants
//
#define DATA_TABLE_SIZE			300
#define DATA_TABLE_NV_SIZE		64
//
#define DATA_TABLE_NV_START		0
#define DATA_TABLE_WR_START		64
#define DATA_TABLE_WP_START		96
#define DATA_TABLE_FWINF_START	256

/*
 * DATA TABLE START 				------------- 0
 * 			[NON-VOLATILE AREA]
 *		END OF NON-VOLATILE AREA	------------- 63
 * 		START OF READ/WRITE AREA	------------- 64
 * 			[VOLATILE AREA]
 *		END OF READ/WRITE AREA		------------- 95
 * 		START OF READ-ONLY AREA		------------- 96
 * 			[VOLATILE R-O AREA]
 *		END OF READ-ONLY AREA		------------- 255
 * 		START OF READ-ONLY FW INFO AREA	--------- 256
 * 			[VOLATILE R-O AREA]
 * 		END OF READ-ONLY FW INFO AREA	--------- 300
 * DATA TABLE END 					------------- [300]
 */


// Types
//
typedef void (*FUNC_SetDefaultValues)();
typedef void (*FUNC_EPROM_WriteValues)(Int32U EPROMAddress, pInt16U Buffer, Int16U BufferSize);
typedef void (*FUNC_EPROM_ReadValues)(Int32U EPROMAddress, pInt16U Buffer, Int16U BufferSize);
//
typedef struct __EPROMServiceConfig
{
	FUNC_EPROM_WriteValues WriteService;
	FUNC_EPROM_ReadValues ReadService;
} EPROMServiceConfig, *pERPOMServiceConfig;


// Variables
//
extern volatile Int16U DataTable[DATA_TABLE_SIZE];


// Functions
//
// Initialize data table
void DT_Init(EPROMServiceConfig EPROMService, Boolean NoRestore);
// Restore part of data table from non-volatile EPROM memory
void DT_RestoreNVPartFromEPROM();
// Save part of data table to non-volatile EPROM memory
void DT_SaveNVPartToEPROM();
// Reset non-volatile part of data table
void DT_ResetNVPart(FUNC_SetDefaultValues SetFunc);
// Reset volatile read-write part of data table
void DT_ResetWRPart(FUNC_SetDefaultValues SetFunc);
void DT_ResetNVPartToDefault(void);
void DT_SaveFirmwareInfo(Int16U SlaveNID, Int16U MasterNID);
#endif // __DATA_TABLE_H

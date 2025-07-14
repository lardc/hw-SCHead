// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
//
#include "DataTable.h"
#include "Global.h"

#define PULSE_DURATION_MIN 				10000  // в мкс
#define PULSE_DURATION_MAX 				20000  // в мкс
#define PULSE_DURATION_DEF 				10000  // в мкс

#define PULSE_COUNT_MIN					1
#define PULSE_COUNT_MAX					3
//
#define SYNC_OSC						0
#define SYNC_SCPC						1
// Types
//
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;


// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H

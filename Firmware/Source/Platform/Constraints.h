// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"

// в мкс
#define PULSE_PAUSE_TIME_MIN 			0		// в мкс
#define PULSE_PAUSE_TIME_MAX 			20000	// в мкс
#define PULSE_PAUSE_TIME_DEF 			10000	// в мкс

#define PULSE_COUNT_MIN					1
#define PULSE_COUNT_MAX					3

// Types
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;

// Variables
extern const TableItemConstraint NVConstraint[];
extern const TableItemConstraint VConstraint[];

#endif // __CONSTRAINTS_H

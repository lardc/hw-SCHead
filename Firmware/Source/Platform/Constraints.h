#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"

//Definitions
#define BAT_VOLTAGE_MIN				300		// в В
#define BAT_VOLTAGE_MAX				1500	// в В
#define BAT_VOLTAGE_DEF				500		// в В

#define GAIN_COEFFICIENT_MIN		1
#define GAIN_COEFFICIENT_MAX		10000

#define GAIN_VOLTAGE_DEF			1000

#define VOLTAGE_ERROR_MIN			1		// в % / 10
#define VOLTAGE_ERROR_MAX			100		// в % / 10
#define VOLTAGE_ERROR_DEF			5		// в % / 10

#define CHARGE_TIMEOUT_MIN			1000	// в мс
#define CHARGE_TIMEOUT_MAX			60000	// в мс
#define CHARGE_TIMEOUT_DEF			10000	// в мс

#define FAN_OPERATE_MIN				10		// в сек
#define FAN_OPERATE_MAX				300		// в сек
#define FAN_OPERATE_DEF				60		// в сек

#define RECHARGE_TIMEOUT_MIN		0		// в мс
#define RECHARGE_TIMEOUT_MAX		500		// в мс
#define RECHARGE_TIMEOUT_DEF		10		// в мс

#define RESISTANCE_PER_LSB_MIN		0		// в Ом
#define RESISTANCE_PER_LSB_MAX		48000	// в Ом
#define RESISTANCE_PER_LSB_DEF		48000	// в Ом

#define CURRENT_MAX_PER_BIT_MIN		1		// в А
#define CURRENT_MAX_PER_BIT_MAX		1700	// в А
#define CURRENT_MAX_PER_BIT_DEF		1612	// в А

#define SYNC_WAIT_TIMEOUT_MIN		0		// в мс
#define SYNC_WAIT_TIMEOUT_MAX		10000	// в мс
#define SYNC_WAIT_TIMEOUT_DEF		500		// в мс

#define AFTER_PULSE_TIMEOUT_MIN		0		// в мс
#define AFTER_PULSE_TIMEOUT_MAX		30000	// в мс
#define AFTER_PULSE_TIMEOUT_DEF		2000	// в мс

#define GATE_RESOLUTION_MIN			1
#define GATE_RESOLUTION_MAX			11
#define GATE_RESOLUTION_DEF			9

#define V_BAT_OFFSET_MIN			INT16S_MIN
#define V_BAT_OFFSET_MAX			INT16S_MAX
#define V_BAT_OFFSET_DEF			30

#define V_BAT_K_MIN					1
#define V_BAT_K_MAX					1000
#define V_BAT_K_DEF					480

#define FAN_PERIOD_MIN				60
#define FAN_PERIOD_MAX				600
#define FAN_PERIOD_DEF				180

// Types
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;

// Variables
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];

#endif // __CONSTRAINTS_H

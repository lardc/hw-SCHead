// -----------------------------------------
// Global definitions
// ----------------------------------------

// Header
#include "Constraints.h"

// Constants
//
const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE] =
{
  {0, INT16U_MAX, K_DUT_U_DEFAULT},								                // 0
  {0, INT16U_MAX, K_DUT_I_DEFAULT},						                        // 1
  {0, INT16U_MAX, U1_OFFSET_DEFAULT},							                // 2
  {0, INT16U_MAX, U2_OFFSET_DEFAULT},					                        // 3
  {0, INT16U_MAX, I1_OFFSET_DEFAULT},								            // 4
  {0, INT16U_MAX, I2_OFFSET_DEFAULT},									        // 5
  {0, INT16U_MAX, K_SC_SET_DEFAULT},											// 6
  {0, INT16U_MAX, B_SC_SET_DEFAULT},											// 7
  {0, INT16U_MAX, K_U_CAL_DEFAULT},												// 8
  {0, INT16U_MAX, B_U_CAL_DEFAULT},												// 9
  {0, INT16U_MAX, K1_I_CAL_DEFAULT},											// 10
  {0, INT16U_MAX, B1_I_CAL_DEFAULT},											// 11
  {0, INT16U_MAX, K2_I_CAL_DEFAULT},    										// 12
  {0, INT16U_MAX, B2_I_CAL_DEFAULT},											// 13
  {0, INT16U_MAX, K3_I_CAL_DEFAULT},											// 14
  {0, INT16U_MAX, B3_I_CAL_DEFAULT},											// 15
  {0, INT16U_MAX, K4_I_CAL_DEFAULT},											// 16
  {0, INT16U_MAX, B4_I_CAL_DEFAULT},											// 17
  {0, INT16U_MAX, K5_I_CAL_DEFAULT},											// 18
  {0, INT16U_MAX, B5_I_CAL_DEFAULT},											// 19
  {0, SCTU_NUM_MAX, SCTU_NUM_MAX},												// 20
  {0, INT16U_MAX, ADR_SCPC0},													// 21
  {0, INT16U_MAX, K1_I_CAL_DEFAULT},											// 22
  {0, INT16U_MAX, B1_I_CAL_DEFAULT},											// 23
  {0, INT16U_MAX, K2_I_CAL_DEFAULT},											// 24
  {0, INT16U_MAX, B2_I_CAL_DEFAULT},											// 25
  {0, INT16U_MAX, K3_I_CAL_DEFAULT},											// 26
  {0, INT16U_MAX, B3_I_CAL_DEFAULT},											// 27
  {0, INT16U_MAX, K4_I_CAL_DEFAULT},											// 28
  {0, INT16U_MAX, B4_I_CAL_DEFAULT},											// 29
  {0, INT16U_MAX, K5_I_CAL_DEFAULT},											// 30
  {0, INT16U_MAX, B5_I_CAL_DEFAULT},											// 31
  {0, 0, 0},																	// 32
  {0, 0, 0},																	// 33
  {0, 0, 0},																	// 34
  {0, 0, 0},																	// 35
  {0, 0, 0},																	// 36
  {0, 0, 0},																	// 37
  {0, 0, 0},																	// 38
  {0, 0, 0},																	// 39
  {0, INT16U_MAX, 0},															// 40
  {0, INT16U_MAX, 0},															// 41
  {0, 0, 0},																	// 42
  {0, 0, 0},																	// 43
  {0, 0, 0},																	// 44
  {0, 0, 0},																	// 45
  {0, 0, 0},																	// 46
  {0, 0, 0},																	// 47
  {0, 0, 0},																	// 48
  {0, 0, 0},																	// 49
  {0, 0, 0},																	// 50
  {0, 0, 0},																	// 51
  {0, 0, 0},																	// 52
  {0, 0, 0},																	// 53
  {0, 0, 0},																	// 54
  {0, 0, 0},																	// 55
  {0, 0, 0},																	// 56
  {0, 0, 0},																	// 57
  {0, 0, 0},																	// 58
  {0, 0, 0},																	// 59
  {0, 0, 0},																	// 60
  {0, 0, 0},																	// 61
  {0, 0, 0},																	// 62
  {0, 0, 0}                                                                                                                             // 63
};

const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START] =
{
  {SCTU_SC_VALUE_MIN, INT16U_MAX, 1000},  						                // 64
  {0, INT16U_MAX, 0},  							                                // 65
  {0, INT16U_MAX, DIODE},	        											// 66
  {0, INT16U_MAX, R_SHUNT_DEFAULT},	       									    // 67
  {0, INT16U_MAX, 0},														    // 68
  {0, INT16U_MAX, 0},															// 69
  {0, INT16U_MAX, WAVEFORM_SINE},									            // 70
  {TRAPEZE_EDGE_TIME_MIN, TRAPEZE_EDGE_TIME_MAX, TRAPEZE_EDGE_TIME_MAX},        // 71
  {1, 2, CHANNEL_DEFAULT},													    // 72
  {0, 4, 0},																	// 73
  {0, INT16U_MAX, 0},															// 74
  {0, INT16U_MAX, 0},															// 75
  {0, INT16U_MAX, 0},															// 76
  {0, INT16U_MAX, 0},															// 77
  {EMUL_SCPC_DIS, EMUL_SCPC_EN, EMUL_SCPC_DIS},									// 78
  {0, 0, 0},																	// 79
  {PULSE_DURATION_MIN, PULSE_DURATION_MAX, PULSE_DURATION_DEF},		   		    // 80
  {PULSE_COUNT_MIN, PULSE_COUNT_MAX, PULSE_COUNT_MIN},							// 81
  {0, 0, 0},																	// 82
  {0, 0, 0},																	// 83
  {0, 0, 0},																	// 84
  {0, 0, 0},																	// 85
  {0, 0, 0},																	// 86
  {0, 0, 0},																	// 87
  {0, 0, 0},																	// 88
  {0, 0, 0},																	// 89
  {0, 0, 0},																	// 90
  {0, 0, 0},																	// 91
  {0, 0, 0},																	// 92
  {0, 0, 0},																	// 93
  {0, 0, 0},																	// 94
};

// No more

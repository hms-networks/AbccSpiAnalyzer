/*******************************************************************************
********************************************************************************
** COPYRIGHT NOTIFICATION (c) 2017 HMS Industrial Networks AB                 **
**                                                                            **
** This code is the property of HMS Industrial Networks AB.                   **
** The source code may not be reproduced, distributed, or used without        **
** permission. When used together with a product from HMS, permission is      **
** granted to modify, reproduce and distribute the code in binary form        **
** without any restrictions.                                                  **
**                                                                            **
** THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT    **
** WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR     **
** THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR     **
** THAT DEFECTS IN IT CAN BE CORRECTED.                                       **

********************************************************************************
********************************************************************************
** This file contains Energy Measurement specific definitions used by
** ABCC modules as well as applications designed to use such modules.
********************************************************************************
********************************************************************************
*/

#ifndef ABP_EME_H_
#define ABP_EME_H_

#include "abp.h"


/*------------------------------------------------------------------------------
** The Energy Measurement object specific message commands.
**------------------------------------------------------------------------------
*/
#define ABP_EME_CMD_GET_ATTRIBUTE_MEASUREMENT_LIST                 0x10

/*------------------------------------------------------------------------------
**
** The Energy Measurement instance attributes.
**
**------------------------------------------------------------------------------
*/
#define ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL                       1
#define ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_MIN                   2
#define ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_MAX                   3

#define ABP_EME_IA_VOLTAGE_PHASE_PHASE                         4
#define ABP_EME_IA_VOLTAGE_PHASE_PHASE_MIN                     5
#define ABP_EME_IA_VOLTAGE_PHASE_PHASE_MAX                     6

#define ABP_EME_IA_VOLTAGE_PHASE_GROUND                        7
#define ABP_EME_IA_VOLTAGE_PHASE_GROUND_MIN                    8
#define ABP_EME_IA_VOLTAGE_PHASE_GROUND_MAX                    9

#define ABP_EME_IA_CURRENT                                     10
#define ABP_EME_IA_CURRENT_MIN                                 11
#define ABP_EME_IA_CURRENT_MAX                                 12

#define ABP_EME_IA_APPARENT_POWER                              13
#define ABP_EME_IA_APPARENT_POWER_MIN                          14
#define ABP_EME_IA_APPARENT_POWER_MAX                          15

#define ABP_EME_IA_ACTIVE_POWER                                16
#define ABP_EME_IA_ACTIVE_POWER_MIN                            17
#define ABP_EME_IA_ACTIVE_POWER_MAX                            18

#define ABP_EME_IA_REACTIVE_POWER                              19
#define ABP_EME_IA_REACTIVE_POWER_MIN                          20
#define ABP_EME_IA_REACTIVE_POWER_MAX                          21

#define ABP_EME_IA_POWER_FACTOR                                22
#define ABP_EME_IA_POWER_FACTOR_MIN                            23
#define ABP_EME_IA_POWER_FACTOR_MAX                            24

#define ABP_EME_IA_FREQUENCY                                   25
#define ABP_EME_IA_FREQUENCY_MIN                               26
#define ABP_EME_IA_FREQUENCY_MAX                               27

#define ABP_EME_IA_FIELD_ROTATION                              28
#define ABP_EME_IA_TOTAL_ACTIVE_ENERGY                         29
#define ABP_EME_IA_TOTAL_REACTIVE_ENERGY                       30
#define ABP_EME_IA_TOTAL_APPARENT_ENERGY                       31

/*------------------------------------------------------------------------------
**
** The data size of the Energy Measurement instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/
#define ABP_EME_TIMESTAMP_VALUE_DS              ( ABP_UINT64_SIZEOF +         \
                                                  ABP_FLOAT_SIZEOF )
#define ABP_EME_3X_TIMESTAMP_VALUE_DS           ( ABP_EME_TIMESTAMP_VALUE_DS * 3 )
#define ABP_EME_4X_TIMESTAMP_VALUE_DS           ( ABP_EME_TIMESTAMP_VALUE_DS * 4 )

#define ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_DS     ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_MIN_DS ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_MAX_DS ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_VOLTAGE_PHASE_PHASE_DS       ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_VOLTAGE_PHASE_PHASE_MIN_DS   ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_VOLTAGE_PHASE_PHASE_MAX_DS   ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_VOLTAGE_PHASE_GROUND_DS      ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_VOLTAGE_PHASE_GROUND_MIN_DS  ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_VOLTAGE_PHASE_GROUND_MAX_DS  ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_CURRENT_DS                   ( ABP_EME_TIMESTAMP_VALUE_DS * 5 )
#define ABP_EME_IA_CURRENT_MIN_DS               ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_CURRENT_MAX_DS               ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_APPARENT_POWER_DS            ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_APPARENT_POWER_MIN_DS        ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_APPARENT_POWER_MAX_DS        ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_ACTIVE_POWER_DS              ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_ACTIVE_POWER_MIN_DS          ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_ACTIVE_POWER_MAX_DS          ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_REACTIVE_POWER_DS            ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_REACTIVE_POWER_MIN_DS        ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_REACTIVE_POWER_MAX_DS        ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_POWER_FACTOR_DS              ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_POWER_FACTOR_MIN_DS          ABP_EME_4X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_POWER_FACTOR_MAX_DS          ABP_EME_4X_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_FREQUENCY_DS                 ABP_EME_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_FREQUENCY_MIN_DS             ABP_EME_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_FREQUENCY_MAX_DS             ABP_EME_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_FIELD_ROTATION_DS            ABP_EME_TIMESTAMP_VALUE_DS

#define ABP_EME_IA_TOTAL_ACTIVE_ENERGY_DS       ABP_EME_3X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_TOTAL_REACTIVE_ENERGY_DS     ABP_EME_3X_TIMESTAMP_VALUE_DS
#define ABP_EME_IA_TOTAL_APPARENT_ENERGY_DS     ABP_EME_3X_TIMESTAMP_VALUE_DS

#endif  /* inclusion lock */

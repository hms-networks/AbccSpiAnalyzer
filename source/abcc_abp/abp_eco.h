/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_eco.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_eco.h - Anybus-CC Energy Control Object Protocol Definitions.
**
** This software component contains Energy Control definitions used by Anybus-CC
** modules as well as applications designed to use such modules.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011 HMS Industrial Networks AB                 **
**                                                                            **
** This code is the property of HMS Industrial Networks AB.                   **
** The source code may not be reproduced, distributed, or used without        **
** permission. When used together with a product from HMS, this code can be   **
** modified, reproduced and distributed in binary form without any            **
** restrictions.                                                              **
**                                                                            **
** THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT    **
** WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR     **
** THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR     **
** THAT DEFECTS IN IT CAN BE CORRECTED.                                       **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef ABP_ECO_H
#define ABP_ECO_H


/*******************************************************************************
**
** Anybus-CC Energy Control Object constants.
**
** Object revision: 2.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC Energy Control object specific attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ECO_OA_CURRENT_ENERGY_SAVING_MODE         11
#define ABP_ECO_OA_REMAINING_TIME_TO_DEST             12
#define ABP_ECO_OA_ENERGY_CONSUMP_TO_DEST             13
#define ABP_ECO_OA_TRANSITION_TO_POWER_OFF_SUPPORTED  14


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Energy Control object specific
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ECO_OA_CURRENT_ENERGY_SAVING_MODE_DS         ABP_UINT16_SIZEOF
#define ABP_ECO_OA_REMAINING_TIME_TO_DEST_DS             ABP_UINT32_SIZEOF
#define ABP_ECO_OA_ENERGY_CONSUMP_TO_DEST_DS             ABP_FLOAT_SIZEOF
#define ABP_ECO_OA_TRANSITION_TO_POWER_OFF_SUPPORTED_DS  ABP_BOOL_SIZEOF

/*------------------------------------------------------------------------------
**
** The Anybus-CC Energy Control object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ECO_IA_MODE_ATTRIBUTES           1
#define ABP_ECO_IA_TIME_MIN_PAUSE            2
#define ABP_ECO_IA_TIME_TO_PAUSE             3
#define ABP_ECO_IA_TIME_TO_OPERATE           4
#define ABP_ECO_IA_TIME_MIN_LENGTH_OF_STAY   5
#define ABP_ECO_IA_TIME_MAX_LENGTH_OF_STAY   6
#define ABP_ECO_IA_MODE_POWER_CONSUMP        7
#define ABP_ECO_IA_ENERGY_CONSUMP_TO_PAUSE   8
#define ABP_ECO_IA_ENERGY_CONSUMP_TO_OPERATE 9
#define ABP_ECO_IA_AVAILABILITY              10
#define ABP_ECO_IA_POWER_CONSUMPTION         11


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Energy Control object instance attributes
** (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ECO_IA_MODE_ATTRIBUTES_DS              ABP_UINT16_SIZEOF
#define ABP_ECO_IA_TIME_MIN_PAUSE_DS               ABP_UINT32_SIZEOF
#define ABP_ECO_IA_TIME_TO_PAUSE_DS                ABP_UINT32_SIZEOF
#define ABP_ECO_IA_TIME_TO_OPERATE_DS              ABP_UINT32_SIZEOF
#define ABP_ECO_IA_TIME_MIN_LENGTH_OF_STAY_DS      ABP_UINT32_SIZEOF
#define ABP_ECO_IA_TIME_MAX_LENGTH_OF_STAY_DS      ABP_UINT32_SIZEOF
#define ABP_ECO_IA_MODE_POWER_CONSUMP_DS           ABP_FLOAT_SIZEOF
#define ABP_ECO_IA_ENERGY_CONSUMP_TO_PAUSE_DS      ABP_FLOAT_SIZEOF
#define ABP_ECO_IA_ENERGY_CONSUMP_TO_OPERATE_DS    ABP_FLOAT_SIZEOF
#define ABP_ECO_IA_AVAILABILITY_DS                 ABP_BOOL_SIZEOF
#define ABP_ECO_IA_POWER_CONSUMPTION_DS            ABP_UINT32_SIZEOF


/*------------------------------------------------------------------------------
**
** The Anybus-CC Energy Control object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_ECO_CMD_START_PAUSE           0x10
#define ABP_ECO_CMD_END_PAUSE             0x11
#define ABP_ECO_CMD_PREVIEW_PAUSE_TIME    0x12


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_eco.h
**
********************************************************************************
*/

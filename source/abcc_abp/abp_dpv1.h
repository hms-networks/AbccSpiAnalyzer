/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_dpv1.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_dpv1 - Anybus-CC Protocol - PROFIBUS DP-V1 application object definitions
**            and PROFIBUS specific constants for the Network object
**            and Diagnostic object.
**
** This software component contains protocol definitions used by Anybus-CC
** modules as well as applications designed to use such modules.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2008 HMS Industrial Networks AB                 **
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

#ifndef ABP_DPV1_H
#define ABP_DPV1_H


/*******************************************************************************
**
** Diagnostic object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network specific error codes for the Diagnostic Object (Only for ABCC40)
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DI_DPV1_ERR_ALARM_TYPE_DISABLED    = 0x08,
   ABP_DI_DPV1_ERR_TOO_MANY_ACTIVE_ALARMS = 0x09
};


/*******************************************************************************
**
** Network object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network object instance attribute # 7 information
** PROFIBUS specific exception constants
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_NW_EXCPT_DPV1_TOO_MUCH_DEFAULT_CFG_DATA       = 0x01,
   ABP_NW_EXCPT_DPV1_CFG_DATA_ATTR_TOO_BIG           = 0x02,
   ABP_NW_EXCPT_DPV1_CFG_DATA_MISMATCH_ADI_MAP       = 0x03,
   ABP_NW_EXCPT_DPV1_TOO_MUCH_PROCESS_DATA           = 0x04,
   ABP_NW_EXCPT_DPV1_CFG_DATA_AND_ADI_MAP_SPEC       = 0x05,
   ABP_NW_EXCPT_DPV1_SSA_DISABLED                    = 0x06,
   ABP_NW_EXCPT_DPV1_INV_BUFFER_MODE_ATTR            = 0x07,
   ABP_NW_EXCPT_DPV1_CFG_DATA_ATTR_INVALID           = 0x08,
   ABP_NW_EXCPT_DPV1_CFG_DATA_ATTR_TOO_MUCH_IO_DATA  = 0x09,
   ABP_NW_EXCPT_DPV1_UNABLE_TO_GET_ABCCDP_PARAM_LIST = 0x0A,
   ABP_NW_EXCPT_DPV1_INVALID_APPL_REMAP_CMD_RSP      = 0x0B,
   ABP_NW_EXCPT_DPV1_INVALID_MAP_SLOT_0              = 0x0C,
   ABP_NW_EXCPT_DPV1_INVALID_MAP_EMPTY_SLOT          = 0x0D
};


/*******************************************************************************
**
** PROFIBUS DP-V1 object constants.
**
** Object revision: 4
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** PROFIBUS DP-V1 instance attributes
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV1_IA_IDENT_NUMBER         = 1,
   ABP_DPV1_IA_PRM_DATA             = 2,
   ABP_DPV1_IA_EXPECTED_CFG_DATA    = 3,
   ABP_DPV1_IA_SSA_ENABLED          = 4,
   ABP_DPV1_IA_SIZEOF_ID_REL_DIAG   = 5,
   ABP_DPV1_IA_BUFFER_MODE          = 6,
   ABP_DPV1_IA_ALARM_SETTINGS       = 7,
   ABP_DPV1_IA_MANUFACTURER_ID      = 8,
   ABP_DPV1_IA_ORDER_ID             = 9,
   ABP_DPV1_IA_SERIAL_NO            = 10,
   ABP_DPV1_IA_HW_REV               = 11,
   ABP_DPV1_IA_SW_REV               = 12,
   ABP_DPV1_IA_REV_COUNTER          = 13,
   ABP_DPV1_IA_PROFILE_ID           = 14,
   ABP_DPV1_IA_PROFILE_SPEC_TYPE    = 15,
   ABP_DPV1_IA_IM_VERSION           = 16,
   ABP_DPV1_IA_IM_SUPPORTED         = 17,
   ABP_DPV1_IA_IM_HEADER            = 18,
   ABP_DPV1_IA_CHK_CFG_BEHAVIOR     = 19,
   ABP_DPV1_IA_RESERVED             = 20
};

/*------------------------------------------------------------------------------
**
** The data size of the PROFIBUS DP-V1 instance attributes
**
**------------------------------------------------------------------------------
*/

#define ABP_DPV1_IA_IDENT_NUMBER_DS          ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_PRM_DATA_MAX_DS          ( ABP_UINT8_SIZEOF * 244 )
#define ABP_DPV1_IA_CFG_DATA_MAX_DS          ( ABP_UINT8_SIZEOF * 244 )
#define ABP_DPV1_IA_SSA_ENABLED_DS           ( ABP_BOOL_SIZEOF )
#define ABP_DPV1_IA_SIZEOF_ID_REL_DIAG_DS    ( ABP_UINT8_SIZEOF )
#define ABP_DPV1_IA_BUFFER_MODE_DS           ( ABP_UINT8_SIZEOF )
#define ABP_DPV1_IA_ALARM_SETTINGS_DS        ( ABP_UINT8_SIZEOF +              \
                                               ABP_UINT8_SIZEOF +              \
                                               ABP_BOOL_SIZEOF )
#define ABP_DPV1_IA_MANUFACTURER_ID_DS       ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_ORDER_ID_DS              ( ABP_CHAR_SIZEOF * 20 )
#define ABP_DPV1_IA_SERIAL_NO_DS             ( ABP_CHAR_SIZEOF * 16 )
#define ABP_DPV1_IA_HW_REV_DS                ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_SW_REV_DS                ( ABP_CHAR_SIZEOF +               \
                                               ( ABP_UINT8_SIZEOF * 3 ) )
#define ABP_DPV1_IA_REV_COUNTER_DS           ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_PROFILE_ID_DS            ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_PROFILE_SPEC_TYPE_DS     ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_IM_VERSION_DS            ( ABP_UINT8_SIZEOF * 2 )
#define ABP_DPV1_IA_IM_SUPPORTED_DS          ( ABP_UINT16_SIZEOF )
#define ABP_DPV1_IA_IM_HEADER_DS             ( ABP_UINT8_SIZEOF * 10 )
#define ABP_DPV1_IA_CHK_CFG_BEHAVIOR_DS      ( ABP_UINT8_SIZEOF )
#define ABP_DPV1_IA_SKIP_ADI_DS              ( ABP_BOOL_SIZEOF )


/*------------------------------------------------------------------------------
**
** Values of Buffer mode attribute
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV1_DEF_BUFFER_MODE = 0,
   ABP_DPV1_PRM_BUFFER_MODE,
   ABP_DPV1_PD_BUFFER_MODE,
   ABP_DPV1_ALARM_BUFFER_MODE,
   ABP_DPV1_PDIM_BUFFER_MODE,
   ABP_DPV1_DEF_BUFFER_MODE_2,
   ABP_DPV1_PRM_BUFFER_MODE_2,
   ABP_DPV1_PD_BUFFER_MODE_2,

   /*
   ** The buffer modes below are for ABCC-DPV0 only.
   */

   ABP_DPV1_V0_MODE1    = 200,
   ABP_DPV1_V0_MODE2,
   ABP_DPV1_V0_MODE3,
   ABP_DPV1_V0_MODE4
};

/*------------------------------------------------------------------------------
**
** Values of Profile ID attribute
** Note! This is not a complete list. The Profile ID's is defined by the
** PROFIBUS/PROFINET International organization.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV1_PROFILE_ID_NONE            = 0x0000,
   ABP_DPV1_PROFILE_ID_PROFIDRIVE      = 0x3A00,
   ABP_DPV1_PROFILE_ID_ROBOT           = 0x3B00,
   ABP_DPV1_PROFILE_ID_DISPLAY         = 0x3C00,
   ABP_DPV1_PROFILE_ID_ENCODER         = 0x3D00,
   ABP_DPV1_PROFILE_ID_FLUID_POWER     = 0x3F00,
   ABP_DPV1_PROFILE_ID_WATER_TREATMENT = 0x4900,
   ABP_DPV1_PROFILE_ID_IOLINK_MASTER   = 0x4E00,
   ABP_DPV1_PROFILE_ID_IDENTIFICATION  = 0x5B00,
   ABP_DPV1_PROFILE_ID_WEIGHING_DOSAGE = 0x5C00,
   ABP_DPV1_PROFILE_ID_PUMPS           = 0x5D00,
   ABP_DPV1_PROFILE_ID_SWITCHGEAR      = 0x5E00,
   ABP_DPV1_PROFILE_ID_REMOTE_IO       = 0x5F00,
   ABP_DPV1_PROFILE_ID_LABORATORY      = 0x6200,
   ABP_DPV1_PROFILE_ID_PA              = 0x9700,
   ABP_DPV1_PROFILE_ID_GENERIC_DEVICE  = 0xF600
};

/*------------------------------------------------------------------------------
**
** Values of Check Cfg Behavior attribute (Only in ABCC30 series)
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV1_CHK_CFG_BEHAVIOR_0 = 0,
   ABP_DPV1_CHK_CFG_BEHAVIOR_1 = 1
};


/*------------------------------------------------------------------------------
**
** The PROFIBUS DP-V1 object specific message commands.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV1_CMD_GET_IM_RECORD = 0x10,
   ABP_DPV1_CMD_SET_IM_RECORD = 0x11,
   ABP_DPV1_CMD_ALARM_ACK     = 0x12,     /* Only for ABCC30 */
   ABP_DPV1_CMD_GET_RECORD    = 0x13,     /* Only for ABCC40 */
   ABP_DPV1_CMD_SET_RECORD    = 0x14      /* Only for ABCC40 */
};

/*------------------------------------------------------------------------------
**
** The data sizes of the PROFIBUS DP-V1 object specific message commands.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV1_CMD_GET_IM_RECORD_DS       = 4,
   ABP_DPV1_CMD_SET_IM_RECORD_DS       = 68,
   ABP_DPV1_CMD_GET_RECORD_DS          = 12,
   ABP_DPV1_CMD_GET_RECORD_RSP_MAX_DS  = 240,
   ABP_DPV1_CMD_SET_RECORD_MAX_DS      = 251
};

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_dpv1.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_nwccl.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_nwccl - Anybus-CC Protocol - Network CC-Link object definitions
**
** This software component contains protocol definitions used by the CC-Link
** Anybus-CC module as well as applications designed to use such modules.
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

#ifndef ABP_NWCCL_H
#define ABP_NWCCL_H


/*******************************************************************************
**
** Anybus-CC Network CC-Link object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network CC-Link object instance number
**
**------------------------------------------------------------------------------
*/

#define ABP_NWCCL_OI_ID                   1


/*------------------------------------------------------------------------------
**
** Network CC-Link object instance attributes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_NWCCL_IA_NETWORK_SETTINGS        = 1,
   ABP_NWCCL_IA_SYSTEM_AREA_HANDLER     = 2,
   ABP_NWCCL_IA_ERROR_CODE_POSITION     = 3,
   ABP_NWCCL_IA_LAST_MAPPING_INFO       = 4,
   ABP_NWCCL_IA_CCL_CONF_TEST_MODE      = 5,
   ABP_NWCCL_IA_ERROR_INFO              = 6
};


/*------------------------------------------------------------------------------
**
** The data size of the Network CC-Link object instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_NWCCL_IA_NETWORK_SETTINGS_DS     ABP_CCL_IA_NETWORK_SETTINGS_DS
#define ABP_NWCCL_IA_SYSTEM_AREA_HANDLER_DS  ABP_SINT16_SIZEOF
#define ABP_NWCCL_IA_ERROR_CODE_POSITION_DS  ABP_SINT8_SIZEOF
#define ABP_NWCCL_IA_LAST_MAPPING_INFO_DS    ( ABP_UINT8_SIZEOF +             \
                                             ABP_UINT16_SIZEOF +              \
                                             ABP_UINT16_SIZEOF )
#define ABP_NWCCL_IA_CCL_CONF_TEST_MODE_DS   ABP_BOOL_SIZEOF
#define ABP_NWCCL_IA_ERROR_INFO_DS           ( ABP_UINT8_SIZEOF +             \
                                               ABP_UINT8_SIZEOF +             \
                                               ABP_UINT8_SIZEOF )

/*------------------------------------------------------------------------------
**
** Network CC-Link object specific commands
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_NWCCL_MsgCmdType
{
   ABP_NWCCL_CMD_MAP_ADI_SPEC_WRITE_AREA   = 0x10,
   ABP_NWCCL_CMD_MAP_ADI_SPEC_READ_AREA    = 0x11,
   ABP_NWCCL_CMD_CCL_CONF_TEST_MODE        = 0x12
}ABP_NWCCL_MsgCmdType;


/*------------------------------------------------------------------------------
**
** The Network CC-Link object specific error codes.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_NWCCL_MsgErrorCodeType
{
   ABP_NWCCL_ERR_INVALID_ADI_DATA_TYPE   = 0x01,
   ABP_NWCCL_ERR_INVALID_NUM_ELEMENTS    = 0x02,
   ABP_NWCCL_ERR_INVALID_TOTAL_SIZE      = 0x03,
   ABP_NWCCL_ERR_INVALID_ORDER_NUM       = 0x04,
   ABP_NWCCL_ERR_INVALID_MAP_CMD_SEQ     = 0x05,
   ABP_NWCCL_ERR_INVALID_CCL_AREA        = 0x06,
   ABP_NWCCL_ERR_INVALID_OFFSET          = 0x07,
   ABP_NWCCL_ERR_DATA_OVERLAPPING        = 0x08
}ABP_NWCCL_MsgErrorCodeType;

#endif  /* inclusion lock */

/*******************************************************************************
**
** end of abp_nwccl.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_src3.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_src3 - Anybus-CC Protocol - SERCOS III Definitions.
**
** This software component contains protocol definitions used by the SERCOS III
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

#ifndef ABP_SRC3_H
#define ABP_SRC3_H


/*******************************************************************************
**
** Anybus-CC SERCOS III object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** SERCOS III object instance number
**
**------------------------------------------------------------------------------
*/

#define ABP_SRC3_OI_ID                     1


/*------------------------------------------------------------------------------
**
** SERCOS III object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SRC3_IA_COMPONENT_NAME         1
#define ABP_SRC3_IA_VENDOR_CODE            2
#define ABP_SRC3_IA_DEVICE_NAME            3
#define ABP_SRC3_IA_VENDOR_DEVICE_ID       4
#define ABP_SRC3_IA_SOFTWARE_REVISION      5
#define ABP_SRC3_IA_SERIAL_NUMBER          6
#define ABP_SRC3_IA_MAJOR_EVT_LATCHING     7

/*------------------------------------------------------------------------------
**
** The data size of the SERCOS III object instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_SRC3_CFG_STR_LEN               32

#define ABP_SRC3_IA_COMPONENT_NAME_DS      ( ABP_CHAR_SIZEOF * ABP_SRC3_CFG_STR_LEN )
#define ABP_SRC3_IA_VENDOR_CODE_DS         ABP_UINT16_SIZEOF
#define ABP_SRC3_IA_DEVICE_NAME_DS         ( ABP_CHAR_SIZEOF * ABP_SRC3_CFG_STR_LEN )
#define ABP_SRC3_IA_VENDOR_DEVICE_ID_DS    ( ABP_CHAR_SIZEOF * ABP_SRC3_CFG_STR_LEN )
#define ABP_SRC3_IA_SOFTWARE_REVISION_DS   ( ABP_CHAR_SIZEOF * ABP_SRC3_CFG_STR_LEN )
#define ABP_SRC3_IA_SERIAL_NUMBER_DS       ( ABP_CHAR_SIZEOF * ABP_SRC3_CFG_STR_LEN )
#define ABP_SRC3_IA_MAJOR_EVT_LATCHING_DS  ABP_BOOL_SIZEOF


/*------------------------------------------------------------------------------
**
** The SERCOS III Object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_SRC3_CMD_RESET_DIAGNOSTIC     0x10


/*******************************************************************************
**
** Network object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network object instance attribute # 7 information
** SERCOS III specific exception constants
**
**------------------------------------------------------------------------------
*/

#define ABP_SRC3_NW_EXCPT_ILLEGAL_DATA_TYPE           1
#define ABP_SRC3_NW_EXCPT_INSTANCE_BY_ORDER_ERROR     2
#define ABP_SRC3_NW_EXCPT_HIGHEST_INSTANCE_ERROR      3
#define ABP_SRC3_NW_EXCPT_NUMBER_OF_INSTANCES_ERROR   4
#define ABP_SRC3_NW_EXCPT_INSTANCE_LOWER_THAN_ORDER_NUMBER  5


/*******************************************************************************
**
** Network configuration object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Network configuration instances common to most Anybus-CC modules.
**
** Note:
**       In case the values of these instances originate from input devices
**       controlled by the end user (DIP switches or similar), the application
**       shall keep these instances updated at all times because some networks
**       require that a changed switch is indicated by the LEDs.
**
**------------------------------------------------------------------------------
*/

#define ABP_NC_INST_NUM_SERCOS_ADR       0x03

/*******************************************************************************
**
** Diagnostic object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Device specific emergency (EMCY) error codes.
** These should be used in combination with the ABP_DI_EVENT_NW_SPECIFIC
** diagnostic code.
**
**------------------------------------------------------------------------------
*/

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_src3.h
**
********************************************************************************
*/

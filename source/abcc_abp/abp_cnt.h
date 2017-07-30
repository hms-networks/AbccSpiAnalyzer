/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_cnt.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC Protocol Definitions for ControlNet.
**
** This file contains network specific definitions used by the Anybus-CC
** ControlNet module as well as applications designed to use such module.
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

#ifndef ABP_CNT_H
#define ABP_CNT_H


/*******************************************************************************
**
** Anybus-CC ControlNet object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC ControlNet Object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_CNT_IA_VENDOR_ID                1
#define ABP_CNT_IA_DEVICE_TYPE              2
#define ABP_CNT_IA_PRODUCT_CODE             3
#define ABP_CNT_IA_REVISION                 4
#define ABP_CNT_IA_SERIAL_NUMBER            5
#define ABP_CNT_IA_PRODUCT_NAME             6
#define ABP_CNT_IA_PROD_INSTANCE            7
#define ABP_CNT_IA_CONS_INSTANCE            8
#define ABP_CNT_IA_ENABLE_APP_CIP_OBJECTS   11
#define ABP_CNT_IA_ENABLE_PARAM_OBJECT      12
#define ABP_CNT_IA_CONFIG_INSTANCE          15


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC ControlNet Object instance attributes (in
** bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_CNT_IA_VENDOR_ID_DS                    ABP_UINT16_SIZEOF
#define ABP_CNT_IA_DEVICE_TYPE_DS                  ABP_UINT16_SIZEOF
#define ABP_CNT_IA_PRODUCT_CODE_DS                 ABP_UINT16_SIZEOF
#define ABP_CNT_IA_REVISION_DS                     ( 2 * ABP_UINT8_SIZEOF )
#define ABP_CNT_IA_SERIAL_NUMBER_DS                ABP_UINT32_SIZEOF
#define ABP_CNT_IA_PROD_INSTANCE_DS                ABP_UINT16_SIZEOF
#define ABP_CNT_IA_CONS_INSTANCE_DS                ABP_UINT16_SIZEOF
#define ABP_CNT_IA_ENABLE_APP_CIP_OBJECTS_DS       ABP_BOOL_SIZEOF
#define ABP_CNT_IA_ENABLE_PARAM_OBJECT_DS          ABP_BOOL_SIZEOF
#define ABP_CNT_IA_CONFIG_INSTANCE_DS              ABP_UINT16_SIZEOF


/*------------------------------------------------------------------------------
**
** The Anybus-CC ControlNet Object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_CNT_CMD_PROCESS_CIP_OBJ_REQUEST     0x10
#define ABP_CNT_CMD_SET_CONFIG_DATA             0x11
#define ABP_CNT_CMD_GET_CONFIG_DATA             0x13


/*------------------------------------------------------------------------------
**
** The Anybus-CC ControlNet Object specific exception codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_CNT_NW_EXCPT_INFO_INVALID_SY_INST   0x01


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_cnt.h
**
********************************************************************************
*/

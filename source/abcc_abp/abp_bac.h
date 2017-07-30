/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_bac.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC Protocol Definitions for BACnet Object.
**
** This file contains network specific definitions used by the Anybus-CC
** BACnet modules as well as applications designed to use such module.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2010 HMS Industrial Networks AB                 **
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

#ifndef ABP_BAC_H
#define ABP_BAC_H


/*******************************************************************************
**
** Anybus-CC BACnet object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC BACnet Object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_BAC_IA_OBJECT_NAME                         1
#define ABP_BAC_IA_VENDOR_NAME                         2
#define ABP_BAC_IA_VENDOR_IDENTIFIER                   3
#define ABP_BAC_IA_MODEL_NAME                          4
#define ABP_BAC_IA_FIRMWARE_REVISION                   5
#define ABP_BAC_IA_APP_SOFTWARE_VERSION                6
#define ABP_BAC_IA_SUPPORT_ADV_MAPPING                 7
#define ABP_BAC_IA_CURRENT_DATE_AND_TIME               8
#define ABP_BAC_IA_PASSWORD                            9


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC BACnet Object instance attributes (in
** bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_BAC_IA_OBJECT_NAME_MAX_DS                  ( 64 * ABP_UINT8_SIZEOF )
#define ABP_BAC_IA_VENDOR_NAME_MAX_DS                  ( 64 * ABP_UINT8_SIZEOF )
#define ABP_BAC_IA_VENDOR_IDENTIFIER_DS                ABP_UINT16_SIZEOF
#define ABP_BAC_IA_MODEL_NAME_MAX_DS                   ( 64 * ABP_UINT8_SIZEOF )
#define ABP_BAC_IA_FIRMWARE_REVISION_MAX_DS            ( 16 * ABP_UINT8_SIZEOF )
#define ABP_BAC_IA_APP_SOFTWARE_VERSION_MAX_DS         ( 16 * ABP_UINT8_SIZEOF )
#define ABP_BAC_IA_SUPPORT_ADV_MAPPING_DS              ABP_BOOL_SIZEOF
#define ABP_BAC_IA_CURRENT_DATE_AND_TIME_DS            7
#define ABP_BAC_IA_PASSWORD_MAX_DS                     ( 20 * ABP_UINT8_SIZEOF )


/*------------------------------------------------------------------------------
**
** The Anybus-CC BACnet Object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_BAC_CMD_GET_ADI_BY_BACNET_OBJ_INST         0x10
#define ABP_BAC_CMD_GET_ADI_BY_BACNET_OBJ_INST_NAME    0x11
#define ABP_BAC_CMD_GET_ALL_BACNET_OBJ_INSTANCES       0x12
#define ABP_BAC_CMD_GET_BACNET_OBJ_INST_BY_ADI         0x13


/*------------------------------------------------------------------------------
**
** BACnet specific exception information codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_AV     0x01
#define ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_BV     0x02
#define ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_MSV    0x03
#define ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_BY_ADI 0x04


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_bac.h
**
********************************************************************************
*/

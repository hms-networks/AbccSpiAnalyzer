/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_eip.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC Protocol Definitions for EtherNet/IP Object.
**
** This file contains network specific definitions used by the Anybus-CC
** EtherNet/IP module as well as applications designed to use such module.
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

#ifndef ABP_EIP_H
#define ABP_EIP_H


/*******************************************************************************
**
** Anybus-CC EtherNet/IP object constants.
**
** Object revision: 2.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC EtherNet/IP Object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_EIP_IA_VENDOR_ID                  1
#define ABP_EIP_IA_DEVICE_TYPE                2
#define ABP_EIP_IA_PRODUCT_CODE               3
#define ABP_EIP_IA_REVISION                   4
#define ABP_EIP_IA_SERIAL_NUMBER              5
#define ABP_EIP_IA_PRODUCT_NAME               6
#define ABP_EIP_IA_PROD_INSTANCE              7
#define ABP_EIP_IA_CONS_INSTANCE              8
#define ABP_EIP_IA_COMM_SETTINGS_FROM_NET     9
#define ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS     11
#define ABP_EIP_IA_ENABLE_PARAM_OBJECT        12
#define ABP_EIP_IA_INPUT_INSTANCE_OBJECT      13
#define ABP_EIP_IA_LISTEN_INSTANCE_OBJECT     14
#define ABP_EIP_IA_CONFIG_INSTANCE            15
#define ABP_EIP_IA_DISABLE_STRICT_IO_MATCH    16
#define ABP_EIP_IA_ENABLE_UNCONNECTED_SEND    17
#define ABP_EIP_IA_INPUT_EXT_INSTANCE_OBJECT  18
#define ABP_EIP_IA_LISTEN_EXT_INSTANCE_OBJECT 19
#define ABP_EIP_IA_IF_LABEL_PORT_1            20
#define ABP_EIP_IA_IF_LABEL_PORT_2            21
#define ABP_EIP_IA_IF_LABEL_PORT_INT          22
#define ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS_EXT 23 /* ABCC30 */
#define ABP_EIP_IA_PREPEND_PRODUCING          24
#define ABP_EIP_IA_PREPEND_CONSUMING          25
#define ABP_EIP_IA_ENABLE_EIP_QC              26
#define ABP_EIP_IA_PROD_INSTANCE_MAP          27 /* ABCC30 */
#define ABP_EIP_IA_CONS_INSTANCE_MAP          28 /* ABCC30 */
#define ABP_EIP_IA_IGNORE_SEQ_COUNT_CHECK     29
#define ABP_EIP_IA_ABCC_ADI_OBJECT            30
#define ABP_EIP_IA_ABCC_ENABLE_DLR            31
#define ABP_EIP_IA_ABCC_ENABLE_CIP_SYNC       32


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC EtherNet/IP Object instance attributes (in
** bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_EIP_IA_VENDOR_ID_DS                    ABP_UINT16_SIZEOF
#define ABP_EIP_IA_DEVICE_TYPE_DS                  ABP_UINT16_SIZEOF
#define ABP_EIP_IA_PRODUCT_CODE_DS                 ABP_UINT16_SIZEOF
#define ABP_EIP_IA_REVISION_DS                     ( 2 * ABP_UINT8_SIZEOF )
#define ABP_EIP_IA_SERIAL_NUMBER_DS                ABP_UINT32_SIZEOF
#define ABP_EIP_IA_PRODUCT_NAME_MAX_DS             ( 32 * ABP_UINT8_SIZEOF )
#define ABP_EIP_IA_PROD_INSTANCE_DS                ( 6 * ABP_UINT16_SIZEOF )
#define ABP_EIP_IA_CONS_INSTANCE_DS                ( 6 * ABP_UINT16_SIZEOF )
#define ABP_EIP_IA_COMM_SETTINGS_FROM_NET_DS       ABP_BOOL_SIZEOF
#define ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS_DS       ABP_BOOL_SIZEOF
#define ABP_EIP_IA_ENABLE_PARAM_OBJECT_DS          ABP_BOOL_SIZEOF
#define ABP_EIP_IA_INPUT_INSTANCE_DS               ABP_UINT16_SIZEOF
#define ABP_EIP_IA_LISTEN_INSTANCE_DS              ABP_UINT16_SIZEOF
#define ABP_EIP_IA_CONFIG_INSTANCE_DS              ABP_UINT16_SIZEOF
#define ABP_EIP_IA_DISABLE_STRICT_IO_MATCH_DS      ABP_BOOL_SIZEOF
#define ABP_EIP_IA_ENABLE_UNCONNECTED_SEND_DS      ABP_BOOL_SIZEOF
#define ABP_EIP_IA_INPUT_EXT_INSTANCE_DS           ABP_UINT16_SIZEOF
#define ABP_EIP_IA_LISTEN_EXT_INSTANCE_DS          ABP_UINT16_SIZEOF
#define ABP_EIP_IA_IF_LABEL_PORT_1_MAX_DS          ( 64 * ABP_UINT8_SIZEOF )
#define ABP_EIP_IA_IF_LABEL_PORT_2_MAX_DS          ( 64 * ABP_UINT8_SIZEOF )
#define ABP_EIP_IA_IF_LABEL_PORT_INT_MAX_DS        ( 64 * ABP_UINT8_SIZEOF )
#define ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS_EXT_DS   ABP_BOOL_SIZEOF               /* ABCC30 */
#define ABP_EIP_IA_PREPEND_PRODUCING_DS            ABP_UINT16_SIZEOF
#define ABP_EIP_IA_PREPEND_CONSUMING_DS            ABP_UINT16_SIZEOF
#define ABP_EIP_IA_ENABLE_EIP_QC_DS                ABP_BOOL_SIZEOF
#define ABP_EIP_IA_PROD_INSTANCE_MAP_DS            ( 6 * 2 * ABP_UINT16_SIZEOF ) /* ABCC30 */
#define ABP_EIP_IA_CONS_INSTANCE_MAP_DS            ( 6 * 2 * ABP_UINT16_SIZEOF ) /* ABCC30 */
#define ABP_EIP_IA_IGNORE_SEQ_COUNT_CHECK_DS       ABP_BOOL_SIZEOF
#define ABP_EIP_IA_ABCC_ADI_OBJECT_DS              ABP_UINT16_SIZEOF
#define ABP_EIP_IA_ABCC_ENABLE_DLR_DS              ABP_BOOL_SIZEOF
#define ABP_EIP_IA_ABCC_ENABLE_CIP_SYNC_DS         ABP_BOOL_SIZEOF


/*------------------------------------------------------------------------------
**
** The Anybus-CC EtherNet/IP Object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST        0x10
#define ABP_EIP_CMD_SET_CONFIG_DATA                0x11
#define ABP_EIP_CMD_PROCESS_CIP_ROUTING_REQUEST    0x12
#define ABP_EIP_CMD_GET_CONFIG_DATA                0x13
#define ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST_EXT    0x14 /* ABCC30 */


/*------------------------------------------------------------------------------
**
** Object specific error codes.
** Defines the second byte in a response from the application if the first byte
** is 0xFF (object specific error).
**
**------------------------------------------------------------------------------
*/

#define ABP_EIP_ERR_OWNERSHIP_CONFLICT         0x01  /* Ownership conflict    */
#define ABP_EIP_ERR_INVALID_CONFIG             0x02  /* Invalid configuration */


/*------------------------------------------------------------------------------
**
** Exception information codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_EIP_NW_EXCPT_INFO_INVALID_SY_INST        0x01
#define ABP_EIP_NW_EXCPT_INFO_INVALID_PROD_MAP_SIZE  0x02
#define ABP_EIP_NW_EXCPT_INFO_INVALID_CONS_MAP_SIZE  0x03
#define ABP_EIP_NW_EXCPT_INFO_MISSING_MAC_ADDRESS    0x04


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_eip.h
**
********************************************************************************
*/

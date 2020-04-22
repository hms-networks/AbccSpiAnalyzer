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
** This file contains OPC UA specific definitions.
********************************************************************************
********************************************************************************
*/

#ifndef ABP_OPCUA_H_
#define ABP_OPCUA_H_

/*------------------------------------------------------------------------------
** OPC UA instance attributes
**------------------------------------------------------------------------------
*/
#define ABP_OPCUA_IA_MODEL                   1
#define ABP_OPCUA_IA_APPLICATION_URI         2
#define ABP_OPCUA_IA_VENDOR_NAMESPACE_URI    3
#define ABP_OPCUA_IA_DEVICE_TYPE_NAME        4
#define ABP_OPCUA_IA_DEVICE_INST_NAME        5
#define ABP_OPCUA_IA_PRODUCT_URI             6
#define ABP_OPCUA_IA_LIMITS                  7

/*------------------------------------------------------------------------------
** The data size of the OPC UA instance attributes
**------------------------------------------------------------------------------
*/
#define ABP_OPCUA_IA_MODEL_DS                    ( ABP_UINT8_SIZEOF )
#define ABP_OPCUA_IA_APPLICATION_URI_MAX_DS      ( 128 * ABP_CHAR_SIZEOF )
#define ABP_OPCUA_IA_VENDOR_NAMESPACE_URI_MAX_DS ( 128 * ABP_CHAR_SIZEOF )
#define ABP_OPCUA_IA_DEVICE_TYPE_NAME_MAX_DS     ( 64 * ABP_CHAR_SIZEOF )
#define ABP_OPCUA_IA_DEVICE_INST_NAME_MAX_DS     ( 64 * ABP_CHAR_SIZEOF )
#define ABP_OPCUA_IA_PRODUCT_URI_MAX_DS          ( 128 * ABP_CHAR_SIZEOF )
#define ABP_OPCUA_IA_LIMITS_DS                   ( ABP_UINT16_SIZEOF + ABP_UINT32_SIZEOF )

/*------------------------------------------------------------------------------
** Values of Model attribute
**------------------------------------------------------------------------------
*/
typedef enum ABP_OpcuaModel
{
   ABP_OPCUA_MODEL_DISABLED            = 0,
   ABP_OPCUA_MODEL_CC40                = 1
}
ABP_OpcuaModelType;

/*------------------------------------------------------------------------------
** Valid ranges for the "Limits" attribute
**------------------------------------------------------------------------------
*/
#define ABP_OPCUA_IA_LIMITS_MAX_MON_ITEMS_MIN    ( 8 )
#define ABP_OPCUA_IA_LIMITS_MAX_MON_ITEMS_MAX    ( 100 )
#define ABP_OPCUA_IA_LIMITS_MIN_PUB_INT_MIN      ( 1000 )
#define ABP_OPCUA_IA_LIMITS_MIN_PUB_INT_MAX      ( 1000 * 3600 * 24 )

#endif  /* inclusion lock */

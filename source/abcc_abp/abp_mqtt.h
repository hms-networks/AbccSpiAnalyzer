/*******************************************************************************
********************************************************************************
** COPYRIGHT NOTIFICATION (c) 2018 HMS Industrial Networks AB                 **
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
** This file contains MQTT specific definitions.
********************************************************************************
********************************************************************************
*/

#ifndef ABP_MQTT_H_
#define ABP_MQTT_H_

/*------------------------------------------------------------------------------
** MQTT instance attributes
**------------------------------------------------------------------------------
*/
#define ABP_MQTT_IA_MODE        ( 1 )
#define APB_MQTT_IA_LAST_WILL   ( 2 )

/*------------------------------------------------------------------------------
** The data size of the MQTT instance attributes
**------------------------------------------------------------------------------
*/
#define ABP_MQTT_IA_MODE_DS            ( ABP_UINT8_SIZEOF )
#define ABP_MQTT_IA_LASTWILL_MAX_DS    ( 390 * ABP_OCTET_SIZEOF )

/*------------------------------------------------------------------------------
** Values of the Mode attribute
**------------------------------------------------------------------------------
*/
typedef enum ABP_MqttMode
{
   ABP_MQTT_MODE_DISABLED = 0,
   ABP_MQTT_MODE_JSON     = 1
}
ABP_MqttModeType;

/*------------------------------------------------------------------------------
** Last will instance attribute specific defines
**------------------------------------------------------------------------------
*/
#define ABP_MQTT_LASTWILL_QOS_OFFSET                    ( 0 )
#define ABP_MQTT_LASTWILL_RETAIN_OFFSET                 ( 1 )
#define ABP_MQTT_LASTWILL_TOPIC_LENGTH_OFFSET           ( 2 )
#define ABP_MQTT_LASTWILL_TOPIC_STR_OFFSET              ( 4 )
#define ABP_MQTT_LASTWILL_MESSAGE_LENGTH_BASE_OFFSET    ( 4 )
#define ABP_MQTT_LASTWILL_MESSSAGE_STR_BASE_OFFSET      ( 6 )

/*------------------------------------------------------------------------------
** The MQTT Object specific message commands
**------------------------------------------------------------------------------
*/
#define ABP_MQTT_CMD_GET_PUBLISH_CONFIGURATION   ( 0x10 )

/*------------------------------------------------------------------------------
** Get_Publish_Configuration (0x10) specific defines
**------------------------------------------------------------------------------
*/
#define ABP_MQTT_GETPUBCONF_RETAIN_OFFSET   ( 0 )
#define ABP_MQTT_GETPUBCONF_TOPIC_OFFSET    ( 1 )

#endif  /* inclusion lock */

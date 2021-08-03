/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_time.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC time Object Protocol Definitions.
**
** This software component contains Time object definitions used by Anybus-CC
** modules as well as applications designed to use such modules.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2021 HMS Industrial Networks AB                 **
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

#ifndef ABP_TIME_H
#define ABP_TIME_H


/*******************************************************************************
**
** Time object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Time object specific object attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_TIME_OA_PROTOCOLS             11    /* List of available Protocols */


/*------------------------------------------------------------------------------
**
** Time protocols.
**
**------------------------------------------------------------------------------
*/

#define ABP_TIME_PROTOCOL_CIP_SYNC             0x00
#define ABP_TIME_PROTOCOL_SERCOS3              0x01
#define ABP_TIME_PROTOCOL_POWERLINK            0x02
#define ABP_TIME_PROTOCOL_ETHERCAT             0x03
#define ABP_TIME_PROTOCOL_PROFINET             0x04
#define ABP_TIME_PROTOCOL_CANOPEN              0x05
#define ABP_TIME_PROTOCOL_BACNET               0x06
#define ABP_TIME_PROTOCOL_NTP                  0x07
#define ABP_TIME_PROTOCOL_OPCUA_DISCOVERY      0x08


/*------------------------------------------------------------------------------
** Structure describing instance protocols
**------------------------------------------------------------------------------
*/
typedef struct ABP_TIME_InstanceProtocolType
{
   /*
   ** Instance number
   */
   UINT16  iInstance;

   /*
   ** Protocol
   */
   UINT8  bProtocol;

   /*
   ** Reserved
   */
   UINT8 bReserved;
}
ABP_TIME_InstanceProtocolType;


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Time object specific
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/


/*------------------------------------------------------------------------------
**
** The Time object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_TIME_IA_PROTOCOL               1    /* Protocol                   */
#define ABP_TIME_IA_CURRENT_TIME           2    /* Current time               */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC time object instance
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_TIME_IA_PROTOCOL_DS           ( ABP_ENUM_SIZEOF )
#define ABP_TIME_IA_CURRENT_TIME_DS       ( ABP_UINT64_SIZEOF )


/*------------------------------------------------------------------------------
**
** The time object specific message commands.
**
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
**
** Time object specific error codes.
**
**------------------------------------------------------------------------------
*/

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_time.h
**
********************************************************************************
*/

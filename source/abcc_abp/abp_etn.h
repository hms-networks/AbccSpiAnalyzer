/*******************************************************************************
********************************************************************************
**                                                                            **
** ABP version 7.31.01 (2016-09-16)                                           **
**                                                                            */
/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_etn.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC Protocol Definitions for Ethernet Object.
**
** This file contains network specific definitions used by the Anybus-CC
** Ethernet module as well as applications designed to use such module.
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

#ifndef ABP_ETN_H
#define ABP_ETN_H


/*******************************************************************************
**
** Anybus-CC Ethernet object constants.
**
** Object revision: 2.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC Ethernet Object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ETN_IA_MAC_ADDRESS                 1
#define ABP_ETN_IA_ENABLE_HICP                 2
#define ABP_ETN_IA_ENABLE_WEB                  3
#define ABP_ETN_IA_ENABLE_MOD_TCP              4
#define ABP_ETN_IA_ENABLE_WEB_ADI_ACCESS       5
#define ABP_ETN_IA_ENABLE_FTP                  6
#define ABP_ETN_IA_ENABLE_ADMIN_MODE           7
#define ABP_ETN_IA_NETWORK_STATUS              8
#define ABP_ETN_IA_PORT1_MAC_ADDRESS           9
#define ABP_ETN_IA_PORT2_MAC_ADDRESS           10
#define ABP_ETN_IA_ENABLE_ACD                  11
#define ABP_ETN_IA_PORT1_STATE                 12
#define ABP_ETN_IA_PORT2_STATE                 13
#define ABP_ETN_IA_ENABLE_WEB_UPDATE           14
#define ABP_ETN_IA_ENABLE_HICP_RESET           15
#define ABP_ETN_IA_IP_CONFIGURATION            16
#define ABP_ETN_IA_IP_ADDRESS_BYTE_0_2         17
#define ABP_ETN_IA_ETH_PHY_CONFIG              18

/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Ethernet Object instance attributes (in
** bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ETN_IA_MAC_ADDRESS_DS               ( 6 * ABP_UINT8_SIZEOF )
#define ABP_ETN_IA_ENABLE_HICP_DS               ABP_BOOL_SIZEOF
#define ABP_ETN_IA_ENABLE_WEB_DS                ABP_BOOL_SIZEOF
#define ABP_ETN_IA_ENABLE_MOD_TCP_DS            ABP_BOOL_SIZEOF
#define ABP_ETN_IA_ENABLE_WEB_ADI_ACCESS_DS     ABP_BOOL_SIZEOF
#define ABP_ETN_IA_ENABLE_FTP_DS                ABP_BOOL_SIZEOF
#define ABP_ETN_IA_ENABLE_ADMIN_MODE_DS         ABP_BOOL_SIZEOF
#define ABP_ETN_IA_NETWORK_STATUS_DS            ABP_UINT16_SIZEOF
#define ABP_ETN_IA_PORT1_MAC_ADDRESS_DS         ( 6 * ABP_UINT8_SIZEOF )
#define ABP_ETN_IA_PORT2_MAC_ADDRESS_DS         ( 6 * ABP_UINT8_SIZEOF )
#define ABP_ETN_IA_ENABLE_ACD_DS                ABP_BOOL_SIZEOF
#define ABP_ETN_IA_PORT1_STATE_DS               ABP_ENUM_SIZEOF
#define ABP_ETN_IA_PORT2_STATE_DS               ABP_ENUM_SIZEOF
#define ABP_ETN_IA_ENABLE_WEB_UPDATE_DS         ABP_BOOL_SIZEOF
#define ABP_ETN_IA_ENABLE_HICP_RESET_DS         ABP_BOOL_SIZEOF
#define ABP_ETN_IA_IP_CONFIGURATION_DS          ( 3 * ABP_UINT32_SIZEOF )
#define ABP_ETN_IA_IP_ADDRESS_BYTE_0_2_DS       ( 3 * ABP_UINT8_SIZEOF )


/*------------------------------------------------------------------------------
**
** Network status attribute bit definitions.
**
**------------------------------------------------------------------------------
*/

#define ABP_ETN_IA_NETWORK_STATUS_LINK           0x0001
#define ABP_ETN_IA_NETWORK_STATUS_IP_INUSE       0x0002
#define ABP_ETN_IA_NETWORK_STATUS_IP_CONFLICT    0x0004
#define ABP_ETN_IA_NETWORK_STATUS_LINK_PORT1     0x0008
#define ABP_ETN_IA_NETWORK_STATUS_LINK_PORT2     0x0010


/*------------------------------------------------------------------------------
**
** Port 1 and Port 2 state attribute values
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_EtnPortStateType
{
   ABP_ETN_IA_PORT_STATE_ENABLE     = 0x00, /* Enable Ethernet port           */
   ABP_ETN_IA_PORT_STATE_DISABLE    = 0x01, /* Disable Ethernet port          */
   ABP_ETN_IA_PORT_STATE_INACTIVATE = 0x02, /* Inactivate Ethernet port       */

   ABP_ETN_IA_PORT_STATE_NUM_VALUES         /* Number of port states          */
}
ABP_EtnPortStateType;


/*------------------------------------------------------------------------------
**
** Ethernet PHY Configuration attribute bit definitions.
**
** ABP_ETN_IA_ETH_PHY_CFG_FALLBACK_DUPLEX - 0 = Half duplex, 1 = Full duplex
**
**------------------------------------------------------------------------------
*/

#define ABP_ETN_IA_ETH_PHY_CFG_FALLBACK_DUPLEX   0x0001

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_etn.h
**
********************************************************************************
*/

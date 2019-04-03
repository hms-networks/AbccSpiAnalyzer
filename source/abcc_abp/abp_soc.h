/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_soc.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC Socket Interface Object Protocol Definitions.
**
** This software component contains SOC definitions used by Anybus-CC
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

#ifndef ABP_SOC_H
#define ABP_SOC_H


/*******************************************************************************
**
** Socket Interface object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Socket Interface object specific object attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SOC_OA_MAX_INST               11    /* Max number of instances    */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Socket Interface object specific attributes
** (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_SOC_OA_MAX_INST_DS             ABP_UINT16_SIZEOF


/*------------------------------------------------------------------------------
**
** The Socket Interface instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SOC_IA_SOCK_TYPE               1    /* Socket type                */
#define ABP_SOC_IA_LOCAL_PORT              2    /* Local port                 */
#define ABP_SOC_IA_HOST_IP                 3    /* Host IP address            */
#define ABP_SOC_IA_HOST_PORT               4    /* Host port                  */
#define ABP_SOC_IA_TCP_STATE               5    /* TCP state                  */
#define ABP_SOC_IA_RX_BYTES                6    /* Bytes in RX buffer         */
#define ABP_SOC_IA_TX_BYTES                7    /* Bytes in TX buffer         */
#define ABP_SOC_IA_SO_REUSE_ADDR           8    /* Reuse address option       */
#define ABP_SOC_IA_SO_KEEP_ALIVE           9    /* Keep alive option          */
#define ABP_SOC_IA_IP_MULT_TTL            10    /* Multicast time-to-live     */
#define ABP_SOC_IA_IP_MULT_LOOP           11    /* Multicast loopback         */
#define ABP_SOC_IA_TCP_ACKDELAYTIME       12    /* TCP acknowledge delay time */
#define ABP_SOC_IA_TCP_NODELAY            13    /* Disable Nagle's algorithm  */
#define ABP_SOC_IA_TCP_CONNTIMEO          14    /* Connect timeout            */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Socket Interface object instance attributes
** (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_SOC_IA_SOCK_TYPE_DS                    ABP_UINT8_SIZEOF
#define ABP_SOC_IA_LOCAL_PORT_DS                   ABP_UINT16_SIZEOF
#define ABP_SOC_IA_HOST_IP_DS                      ABP_UINT32_SIZEOF
#define ABP_SOC_IA_HOST_PORT_DS                    ABP_UINT16_SIZEOF
#define ABP_SOC_IA_TCP_STATE_DS                    ABP_UINT8_SIZEOF
#define ABP_SOC_IA_RX_BYTES_DS                     ABP_UINT16_SIZEOF
#define ABP_SOC_IA_TX_BYTES_DS                     ABP_UINT16_SIZEOF
#define ABP_SOC_IA_SO_REUSE_ADDR_DS                ABP_BOOL_SIZEOF
#define ABP_SOC_IA_SO_KEEP_ALIVE_DS                ABP_BOOL_SIZEOF
#define ABP_SOC_IA_IP_MULT_TTL_DS                  ABP_UINT8_SIZEOF
#define ABP_SOC_IA_IP_MULT_LOOP_DS                 ABP_BOOL_SIZEOF
#define ABP_SOC_IA_TCP_ACKDELAYTIME_DS             ABP_UINT16_SIZEOF
#define ABP_SOC_IA_TCP_NODELAY_DS                  ABP_BOOL_SIZEOF
#define ABP_SOC_IA_TCP_CONNTIMEO_DS                ABP_UINT16_SIZEOF

/*------------------------------------------------------------------------------
**
** The Socket Interface object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_SOC_CMD_BIND                           0x10
#define ABP_SOC_CMD_SHUTDOWN                       0x11
#define ABP_SOC_CMD_LISTEN                         0x12
#define ABP_SOC_CMD_ACCEPT                         0x13
#define ABP_SOC_CMD_CONNECT                        0x14
#define ABP_SOC_CMD_RECEIVE                        0x15
#define ABP_SOC_CMD_RECEIVE_FROM                   0x16
#define ABP_SOC_CMD_SEND                           0x17
#define ABP_SOC_CMD_SEND_TO                        0x18
#define ABP_SOC_CMD_ADD_MULTICAST_MEMBERSHIP       0x19
#define ABP_SOC_CMD_DROP_MULTICAST_MEMBERSHIP      0x1A
#define ABP_SOC_CMD_DNS_LOOKUP                     0x1B


/*------------------------------------------------------------------------------
**
** Socket types.
**
**------------------------------------------------------------------------------
*/

#define SOC_TYPE_NB_TCP_SOCKET            0  /* Non-blocking TCP              */
#define SOC_TYPE_B_TCP_SOCKET             1  /* Blocking TCP                  */
#define SOC_TYPE_NB_UDP_SOCKET            2  /* Non-blocking UDP              */
#define SOC_TYPE_B_UDP_SOCKET             3  /* Blocking UDP                  */
#define SOC_TYPE_RESERVED                 4  /* Reserved socket type.         */
                                             /* This type is used to reserve  */
                                             /* an instance for later use.    */


/*------------------------------------------------------------------------------
**
** TCP socket states.
** For more information see TCP statemachine (RFC 793)
**
**------------------------------------------------------------------------------
*/

#define SOC_TCP_STATE_CLOSED                          0
#define SOC_TCP_STATE_LISTEN                          1
#define SOC_TCP_STATE_SYN_SENT                        2
#define SOC_TCP_STATE_SYN_RECEIVED                    3
#define SOC_TCP_STATE_ESTABLISHED                     4
#define SOC_TCP_STATE_CLOSE_WAIT                      5
#define SOC_TCP_STATE_FIN_WAIT_1                      6
#define SOC_TCP_STATE_CLOSING                         7
#define SOC_TCP_STATE_LAST_ACK                        8
#define SOC_TCP_STATE_FIN_WAIT_2                      9
#define SOC_TCP_STATE_TIME_WAIT                      10


/*------------------------------------------------------------------------------
**
** Shutdown "how" types.
**
**------------------------------------------------------------------------------
*/

#define SOC_SHUTDOWN_RECV                             0
#define SOC_SHUTDOWN_SEND                             1
#define SOC_SHUTDOWN_BOTH                             2


/*------------------------------------------------------------------------------
**
** Socket object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define SOC_ERR_ENOBUFS                               1
#define SOC_ERR_ETIMEDOUT                             2
#define SOC_ERR_EISCONN                               3
#define SOC_ERR_EOPNOTSUPP                            4
#define SOC_ERR_ECONNABORTED                          5
#define SOC_ERR_EWOULDBLOCK                           6
#define SOC_ERR_ECONNREFUSED                          7
#define SOC_ERR_ECONNRESET                            8
#define SOC_ERR_ENOTCONN                              9
#define SOC_ERR_EALREADY                             10
#define SOC_ERR_EINVAL                               11
#define SOC_ERR_EMSGSIZE                             12
#define SOC_ERR_EPIPE                                13
#define SOC_ERR_EDESTADDRREQ                         14
#define SOC_ERR_ESHUTDOWN                            15

#define SOC_ERR_EHAVEOOB                             17
#define SOC_ERR_ENOMEM                               18
#define SOC_ERR_EADDRNOTAVAIL                        19
#define SOC_ERR_EADDRINUSE                           20

#define SOC_ERR_EINPROGRESS                          22

#define SOC_ERR_ETOOMANYREFS                         28

#define SOC_ERR_CMD_ABORTED                         101
#define SOC_ERR_DNS_NAME                            102
#define SOC_ERR_DNS_TIMEOUT                         103
#define SOC_ERR_DNS_CMD_FAILED                      104


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_soc.h
**
********************************************************************************
*/

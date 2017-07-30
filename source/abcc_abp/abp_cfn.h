/*******************************************************************************
********************************************************************************
** COPYRIGHT NOTIFICATION (c) 2016 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorized company official.                                         **
********************************************************************************
********************************************************************************
** Anybus-CC Protocol - CC-Link IE Field Network Definitions.
**
** This software component contains protocol definitions used by the CC-Link IE
** Field Network Anybus-CC module as well as applications designed to use such
** modules.
**
** The generic portion used by all Anybus-CC modules is available in the file
** ABP.H.
********************************************************************************
********************************************************************************
** Services:
********************************************************************************
********************************************************************************
*/

#ifndef ABP_CFN_H
#define ABP_CFN_H

/*******************************************************************************
** Constants
********************************************************************************
*/
/*------------------------------------------------------------------------------
** CC-Link IE Field Network object instance attributes
**------------------------------------------------------------------------------
*/
#define ABP_CFN_IA_VENDOR_CODE      1
#define ABP_CFN_IA_VENDOR_NAME      2
#define ABP_CFN_IA_MODEL_TYPE       3
#define ABP_CFN_IA_MODEL_NAME       4
#define ABP_CFN_IA_MODEL_CODE       5
#define ABP_CFN_IA_SW_VERSION       6
#define ABP_CFN_IA_ENABLE_SLMP      7
#define ABP_CFN_IA_ENA_SLMP_FORWARD 8

/*------------------------------------------------------------------------------
** The data size of the CC-Link IE Field Network object instance attributes
** (in bytes).
**------------------------------------------------------------------------------
*/
#define ABP_CFN_VENDOR_NAME_STR_LEN 31
#define ABP_CFN_MODEL_NAME_STR_LEN  19

#define ABP_CFN_IA_VENDOR_CODE_DS      ABP_UINT16_SIZEOF
#define ABP_CFN_IA_VENDOR_NAME_DS      ( ABP_CHAR_SIZEOF * ABP_CFN_VENDOR_NAME_STR_LEN )
#define ABP_CFN_IA_MODEL_TYPE_DS       ABP_UINT16_SIZEOF
#define ABP_CFN_IA_MODEL_NAME_DS       ( ABP_CHAR_SIZEOF * ABP_CFN_MODEL_NAME_STR_LEN )
#define ABP_CFN_IA_MODEL_CODE_DS       ABP_UINT32_SIZEOF
#define ABP_CFN_IA_SW_VERSION_DS       ABP_UINT8_SIZEOF
#define ABP_CFN_IA_ENABLE_SLMP_DS      ABP_UINT8_SIZEOF
#define ABP_CFN_IA_ENA_SLMP_FORWARD_DS ABP_UINT8_SIZEOF

/*------------------------------------------------------------------------------
** CC-Link IE Field Network object specific commands
**------------------------------------------------------------------------------
*/
#define ABP_CFN_CMD_BUF_SIZE_NOTIF  0x10
#define ABP_CFN_CMD_SLMP_SERVER_REQ 0x11

#endif  /* inclusion lock */

/*******************************************************************************
********************************************************************************
**                                                                            **
** ABP version 7.16.01 (2015-10-14)                                           **
**                                                                            */
/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_pnio.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_pnio.h - Anybus-CC Protocol Definitions for PROFINET Object.
**
** This file contains network specific definitions used by the Anybus-CC
** PROFINET module as well as applications designed to use such module.
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

#ifndef ABP_PNIO_H
#define ABP_PNIO_H


/*******************************************************************************
**
** Anybus-CC PROFINET object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC PROFINET Object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_PNIO_IA_DEVICE_ID                   1
#define ABP_PNIO_IA_VENDOR_ID                   2
#define ABP_PNIO_IA_STATION_TYPE                3
#define ABP_PNIO_IA_MAX_AR                      4
/*
** Instance attributes 5-6 are reserved
*/
#define ABP_PNIO_IA_RTM                         7
#define ABP_PNIO_IA_IM_ORDER_ID                 8
#define ABP_PNIO_IA_IM_SERIAL_NBR               9
#define ABP_PNIO_IA_IM_HW_REV                   10
#define ABP_PNIO_IA_IM_SW_REV                   11
#define ABP_PNIO_IA_IM_REV_CNT                  12
#define ABP_PNIO_IA_IM_PROFILE_ID               13
#define ABP_PNIO_IA_IM_PROFILE_SPEC_TYPE        14
#define ABP_PNIO_IA_IM_VER                      15
#define ABP_PNIO_IA_IM_SUPPORTED                16
#define ABP_PNIO_IA_PORT1_MAC_ADDRESS           17
#define ABP_PNIO_IA_PORT2_MAC_ADDRESS           18
#define ABP_PNIO_IA_SYSTEM_DESCRIPTION          19
#define ABP_PNIO_IA_INTERFACE_DESCRIPTION       20
#define ABP_PNIO_IA_MOD_ID_ASSIGN_MODE          21
#define ABP_PNIO_IA_SYSTEM_CONTACT              22
#define ABP_PNIO_IA_PROFIENERGY_FUNC            23

/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC PROFINET Object instance attributes (in
** bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_PNIO_IA_DEVICE_ID_DS                   ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_VENDOR_ID_DS                   ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_STATION_TYPE_DS                ( 25 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_MAX_AR_DS                      ABP_UINT32_SIZEOF
#define ABP_PNIO_IA_RTM_DS                         ABP_UINT8_SIZEOF
#define ABP_PNIO_IA_IM_ORDER_ID_DS                 ( 20 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_IM_SERIAL_NBR_DS               ( 16 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_IM_HW_REV_DS                   ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_IM_SW_REV_DS                   ( 4 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_IM_REV_CNT_DS                  ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_IM_PROFILE_ID_DS               ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_IM_PROFILE_SPEC_TYPE_DS        ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_IM_VER_DS                      ( 2 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_IM_SUPPORTED_DS                ABP_UINT16_SIZEOF
#define ABP_PNIO_IA_PORT1_MAC_ADDRESS_DS           ( 6 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_PORT2_MAC_ADDRESS_DS           ( 6 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_SYSTEM_DESCRIPTION_DS          ( 255 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_INTERFACE_DESCRIPTION_DS       ( 255 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_MOD_ID_ASSIGN_MODE_DS          ( ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_SYSTEM_CONTACT_DS              ( 255 * ABP_UINT8_SIZEOF )
#define ABP_PNIO_IA_PROFIENERGY_FUNC_DS            ( ABP_UINT8_SIZEOF )

/*------------------------------------------------------------------------------
**
** The PROFINET IO object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_PNIO_CMD_GET_RECORD                    0x10
#define ABP_PNIO_CMD_SET_RECORD                    0x11
#define ABP_PNIO_CMD_GET_IM_RECORD                 0x12
#define ABP_PNIO_CMD_SET_IM_RECORD                 0x13
#define ABP_PNIO_CMD_AR_CHECK_IND                  0x14
#define ABP_PNIO_CMD_CFG_MISMATCH_IND              0x15
#define ABP_PNIO_CMD_AR_INFO_IND                   0x16
#define ABP_PNIO_CMD_END_OF_PRM_IND                0x17
#define ABP_PNIO_CMD_AR_ABORT_IND                  0x19
#define ABP_PNIO_CMD_PLUG_SUB_FAILED               0x1A
#define ABP_PNIO_CMD_EXPECTED_IDENT_IND            0x1B
#define ABP_PNIO_CMD_SAVE_IP_SUITE                 0x1C
#define ABP_PNIO_CMD_SAVE_STATION_NAME             0x1D

/*------------------------------------------------------------------------------
**
** The data size of the PROFINET Object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_PNIO_CMD_GET_RECORD_DS                 11
#define ABP_PNIO_CMD_SET_RECORD_DS                 255
#define ABP_PNIO_CMD_GET_IM_RECORD_DS              4
#define ABP_PNIO_CMD_SET_IM_RECORD_DS              255
#define ABP_PNIO_CMD_AR_CHECK_IND_DS               255
#define ABP_PNIO_CMD_CFG_MISMATCH_IND_DS           16
#define ABP_PNIO_CMD_AR_INFO_IND_DS                255
#define ABP_PNIO_CMD_END_OF_PRM_IND_DS             8
#define ABP_PNIO_CMD_AR_ABORT_IND_DS               2
#define ABP_PNIO_CMD_PLUG_SUB_FAILED_DS            8
#define ABP_PNIO_CMD_EXPECTED_IDENT_IND_DS         255

/*------------------------------------------------------------------------------
**
** Response codes for Expected_Ident_Ind
**
**------------------------------------------------------------------------------
*/

#define ABP_PNIO_RSP_EXPECTED_IDENT_IND_CONT   0
#define ABP_PNIO_RSP_EXPECTED_IDENT_IND_BLOCK  1

/*------------------------------------------------------------------------------
**
** PROFINET IO specific exception information codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_PNIO_NW_EXCPT_ILLEGAL_VALUE         1
#define ABP_PNIO_NW_EXCPT_WRONG_DATA_SIZE       2
#define ABP_PNIO_NW_EXCPT_ILLEGAL_RSP           3
#define ABP_PNIO_NW_EXCPT_MISSING_MAC_ADDRESS   4
#define ABP_PNIO_NW_EXCPT_CMD_TIMEOUT           5

#endif  /* inclusion lock */

/*******************************************************************************
**
** end of abp_pnio.h
**
********************************************************************************
*/

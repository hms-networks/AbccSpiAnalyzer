/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_nwpnio.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_nwpnio.h - Anybus-CC Network PROFINET Object Protocol Definitions.
**
** This software component contains NWPNIO definitions used by Anybus-CC
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

#ifndef ABP_NWPNIO_H
#define ABP_NWPNIO_H


/*******************************************************************************
**
** Anybus-CC Network PROFINET Interface object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Network PROFINET Object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_NWPNIO_IA_ONLINE_TRANS               1     /* Number of on-line transitions    */
#define ABP_NWPNIO_IA_OFFLINE_TRANS              2     /* Number of off-line transitions   */
#define ABP_NWPNIO_IA_OFFLINE_REASON_CODE        3     /* Reason code of last off-line     */
#define ABP_NWPNIO_IA_ABORT_REASON_CODE          4     /* Last abort reason code           */
#define ABP_NWPNIO_IA_ADDED_APIS                 5     /* Number of added apis             */
#define ABP_NWPNIO_IA_API_LIST                   6     /* List of the added APIs           */
#define ABP_NWPNIO_IA_EST_ARS                    7     /* Number of established ARs        */
#define ABP_NWPNIO_IA_AR_LIST                    8     /* List of established ARs (handles)*/
#define ABP_NWPNIO_IA_PNIO_INIT_ERR_CODE         9     /* Error code PROFINET IO stack init*/
#define ABP_NWPNIO_IA_PORT1_MAC_ADDRESS          10    /* PROFINET IO port 1 MAC address   */
#define ABP_NWPNIO_IA_PORT2_MAC_ADDRESS          11    /* PROFINET IO port 2 MAC address   */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Network PROFINET Object instance attributes
** (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_NWPNIO_IA_ONLINE_TRANS_DS            ( ABP_UINT32_SIZEOF )
#define ABP_NWPNIO_IA_OFFLINE_TRANS_DS           ( ABP_UINT32_SIZEOF )
#define ABP_NWPNIO_IA_OFFLINE_REASON_CODE_DS     ( ABP_UINT16_SIZEOF )
#define ABP_NWPNIO_IA_ABORT_REASON_CODE_DS       ( ABP_UINT16_SIZEOF )
#define ABP_NWPNIO_IA_ADDED_APIS_DS              ( ABP_UINT16_SIZEOF )
#define ABP_NWPNIO_IA_API_LIST_DS                ( 2 * ABP_UINT32_SIZEOF )
#define ABP_NWPNIO_IA_EST_ARS_DS                 ( ABP_UINT16_SIZEOF )
#define ABP_NWPNIO_IA_AR_LIST_DS                 ( 3 * ABP_UINT16_SIZEOF )
#define ABP_NWPNIO_IA_PNIO_INIT_ERR_CODE_DS      ( ABP_UINT16_SIZEOF )
#define ABP_NWPNIO_IA_PORT1_MAC_ADDRESS_DS       ( 6 * ABP_UINT8_SIZEOF )
#define ABP_NWPNIO_IA_PORT2_MAC_ADDRESS_DS       ( 6 * ABP_UINT8_SIZEOF )


/*------------------------------------------------------------------------------
**
** The Anybus-CC Network PROFINET Object Interface object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_NWPNIO_CMD_PLUG_MODULE                 0x10
#define ABP_NWPNIO_CMD_PLUG_SUB_MODULE             0x11
#define ABP_NWPNIO_CMD_PULL_MODULE                 0x12
#define ABP_NWPNIO_CMD_PULL_SUB_MODULE             0x13
#define ABP_NWPNIO_CMD_API_ADD                     0x14
#define ABP_NWPNIO_CMD_APPL_STATE_READY            0x15
#define ABP_NWPNIO_CMD_AR_ABORT                    0x16
#define ABP_NWPNIO_CMD_ADD_SAFETY_MODULE           0x17
#define ABP_NWPNIO_CMD_IM_OPTIONS                  0x18
#define ABP_NWPNIO_CMD_PLUG_SUB_MODULE_EXT         0x19
#define ABP_NWPNIO_CMD_IDENT_CHANGE_DONE           0x1A


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC Network PROFINET Object Interface object
** specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_NWPNIO_CMD_PLUG_MODULE_DS              10
#define ABP_NWPNIO_CMD_PLUG_SUB_MODULE_DS          18
#define ABP_NWPNIO_CMD_PULL_MODULE_DS              6
#define ABP_NWPNIO_CMD_PULL_SUB_MODULE_DS          8
#define ABP_NWPNIO_CMD_API_ADD_DS                  8
#define ABP_NWPNIO_CMD_APPL_STATE_READY_DS         0
#define ABP_NWPNIO_CMD_AR_ABORT_DS                 0
#define ABP_NWPNIO_CMD_ADD_SAFETY_MODULE_DS        4
#define ABP_NWPNIO_CMD_IM_OPTIONS_DS               255
#define ABP_NWPNIO_CMD_PLUG_SUB_MODULE_EXT_DS      20
#define ABP_NWPNIO_CMD_IDENT_CHANGE_DONE_DS        0


/*------------------------------------------------------------------------------
**
** Anybus-CC Network PROFINET Object PlugSubmodule CmdExt0 bit masks
**
**------------------------------------------------------------------------------
*/
#define ABP_NWPNIO_PLUG_SUBMODULE_CMDEXT0_SAFETY_BIT     0x01


/*------------------------------------------------------------------------------
**
** The Anybus-CC Network PROFINET Object Interface object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_NWPNIO_ERR_ADI_WRITE_NOT_MAPPED        0x01
#define ABP_NWPNIO_ERR_ADI_READ_NOT_MAPPED         0x02
#define ABP_NWPNIO_ERR_ADI_ELEM_NOT_PRESENT        0x03
#define ABP_NWPNIO_ERR_ADI_ALREADY_MAPPED          0x04
#define ABP_NWPNIO_ERR_API_0_NOT_ADDED             0x05
#define ABP_NWPNIO_ERR_API_NOT_PRESENT             0x06
#define ABP_NWPNIO_ERR_API_ALREADY_PRESENT         0x07
#define ABP_NWPNIO_ERR_API_CANNOT_BE_ADDED         0x08
#define ABP_NWPNIO_ERR_NO_IO_IN_SLOT_0             0x09
#define ABP_NWPNIO_ERR_SLOT_0_NOT_PROP_PLUGGED     0x0A
#define ABP_NWPNIO_ERR_SLOT_OCCUPIED               0x0B
#define ABP_NWPNIO_ERR_SUBSLOT_OCCUPIED            0x0C
#define ABP_NWPNIO_ERR_NO_MODULE_SPECIFIED_SLOT    0x0D
#define ABP_NWPNIO_ERR_NO_SUBMOD_SPECIFIED_SLOT    0x0E
#define ABP_NWPNIO_ERR_SLOT_OUT_OF_RANGE           0x0F
#define ABP_NWPNIO_ERR_SUBSLOT_OUT_OF_RANGE        0x10
#define ABP_NWPNIO_ERR_AR_NOT_VALID                0x11
#define ABP_NWPNIO_ERR_NO_PEND_APPL_READY          0x12
#define ABP_NWPNIO_ERR_UNKNOWN_STACK_ERROR         0x13
#define ABP_NWPNIO_ERR_MAX_NBR_OF_PLUGGED_SUBMOD   0x14
#define ABP_NWPNIO_ERR_SAFETY_NOT_ENABLED          0x15
#define ABP_NWPNIO_ERR_ADI_DATATYPE_CONSTRAINT     0x16
#define ABP_NWPNIO_ERR_ASM_ALREADY_PLUGGED         0x17


#endif  /* inclusion lock */

/*******************************************************************************
**
** end of abp_nwpnio.h
**
********************************************************************************
*/

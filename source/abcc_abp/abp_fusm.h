/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_fusm.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_fusm.h - Anybus-CC Functional Safety Module Object Protocol Definitions.
**
** This software component contains FUSM definitions used by Anybus-CC
** modules as well as applications designed to use Functional Safety.
**
** This describes the safety object residing in the Anybus.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2012 HMS Industrial Networks AB                 **
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

#ifndef ABP_FUSM_H
#define ABP_FUSM_H


/*------------------------------------------------------------------------------
**
** The Functional Safety Module instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_FUSM_IA_STATE                 1
#define ABP_FUSM_IA_VENDOR_ID             2
#define ABP_FUSM_IA_MODULE_ID             3
#define ABP_FUSM_IA_FW_VERSION            4
#define ABP_FUSM_IA_SERIAL_NUM            5
#define ABP_FUSM_IA_DATA_OUT              6
#define ABP_FUSM_IA_DATA_IN               7
#define ABP_FUSM_IA_ERROR_CNTRS           8
#define ABP_FUSM_IA_FATAL_EVENT           9
#define ABP_FUSM_IA_EXCPT_INFO            10
#define ABP_FUSM_IA_BL_VERSION            11
#define ABP_FUSM_IA_VENDOR_BLK_1          12
#define ABP_FUSM_IA_VENDOR_BLK_2          13


/*------------------------------------------------------------------------------
**
** The data size of the instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_FUSM_IA_STATE_DS              ABP_UINT8_SIZEOF
#define ABP_FUSM_IA_VENDOR_ID_DS          ABP_UINT16_SIZEOF
#define ABP_FUSM_IA_MODULE_ID_DS          ABP_UINT16_SIZEOF
#define ABP_FUSM_IA_FW_VERSION_DS         ( 3 * ABP_UINT8_SIZEOF )
#define ABP_FUSM_IA_SERIAL_NUM_DS         ABP_UINT32_SIZEOF
#define ABP_FUSM_IA_ERROR_CNTRS_DS        ( 4 * ABP_UINT16_SIZEOF )
#define ABP_FUSM_IA_EXCPT_INFO_DS         ABP_UINT8_SIZEOF
#define ABP_FUSM_IA_BL_VERSION_DS         ( 2 * ABP_UINT8_SIZEOF )


/*------------------------------------------------------------------------------
**
** Object specific error codes
**
**------------------------------------------------------------------------------
*/

#define ABP_FUSM_ERR_REJECT_BY_MODULE    0x01
#define ABP_FUSM_ERR_MODULE_RSP_FAULTY   0x02

#define ABP_FUSM_ERR_OBJ_SPEC_DS            2
#define ABP_FUSM_ERR_REJECT_BY_MODULE_DS    4


/*-----------------------------------------------------------------------
**
** ABP_FUSM_IA_ERROR_CNTRS_IDX_...
**
** Index for the different counters in the Error Counters attribute
** (array of UINT16)
**
**-----------------------------------------------------------------------
*/

#define ABP_FUSM_IA_ERROR_CNTRS_IDX_ABCC_DR   0
#define ABP_FUSM_IA_ERROR_CNTRS_IDX_ABCC_SE   1
#define ABP_FUSM_IA_ERROR_CNTRS_IDX_SAFE_DR   2
#define ABP_FUSM_IA_ERROR_CNTRS_IDX_SAFE_SE   3


/*------------------------------------------------------------------------------
**
** Values of the Exception Info attribute
**
**------------------------------------------------------------------------------
*/

#define ABP_FUSM_EXCPT_INFO_NONE                0
#define ABP_FUSM_EXCPT_INFO_BAUDRATE_NOT_SUPP   1
#define ABP_FUSM_EXCPT_INFO_NO_START_MSG        2
#define ABP_FUSM_EXCPT_INFO_UNEXP_MSG_LENGTH    3
#define ABP_FUSM_EXCPT_INFO_UNEXP_CMD_IN_RSP    4
#define ABP_FUSM_EXCPT_INFO_UNEXP_ERR_CODE      5
#define ABP_FUSM_EXCPT_INFO_APPL_NOT_FOUND      6
#define ABP_FUSM_EXCPT_INFO_INV_APPL_CRC        7
#define ABP_FUSM_EXCPT_INFO_NO_FLASH_ACCESS     8
#define ABP_FUSM_EXCPT_INFO_WRONG_UC_IN_RSP     9
#define ABP_FUSM_EXCPT_INFO_BL_TIMEOUT          10
#define ABP_FUSM_EXCPT_INFO_NW_SPEC_PRM_ERR     11
#define ABP_FUSM_EXCPT_INFO_INV_CFG_STRING      12
#define ABP_FUSM_EXCPT_INFO_UC_RSP_DIFFERS      13
#define ABP_FUSM_EXCPT_INFO_INCOMPAT_MODULE     14
#define ABP_FUSM_EXCPT_INFO_MAX_RETRANSMISSIONS 15
#define ABP_FUSM_EXCPT_INFO_FW_FILE_ERROR       16
#define ABP_FUSM_EXCPT_INFO_INV_CYCLE_TIME      17
#define ABP_FUSM_EXCPT_INFO_INV_SPDU_IN_SIZE    18
#define ABP_FUSM_EXCPT_INFO_INV_SPDU_OUT_SIZE   19
#define ABP_FUSM_EXCPT_INFO_BAD_FORMAT_IN_SPDU  20
#define ABP_FUSM_EXCPT_INFO_SAFE_MOD_INIT_FAIL  21


/*------------------------------------------------------------------------------
**
** The Anybus-CC Functional Safety Module object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_FUSM_CMD_ERROR_CONFIRMATION       0x10
#define ABP_FUSM_CMD_SET_IO_CFG_STRING        0x11
#define ABP_FUSM_CMD_GET_SAFETY_OUTPUT_PDU    0x12
#define ABP_FUSM_CMD_GET_SAFETY_INPUT_PDU     0x13


#endif  /* inclusion lock */

/*******************************************************************************
**
** end of abp_fusm.h
**
********************************************************************************
*/

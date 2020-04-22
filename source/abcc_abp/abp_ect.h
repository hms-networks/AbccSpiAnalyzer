/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_ect.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_ect - Anybus-CC Protocol - EtherCAT Definitions.
**
** This software component contains protocol definitions used by the EtherCAT
** Anybus-CC module as well as applications designed to use such modules.
**
** The generic portion used by all Anybus-CC modules is available in the file
** ABP.H.
**
********************************************************************************
********************************************************************************
**
** Services List
** -------------
**
** Public Services:
**
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

#ifndef ABP_ECT_H
#define ABP_ECT_H


/*******************************************************************************
**
** Anybus-CC EtherCAT object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Instance number where the attributes reside on the EtherCAT object.
**
**------------------------------------------------------------------------------
*/

#define ABP_ECT_OI_ID                     1


/*------------------------------------------------------------------------------
**
** EtherCAT object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ECT_IA_VENDOR_ID              1
#define ABP_ECT_IA_PRODUCT_CODE           2
#define ABP_ECT_IA_MAJOR_REV              3
#define ABP_ECT_IA_MINOR_REV              4
#define ABP_ECT_IA_SERIAL_NUMBER          5
#define ABP_ECT_IA_MANF_DEV_NAME          6
#define ABP_ECT_IA_MANF_HW_VER            7
#define ABP_ECT_IA_MANF_SW_VER            8
#define ABP_ECT_IA_ENUM_ADIS              9
#define ABP_ECT_IA_DEVICE_TYPE            10
#define ABP_ECT_IA_WR_PD_ASSY_INST_TRANS  11
#define ABP_ECT_IA_RD_PD_ASSY_INST_TRANS  12
#define ABP_ECT_IA_ADI_TRANS              13

#define ABP_ECT_IA_OBJ_SUB_TRANS          15
#define ABP_ECT_IA_ENABLE_FOE             16
#define ABP_ECT_IA_ENABLE_EOE             17
#define ABP_ECT_IA_CHANGE_SR_SWITCH       18
#define ABP_ECT_IA_SET_DEV_ID_AS_CSA      19
#define ABP_ECT_IA_ETHERCAT_STATE         20
#define ABP_ECT_IA_STATE_TIMEOUTS         21
#define ABP_ECT_IA_COMP_IDENT_LISTS       22
#define ABP_ECT_IA_FSOE_STATUS_IND        23
#define ABP_ECT_IA_CLEAR_IDENT_AL_STS     24
#define ABP_ECT_IA_SII_ORDER_NUM          25
#define ABP_ECT_IA_SII_DEV_NAME           26
#define ABP_ECT_IA_FOEDATA_ACK_DELAY      27
#define ABP_ECT_IA_DEF_TXPDO_ASSIGN       28
#define ABP_ECT_IA_DEF_RXPDO_ASSIGN       29
#define ABP_ECT_IA_SII_COE_DETAILS        30

/*------------------------------------------------------------------------------
**
** The data size of the EtherCAT object instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ECT_CFG_STR_LEN                     64

#define ABP_ECT_IA_VENDOR_ID_DS                 ABP_UINT32_SIZEOF
#define ABP_ECT_IA_PRODUCT_CODE_DS              ABP_UINT32_SIZEOF
#define ABP_ECT_IA_MAJOR_REV_DS                 ABP_UINT16_SIZEOF
#define ABP_ECT_IA_MINOR_REV_DS                 ABP_UINT16_SIZEOF
#define ABP_ECT_IA_SERIAL_NUMBER_DS             ABP_UINT32_SIZEOF
#define ABP_ECT_IA_MANF_DEV_NAME_MAX_DS         ( ABP_CHAR_SIZEOF * ABP_ECT_CFG_STR_LEN )
#define ABP_ECT_IA_MANF_HW_VER_MAX_DS           ( ABP_CHAR_SIZEOF * ABP_ECT_CFG_STR_LEN )
#define ABP_ECT_IA_MANF_SW_VER_MAX_DS           ( ABP_CHAR_SIZEOF * ABP_ECT_CFG_STR_LEN )
#define ABP_ECT_IA_ENUM_ADIS_MAX_DS             1524
#define ABP_ECT_IA_DEVICE_TYPE_DS               ABP_UINT32_SIZEOF
#define ABP_ECT_IA_WR_PD_ASSY_INST_TRANS_MAX_DS 1524
#define ABP_ECT_IA_RD_PD_ASSY_INST_TRANS_MAX_DS 1524
#define ABP_ECT_IA_ADI_TRANS_MAX_DS             1524
#define ABP_ECT_IA_OBJ_SUB_TRANS_MAX_DS         1524
#define ABP_ECT_IA_ENABLE_FOE_DS                ABP_BOOL_SIZEOF
#define ABP_ECT_IA_ENABLE_EOE_DS                ABP_BOOL_SIZEOF
#define ABP_ECT_IA_CHANGE_SR_SWITCH_DS          ABP_BOOL_SIZEOF
#define ABP_ECT_IA_SET_DEV_ID_AS_CSA_DS         ABP_BOOL_SIZEOF
#define ABP_ECT_IA_ETHERCAT_STATE_DS            ABP_UINT8_SIZEOF
#define ABP_ECT_IA_STATE_TIMEOUTS_DS            ( ABP_UINT32_SIZEOF * 4 )
#define ABP_ECT_IA_COMP_IDENT_LISTS_DS          ABP_BOOL_SIZEOF
#define ABP_ECT_IA_FSOE_STATUS_IND_DS           ABP_ENUM_SIZEOF
#define ABP_ECT_IA_CLEAR_IDENT_AL_STS_DS        ABP_BOOL_SIZEOF
#define ABP_ECT_IA_SII_ORDER_NUM_MAX_DS         ( ABP_CHAR_SIZEOF * ABP_ECT_CFG_STR_LEN )
#define ABP_ECT_IA_SII_DEV_NAME_MAX_DS          ( ABP_CHAR_SIZEOF * ABP_ECT_CFG_STR_LEN )
#define ABP_ECT_IA_FOEDATA_ACK_DELAY_DS         ABP_UINT16_SIZEOF
#define ABP_ECT_IA_DEF_TXPDO_ASSIGN_MAX_DS      ( ABP_UINT16_SIZEOF * 64 )
#define ABP_ECT_IA_DEF_RXPDO_ASSIGN_MAX_DS      ( ABP_UINT16_SIZEOF * 64 )
#define ABP_ECT_IA_SII_COE_DETAILS_DS           ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** The Anybus CompactCom EtherCAT Object specific commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_ECT_CMD_GET_OBJECT_DESC             0x10
#define ABP_ECT_CMD_GET_OBJECT_ACCESS           0x11
#define ABP_ECT_CMD_GET_DATA_TYPE               0x12
#define ABP_ECT_CMD_GET_ENUM_DATA               0x13


/*******************************************************************************
**
** Network object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network object instance attribute # 7 information
** EtherCAT specific exception constants
**
**------------------------------------------------------------------------------
*/

#define ABP_ECT_NW_EXCPT_ILLEGAL_DATA_TYPE                                1
#define ABP_ECT_NW_EXCPT_INSTANCE_BY_ORDER_ERROR                          2
#define ABP_ECT_NW_EXCPT_HIGHEST_INSTANCE_ERROR                           3
#define ABP_ECT_NW_EXCPT_NUMBER_OF_INSTANCES_ERROR                        4
#define ABP_ECT_NW_EXCPT_HIGHEST_INSTANCE_LOWER_THAN_NUMBER_OF_INSTANCES  5
#define ABP_ECT_NW_EXCPT_ASM_MAP_ERROR                                    6
#define ABP_ECT_NW_EXCPT_GET_INST_NUMBERS_ERROR                           7
#define ABP_ECT_NW_EXCPT_MDD_ERROR                                        8
#define ABP_ECT_NW_EXCPT_NO_MAC_ADDR                                      9

/*******************************************************************************
**
** Network configuration object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Network configuration instances common to most Anybus-CC modules.
**
** Note:
**       In case the values of these instances originate from input devices
**       controlled by the end user (DIP switches or similar), the application
**       shall keep these instances updated at all times because some networks
**       require that a changed switch is indicated by the LEDs.
**
**------------------------------------------------------------------------------
*/

#define ABP_NC_INST_NUM_DEVICE_ID       0x01

#define ABP_NC_INST_NUM_FSOE_ADDRESS    0x15

/*******************************************************************************
**
** Diagnostic object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Device specific emergency (EMCY) error codes.
** These should be used in combination with the ABP_DI_EVENT_NW_SPECIFIC
** diagnostic code.
**
**------------------------------------------------------------------------------
*/

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_ect.h
**
********************************************************************************
*/

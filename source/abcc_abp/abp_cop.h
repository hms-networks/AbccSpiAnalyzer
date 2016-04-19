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
** abp_cop.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_cop - Anybus-CC Protocol - CANopen Definitions.
**
** This software component contains protocol definitions used by the CANopen
** Anybus-CC module as well as applications designed to use such modules.
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

#ifndef ABP_COP_H
#define ABP_COP_H


/*******************************************************************************
**
** Anybus-CC CANopen object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** CANopen object instance number
**
**------------------------------------------------------------------------------
*/

#define ABP_COP_OI_ID                     1


/*------------------------------------------------------------------------------
**
** CANopen object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_COP_IA_VENDOR_ID              1
#define ABP_COP_IA_PRODUCT_CODE           2
#define ABP_COP_IA_MAJOR_REV              3
#define ABP_COP_IA_MINOR_REV              4
#define ABP_COP_IA_SERIAL_NUMBER          5
#define ABP_COP_IA_MANF_DEV_NAME          6
#define ABP_COP_IA_MANF_HW_VER            7
#define ABP_COP_IA_MANF_SW_VER            8


/*------------------------------------------------------------------------------
**
** The data size of the CANopen object instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_COP_CFG_STR_LEN               24

#define ABP_COP_IA_VENDOR_ID_DS           ABP_UINT32_SIZEOF
#define ABP_COP_IA_PRODUCT_CODE_DS        ABP_UINT32_SIZEOF
#define ABP_COP_IA_MAJOR_REV_DS           ABP_UINT16_SIZEOF
#define ABP_COP_IA_MINOR_REV_DS           ABP_UINT16_SIZEOF
#define ABP_COP_IA_SERIAL_NUMBER_DS       ABP_UINT32_SIZEOF
#define ABP_COP_IA_MANF_DEV_NAME_MAX_DS   ( ABP_CHAR_SIZEOF * ABP_COP_CFG_STR_LEN )
#define ABP_COP_IA_MANF_HW_VER_MAX_DS     ( ABP_CHAR_SIZEOF * ABP_COP_CFG_STR_LEN )
#define ABP_COP_IA_MANF_SW_VER_MAX_DS     ( ABP_CHAR_SIZEOF * ABP_COP_CFG_STR_LEN )


/*******************************************************************************
**
** Network object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network object instance attribute # 7 information
** CANopen specific exception constants
**
**------------------------------------------------------------------------------
*/

#define ABP_COP_NW_EXCPT_ILLEGAL_DATA_TYPE      1


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

/*
** The mappings stored in NV memory does not correspond to the ADI's currently
** mapped by the application.
*/

#define ABP_COP_ECY_NO_PD_MAP       0xFF01


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_cop.h
**
********************************************************************************
*/

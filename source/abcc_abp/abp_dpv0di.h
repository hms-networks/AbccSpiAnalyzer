/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_dpv0di.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_dpv0di - Anybus-CC Protocol - PROFIBUS DP-V0 Diagnostic object definitions
**
** This file contains network specific definitions used by the Anybus-CC
** PROFIBUS DP-V0 module as well as applications designed to use such module.
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

#ifndef ABP_DPV0DI_H
#define ABP_DPV0DI_H


/*******************************************************************************
**
** Anybus-CC PROFIBUS DP-V0 Diagnostic object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The PROFIBUS DP-V0 Diagnostic diagnostic object specific object attributes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV0DI_OA_EXT_DIAG_OVERFLOW  = 12,
   ABP_DPV0DI_OA_STATIC_DIAG        = 13
};


/*------------------------------------------------------------------------------
**
** The data size of the PROFIBUS DP-V0 Diagnostic diagnostic object specific object
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_DPV0DI_OA_MAX_INST_DS            ABP_UINT16_SIZEOF
#define ABP_DPV0DI_OA_EXT_DIAG_OVERFLOW_DS   ABP_UINT8_SIZEOF
#define ABP_DPV0DI_OA_STATIC_DIAG_DS         ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** PROFIBUS DP-V0 Diagnostic Diagnostic instance attributes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV0DI_IA_DIAG_DATA       = 1
};


/*------------------------------------------------------------------------------
**
** PROFIBUS DP-V0 Diagnostic Diagnostic object specific error codes
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV0DI_ERR_NO_ERROR                   = 0x00,
   ABP_DPV0DI_ERR_STD_DIAG_OBJ_USED          = 0x01,
   ABP_DPV0DI_ERR_INVALID_EXT_DIAG_FLAG      = 0x02,
   ABP_DPV0DI_ERR_ADD_DIAG_OBJ_USED          = 0x03
};


/*------------------------------------------------------------------------------
**
** Values of Extended Diagnostic Flag attribute
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_DPV0DI_EXT_DIAG_FLAG_DISABLE     = 0x00,
   ABP_DPV0DI_EXT_DIAG_FLAG_ENABLE      = 0x01
};


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_dpv0di.h
**
********************************************************************************
*/

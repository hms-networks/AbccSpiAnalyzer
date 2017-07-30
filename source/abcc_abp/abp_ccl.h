/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_ccl.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_ccl - Anybus-CC Protocol - CC-Link Definitions.
**
** This software component contains protocol definitions used by the CC-Link
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

#ifndef ABP_CCL_H
#define ABP_CCL_H


/*******************************************************************************
**
** Anybus-CC CC-Link object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** CC-Link object instance number
**
**------------------------------------------------------------------------------
*/

#define ABP_CCL_OI_ID                     1


/*------------------------------------------------------------------------------
**
** CC-Link object instance attributes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_CCL_IA_VENDOR_CODE            = 1,
   ABP_CCL_IA_SOFTWARE_VERSION       = 2,
   ABP_CCL_IA_MODEL_CODE             = 3,
   ABP_CCL_IA_NETWORK_SETTINGS       = 4,
   ABP_CCL_IA_SYS_AREA_HANDLER       = 5,
   ABP_CCL_IA_HOLD_CLEAR_SETTING     = 6
};


/*------------------------------------------------------------------------------
**
** The data size of the CC-Link object instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_CCL_IA_VENDOR_CODE_DS         ABP_UINT16_SIZEOF
#define ABP_CCL_IA_SOFTWARE_VERSION_DS    ABP_UINT8_SIZEOF
#define ABP_CCL_IA_MODEL_CODE_DS          ABP_UINT8_SIZEOF
#define ABP_CCL_IA_NETWORK_SETTINGS_DS    ( ABP_UINT8_SIZEOF +                \
                                            ABP_UINT8_SIZEOF +                \
                                            ABP_UINT8_SIZEOF )
#define ABP_CCL_IA_SYS_AREA_HANDLER_DS    ABP_SINT16_SIZEOF
#define ABP_CCL_IA_HOLD_CLEAR_SETTING_DS  ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** CC-Link object specific commands
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_CCL_CMD_INITIAL_DATA_SETTING_NOTIFICATION   = 0x10,
   ABP_CCL_CMD_INITIAL_DATA_PROCESSING_COMPLETED_NOTIFICATION
};


#endif  /* inclusion lock */

/*******************************************************************************
**
** end of abp_ccl.h
**
********************************************************************************
*/

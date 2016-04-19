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
** abp_safe.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_safe.h - Anybus-CC Functional Safety Object Protocol Definitions.
**
** This software component contains definitions used by Anybus-CC
** modules as well as applications designed to use Functional Safety.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
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

#ifndef ABP_SAFE_H_
#define ABP_SAFE_H_


/*------------------------------------------------------------------------------
**
** The Functional Safety instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SAFE_IA_SAFETY_ENABLE         1
#define ABP_SAFE_IA_BAUD_RATE             2


/*------------------------------------------------------------------------------
**
** The data size of the Functional Safety instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_SAFE_IA_SAFETY_ENABLE_DS      ABP_BOOL_SIZEOF
#define ABP_SAFE_IA_BAUD_RATE_DS          ABP_UINT32_SIZEOF


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_safe.h
**
********************************************************************************
*/

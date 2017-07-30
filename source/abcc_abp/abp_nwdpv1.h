/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_nwdpv1.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_nwdpv1 - Anybus-CC Protocol - Network PROFIBUS DP-V1 object definitions
**
** This file contains network specific definitions used by the Anybus-CC
** PROFIBUS module as well as applications designed to use such module.
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

#ifndef ABP_NWDPV1_H
#define ABP_NWDPV1_H


/*******************************************************************************
**
** Anybus-CC Network PROFIBUS DP-V1 object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Network PROFIBUS DP-V1 object specific message commands.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_NWDPV1_CMD_MAP_ADI_WRITE_AREA = 0x10,
   ABP_NWDPV1_CMD_MAP_ADI_READ_AREA  = 0x11
};


/*------------------------------------------------------------------------------
**
** The Network PROFIBUS DP-V1 object specific error codes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_NWDPV1_ERR_INVALID_ADI_DATA_TYPE   = 0x01,
   ABP_NWDPV1_ERR_INVALID_NUM_ELEMENTS    = 0x02,
   ABP_NWDPV1_ERR_INVALID_TOTAL_SIZE      = 0x03,
   ABP_NWDPV1_ERR_INVALID_ORDER_NUM       = 0x04,
   ABP_NWDPV1_ERR_INVALID_MAP_CMD_SEQ     = 0x05,
   ABP_NWDPV1_ERR_INVALID_CFG_DATA        = 0x06,
   ABP_NWDPV1_ERR_TOO_MUCH_TOTAL_CFG_DATA = 0x07
};


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_nwdpv1.h
**
********************************************************************************
*/

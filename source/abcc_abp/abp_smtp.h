/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_smtp.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC SMTP Client Interface Object Protocol Definitions.
**
** This software component contains SMTP definitions used by Anybus-CC
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

#ifndef ABP_SMTP_H
#define ABP_SMTP_H


/*******************************************************************************
**
** SMTP Client Interface object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The SMTP Client Interface object specific object attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SMTP_OA_MAX_INST              11    /* Max number of instances    */
#define ABP_SMTP_OA_EMAILS_SENT           12    /* Emails sent                */
#define ABP_SMTP_OA_EMAIL_FAILED          13    /* Emails failed to send      */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC SMTP Client Interface object specific
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_SMTP_OA_MAX_INST_DS            ABP_UINT16_SIZEOF
#define ABP_SMTP_OA_EMAILS_SENT_DS         ABP_UINT16_SIZEOF
#define ABP_SMTP_OA_EMAIL_FAILED_DS        ABP_UINT16_SIZEOF


/*------------------------------------------------------------------------------
**
** The SMTP Client Interface instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SMTP_IA_FROM                   1    /* From address               */
#define ABP_SMTP_IA_TO                     2    /* To address                 */
#define ABP_SMTP_IA_SUBJECT                3    /* Message subject            */
#define ABP_SMTP_IA_MESSAGE                4    /* Message body               */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC SMTP Client Interface object instance
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_SMTP_IA_FROM_DS               ( 255 * ABP_UINT8_SIZEOF )
#define ABP_SMTP_IA_TO_DS                 ( 255 * ABP_UINT8_SIZEOF )
#define ABP_SMTP_IA_SUBJECT_DS            ( 255 * ABP_UINT8_SIZEOF )
#define ABP_SMTP_IA_MESSAGE_DS            ( 255 * ABP_UINT8_SIZEOF )


/*------------------------------------------------------------------------------
**
** The SMTP Client Interface object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_SMTP_CMD_SEND_EMAIL                 0x10
#define ABP_SMTP_CMD_SEND_EMAIL_FROM_FILE       0x11


/*------------------------------------------------------------------------------
**
** SMTP Client Interface object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_SMTP_NO_EMAIL_SERVER                1
#define ABP_SMTP_SERVER_NOT_READY               2
#define ABP_SMTP_AUTHENTICATION_ERROR           3
#define ABP_SMTP_SOCKET_ERROR                   4
#define ABP_SMTP_SSI_SCAN_ERROR                 5
#define ABP_SMTP_FILE_ERROR                     6
#define ABP_SMTP_OTHER                        255


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_smtp.h
**
********************************************************************************
*/

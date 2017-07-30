/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_add.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** abp_add - Anybus-CC Protocol - Additional Diagnostic object definitions
** PROFIBUS specific constants for the Diagnostic object
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

#ifndef ABP_ADD_H
#define ABP_ADD_H


/*******************************************************************************
**
** Anybus-CC Additional Diagnostic object constants (ABCC30 only)
**
** Object revision: 2.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Additional diagnostic object specific object attributes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_OA_MAX_INST           = 11,
   ABP_ADD_OA_EXT_DIAG_OVERFLOW  = 12,
   ABP_ADD_OA_STATIC_DIAG        = 13
};


/*------------------------------------------------------------------------------
**
** The data size of the Additional diagnostic object specific object
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ADD_OA_MAX_INST_DS            ABP_UINT16_SIZEOF
#define ABP_ADD_OA_EXT_DIAG_OVERFLOW_DS   ABP_UINT8_SIZEOF
#define ABP_ADD_OA_STATIC_DIAG_DS         ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** Additional Diagnostic instance attributes.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_IA_MODULE_NUMBER   = 1,
   ABP_ADD_IA_IO_TYPE         = 2,
   ABP_ADD_IA_CHANNEL_NUMBER  = 3,
   ABP_ADD_IA_CHANNEL_TYPE    = 4,
   ABP_ADD_IA_ERROR_TYPE      = 5
};

/*------------------------------------------------------------------------------
**
** The max value of the module number
**
**------------------------------------------------------------------------------
*/

#define ABP_ADD_MODULE_NUMBER_MAX     0x3F


/*------------------------------------------------------------------------------
**
** The data size of the Additional Diagnostic instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ADD_IA_MODULE_NUMBER_DS    ABP_UINT8_SIZEOF
#define ABP_ADD_IA_IO_TYPE_DS          ABP_UINT8_SIZEOF
#define ABP_ADD_IA_CHANNEL_NUMBER_DS   ABP_UINT8_SIZEOF
#define ABP_ADD_IA_CHANNEL_TYPE_DS     ABP_UINT8_SIZEOF
#define ABP_ADD_IA_ERROR_TYPE_DS       ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** The Additional Diagnostic object specific message commands.
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_CMD_ALARM_NOTIFICATION = 0x10
};


/*------------------------------------------------------------------------------
**
** Additional Diagnostic object specific error codes
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_ERR_NO_ERROR                   = 0x00,
   ABP_ADD_ERR_INVALID_SLOT_NUMBER        = 0x01,
   ABP_ADD_ERR_INVALID_IO_TYPE            = 0x02,
   ABP_ADD_ERR_INVALID_CHANNEL_NUMBER     = 0x03,
   ABP_ADD_ERR_INVALID_CHANNEL_TYPE       = 0x04,
   ABP_ADD_ERR_INVALID_ERROR_TYPE         = 0x05,
   ABP_ADD_ERR_INVALID_ALARM_SPECIFIER    = 0x06,
   ABP_ADD_ERR_ALARM_TYPE_DISABLED        = 0x07,
   ABP_ADD_ERR_TOO_MANY_ACTIVE_ALARMS     = 0x08,
   ABP_ADD_ERR_ALARM_TYPE_ALREADY_ACTIVE  = 0x09
};


/*------------------------------------------------------------------------------
**
** Value range of slot number (instance attribute and parameter in
** Alarm Notification command)
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_SLOT_NUMBER_MIN = 0x00,
   ABP_ADD_SLOT_NUMBER_MAX = 0xFE
};


/*------------------------------------------------------------------------------
**
** Values of IO type attribute
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_IO_TYPE_INPUT         = 0x01,
   ABP_ADD_IO_TYPE_OUTPUT        = 0x02,
   ABP_ADD_IO_TYPE_INPUT_OUTPUT  = 0x03
};


/*------------------------------------------------------------------------------
**
** Value range of channel number attribute
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_CHANNEL_NUMBER_MIN = 0x00,
   ABP_ADD_CHANNEL_NUMBER_MAX = 0x3F
};


/*------------------------------------------------------------------------------
**
** Values of Channel type attribute
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_CHANNEL_TYPE_BIT      = 0x01,
   ABP_ADD_CHANNEL_TYPE_2_BITS   = 0x02,
   ABP_ADD_CHANNEL_TYPE_4_BITS   = 0x03,
   ABP_ADD_CHANNEL_TYPE_BYTE     = 0x04,
   ABP_ADD_CHANNEL_TYPE_WORD     = 0x05,
   ABP_ADD_CHANNEL_TYPE_2_WORDS  = 0x06
};


/*------------------------------------------------------------------------------
**
** Values of Error type attribute
**
** 0x00       : Reserved
** 0x01 - 0x09: Defined error codes in PROFIBUS specification
** 0x0A - 0x0F: Reserved
** 0x10 - 0x1F: Manufacturer specific
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_ERROR_TYPE_SHORT_CIRCUIT          = 0x01,
   ABP_ADD_ERROR_TYPE_UNDER_VOLTAGE          = 0x02,
   ABP_ADD_ERROR_TYPE_OVER_VOLTAGE           = 0x03,
   ABP_ADD_ERROR_TYPE_OVERLOAD               = 0x04,
   ABP_ADD_ERROR_TYPE_OVER_TEMPERATURE       = 0x05,
   ABP_ADD_ERROR_TYPE_WIRE_BREAK             = 0x06,
   ABP_ADD_ERROR_TYPE_UPPER_LIMIT_EXCEEDED   = 0x07,
   ABP_ADD_ERROR_TYPE_LOWER_LIMIT_EXCEEDED   = 0x08,
   ABP_ADD_ERROR_TYPE_ERROR_GENERAL          = 0x09,

   ABP_ADD_ERROR_TYPE_MANUF_MIN              = 0x10,
   ABP_ADD_ERROR_TYPE_MANUF_MAX              = 0x1F
};


/*------------------------------------------------------------------------------
**
** Values of Alarm type (part of Alarm Notification command)
**
** 0x00       : Reserved
** 0x01 - 0x06: Defined alarm types in PROFIBUS specification
** 0x07 - 0x1F: Reserved
** 0x20 - 0x7E: Manufacturer specific
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_ALARM_TYPE_DIAGNOSIS  = 0x01,
   ABP_ADD_ALARM_TYPE_PROCESS    = 0x02,
   ABP_ADD_ALARM_TYPE_PULL       = 0x03,
   ABP_ADD_ALARM_TYPE_PLUG       = 0x04,
   ABP_ADD_ALARM_TYPE_STATUS     = 0x05,
   ABP_ADD_ALARM_TYPE_UPDATE     = 0x06,

   ABP_ADD_ALARM_TYPE_MANUF_MIN  = 0x20,
   ABP_ADD_ALARM_TYPE_MANUF_MAX  = 0x7E
};


/*------------------------------------------------------------------------------
**
** Values of Alarm specifier (part of Alarm Notification command)
**
**------------------------------------------------------------------------------
*/

enum
{
   ABP_ADD_SPEC_NO_FURTHER_DIFF              = 0x00,
   ABP_ADD_SPEC_ERROR_APPEARS                = 0x01,
   ABP_ADD_SPEC_ERROR_DISAPPEARS_SLOT_OK     = 0x02,
   ABP_ADD_SPEC_ERROR_DISAPPEARS_SLOT_NOT_OK = 0x03
};


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_add.h
**
********************************************************************************
*/

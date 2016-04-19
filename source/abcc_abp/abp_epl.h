/*******************************************************************************
********************************************************************************
**                                                                            **
** ABP version 7.16.01 (2015-10-14)                                           **
**                                                                            */
/*******************************************************************************
********************************************************************************
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorized company official.                                         **
********************************************************************************
********************************************************************************
** Anybus-CC Protocol - POWERLINK Definitions.
**
** This software component contains protocol definitions used by the POWERLINK
** Anybus-CC module as well as applications designed to use such modules.
**
** The generic portion used by all Anybus-CC modules is available in the file
** ABP.H.
********************************************************************************
********************************************************************************
** Services:
********************************************************************************
********************************************************************************
*/

#ifndef ABP_EPL_H
#define ABP_EPL_H


/*******************************************************************************
**
** Anybus-CC POWERLINK object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** POWERLINK object instance number
**
**------------------------------------------------------------------------------
*/

#define ABP_EPL_OI_ID                     1


/*------------------------------------------------------------------------------
**
** POWERLINK object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_EPL_IA_VENDOR_ID              1
#define ABP_EPL_IA_PRODUCT_CODE           2
#define ABP_EPL_IA_MAJOR_REV              3
#define ABP_EPL_IA_MINOR_REV              4
#define ABP_EPL_IA_SERIAL_NUMBER          5
#define ABP_EPL_IA_MANF_DEV_NAME          6
#define ABP_EPL_IA_MANF_HW_VER            7
#define ABP_EPL_IA_MANF_SW_VER            8

#define ABP_EPL_IA_DEVICE_TYPE            10

#define ABP_EPL_IA_MANF_NAME              14


/*------------------------------------------------------------------------------
**
** The data size of the POWERLINK object instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_EPL_CFG_STR_LEN               64

#define ABP_EPL_IA_VENDOR_ID_DS           ABP_UINT32_SIZEOF
#define ABP_EPL_IA_PRODUCT_CODE_DS        ABP_UINT32_SIZEOF
#define ABP_EPL_IA_MAJOR_REV_DS           ABP_UINT16_SIZEOF
#define ABP_EPL_IA_MINOR_REV_DS           ABP_UINT16_SIZEOF
#define ABP_EPL_IA_SERIAL_NUMBER_DS       ABP_UINT32_SIZEOF
#define ABP_EPL_IA_MANF_DEV_NAME_MAX_DS   ( ABP_CHAR_SIZEOF * ABP_EPL_CFG_STR_LEN )
#define ABP_EPL_IA_MANF_HW_VER_MAX_DS     ( ABP_CHAR_SIZEOF * ABP_EPL_CFG_STR_LEN )
#define ABP_EPL_IA_MANF_SW_VER_MAX_DS     ( ABP_CHAR_SIZEOF * ABP_EPL_CFG_STR_LEN )

#define ABP_EPL_IA_DEVICE_TYPE_DS         ABP_UINT32_SIZEOF
#define ABP_EPL_IA_MANF_NAME_MAX_DS       ( ABP_CHAR_SIZEOF * ABP_EPL_CFG_STR_LEN )

/*******************************************************************************
**
** Network object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Network object instance attribute # 7 information
** POWERLINK specific exception constants
**
**------------------------------------------------------------------------------
*/

#define ABP_EPL_NW_EXCPT_GET_INST_NUMBERS_ERROR      7


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
**       require that a changed switch is indicated by the LED’s.
**
**------------------------------------------------------------------------------
*/

#define ABP_EPL_NC_INST_NUM_NODE_ID       0x01


#endif  /* inclusion lock */

/*******************************************************************************
********************************************************************************
** COPYRIGHT NOTIFICATION (c) 2016 HMS Industrial Networks AB                 **
**                                                                            **
** This code is the property of HMS Industrial Networks AB.                   **
** The source code may not be reproduced, distributed, or used without        **
** permission. When used together with a product from HMS, permission is      **
** granted to modify, reproduce and distribute the code in binary form        **
** without any restrictions.                                                  **
**                                                                            **
** THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT    **
** WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR     **
** THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR     **
** THAT DEFECTS IN IT CAN BE CORRECTED.                                       **

********************************************************************************
********************************************************************************
** This file contains Asset Management object definitions used ABCC modules
** as well as applications designed to describe such assets.
********************************************************************************
********************************************************************************
*/

#ifndef ABP_PNAM_H
#define ABP_PNAM_H

#include "abp.h"

/*------------------------------------------------------------------------------
** The PROFINET Asset Management object instance attributes.
**
** ABP_PNAM_IA_INFO_TYPE   - The type of asset described by this instance:
**                             0 : Hardware and firmware
**                             1 : Hardware only (Software revision irrelevant)
**                             2 : Firmware only (Hardware revision irrelevant)
** ABP_PNAM_IA_UNIQUE_ID   - Manufacturer created unique identifier.
** ABP_PNAM_IA_LOCATION_TYPE - Defines which location representation to use:
**                              1: LT = Twelve level tree format
**                              2: SS = Slot- and SubslotNumber format
** ABP_PNAM_IA_LOCATION_LT - Asset location according to "12 level tree format".
** ABP_PNAM_IA_LOCATION_SS - Asset location according to "Slot- and Subslot-
**                           Number format".
** ABP_PNAM_IA_ANNOTATION  - Textual description of the asset.
** ABP_PNAM_IA_ORDER_ID    - The manufacturer defined order id.
** ABP_PNAM_IA_SERIAL      - Serial number in string format
** ABP_PNAM_IA_DEVICE_ID   - Device identification (organization, vendor id,
**                           device id and device sub id)
** ABP_PNAM_IA_TYPE_ID     - Asset type identification according to standard or
**                           manufacturer specific definitions.
** ABP_PNAM_IA_AM_SW_REV   - Software revision in string format
** ABP_PNAM_IA_IM_SW_REV   - Software revision in PROFINET format (e.g. V1.2.3)
** ABP_PNAM_IA_AM_HW_REV   - Hardware revision in string format
** ABP_PNAM_IA_IM_HW_REV   - Hardware revision in UINT16 format.
**------------------------------------------------------------------------------
*/
#define ABP_PNAM_IA_INFO_TYPE       1
#define ABP_PNAM_IA_UNIQUE_ID       2
#define ABP_PNAM_IA_LOCATION_TYPE   3
#define ABP_PNAM_IA_LOCATION_LT     4
#define ABP_PNAM_IA_LOCATION_SS     5
#define ABP_PNAM_IA_ANNOTATION      6
#define ABP_PNAM_IA_ORDER_ID        7
#define ABP_PNAM_IA_SERIAL          8
#define ABP_PNAM_IA_DEVICE_ID       9
#define ABP_PNAM_IA_TYPE_ID         10
#define ABP_PNAM_IA_AM_SW_REV       11
#define ABP_PNAM_IA_IM_SW_REV       12
#define ABP_PNAM_IA_AM_HW_REV       13
#define ABP_PNAM_IA_IM_HW_REV       14

/*------------------------------------------------------------------------------
** Data sizes of the instance attributes
**------------------------------------------------------------------------------
*/
#define ABP_PNAM_IA_INFO_TYPE_DS       ABP_UINT8_SIZEOF
#define ABP_PNAM_IA_UNIQUE_ID_DS       ( 16 * ABP_UINT8_SIZEOF )
#define ABP_PNAM_IA_LOCATION_TYPE_DS   ( ABP_UINT8_SIZEOF )
#define ABP_PNAM_IA_LOCATION_LT_DS     ( 12 * ABP_UINT16_SIZEOF )
#define ABP_PNAM_IA_LOCATION_SS_DS     ( 4 * ABP_UINT16_SIZEOF )
#define ABP_PNAM_IA_ANNOTATION_DS      ( 64 * ABP_CHAR_SIZEOF )
#define ABP_PNAM_IA_ORDER_ID_DS        ( 64 * ABP_CHAR_SIZEOF )
#define ABP_PNAM_IA_SERIAL_DS          ( 16 * ABP_CHAR_SIZEOF )
#define ABP_PNAM_IA_DEVICE_ID_DS       ( 4 * ABP_UINT16_SIZEOF )
#define ABP_PNAM_IA_TYPE_ID_DS         ABP_UINT16_SIZEOF
#define ABP_PNAM_IA_AM_SW_REV_DS       ( 64 * ABP_CHAR_SIZEOF )
#define ABP_PNAM_IA_IM_SW_REV_DS       ( ABP_CHAR_SIZEOF + 3 * ABP_UINT8_SIZEOF )
#define ABP_PNAM_IA_AM_HW_REV_DS       ( 64 * ABP_CHAR_SIZEOF )
#define ABP_PNAM_IA_IM_HW_REV_DS       ABP_UINT16_SIZEOF


/*------------------------------------------------------------------------------
** All valid values for ABP_PNAM_IA_INFO_TYPE attribute
**------------------------------------------------------------------------------
*/
#define ABP_PNAM_INFO_TYPE_FULL     0
#define ABP_PNAM_INFO_TYPE_HW_ONLY  1
#define ABP_PNAM_INFO_TYPE_FW_ONLY  2

/*------------------------------------------------------------------------------
** All valid values for ABP_PNAM_IA_LOCATION_TYPE attribute
**------------------------------------------------------------------------------
*/
#define ABP_PNAM_IA_LOCATION_TYPE_LT   1
#define ABP_PNAM_IA_LOCATION_TYPE_SS   2


#endif  /* inclusion lock */

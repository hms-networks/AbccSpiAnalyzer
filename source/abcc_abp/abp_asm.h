/*******************************************************************************
********************************************************************************
**                                                                            **
** ABP version 7.31.01 (2016-09-16)                                           **
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
** This file contains Assembly Mapping Object specific definitions used by
** ABCC modules as well as applications designed to use such modules.
********************************************************************************
********************************************************************************
** Services:
********************************************************************************
********************************************************************************
*/

#ifndef ABP_ASM_H_
#define ABP_ASM_H_

/*******************************************************************************
** Constants
********************************************************************************
*/

/*------------------------------------------------------------------------------
** The Assembly Mapping object specific object attributes.
**
** ABP_ASM_OA_WRITE_PD_INST_LIST - Array of Assembly instances that can be
**                                 mapped to write process data.
** ABP_ASM_OA_READ_PD_INST_LIST  - Array of Assembly instances that can be
**                                 mapped to read process data
**------------------------------------------------------------------------------
*/
#define ABP_ASM_OA_WRITE_PD_INST_LIST    11
#define ABP_ASM_OA_READ_PD_INST_LIST     12

/*------------------------------------------------------------------------------
** The data size of the Assembly Mapping object specific attributes
** (in bytes).
**------------------------------------------------------------------------------
*/
#define ABP_ASM_OA_WRITE_PD_INST_LIST_DS             ( 127 * ABP_UINT16_SIZEOF )
#define ABP_ASM_OA_READ_PD_INST_LIST_DS              ( 127 * ABP_UINT16_SIZEOF )


/*------------------------------------------------------------------------------
** The Assembly Mapping Object specific instance attributes.
**------------------------------------------------------------------------------
*/
#define ABP_ASM_IA_DESCRIPTOR               1    /* Descriptor                */
#define ABP_ASM_IA_ADI_MAP_XX               2    /* Attributes 2-12 are valid */

/*------------------------------------------------------------------------------
** The data size of the Assembly Mapping object specific instance attributes
** (in bytes).
**------------------------------------------------------------------------------
*/
#define ABP_ASM_IA_DESCRIPTOR_DS                   ABP_UINT32_SIZEOF

/*------------------------------------------------------------------------------
** The Assembly Mapping object specific message commands.
**------------------------------------------------------------------------------
*/
#define ABP_ASM_CMD_WRITE_ASSEMBLY_DATA            0x10
#define ABP_ASM_CMD_READ_ASSEMBLY_DATA             0x11

/*******************************************************************************
** Typedefs
********************************************************************************
*/

/*******************************************************************************
** Public Globals
********************************************************************************
*/

/*******************************************************************************
** Public Services
********************************************************************************
*/

#endif  /* inclusion lock */

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp_fsi.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC File System Interface Object Protocol Definitions.
**
** This software component contains FSI definitions used by Anybus-CC
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

#ifndef ABP_FSI_H
#define ABP_FSI_H


/*******************************************************************************
**
** File System Interface object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The File System Interface object specific object attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_OA_MAX_INST                   11 /* Max number of instances    */
#define ABP_FSI_OA_DISABLE_VFS                12 /* Disables VFS               */
#define ABP_FSI_OA_TOTAL_DISC_SIZE            13 /* Total size for discs       */
#define ABP_FSI_OA_FREE_DISC_SIZE             14 /* Free size for discs        */
#define ABP_FSI_OA_DISC_TYPE                  16 /* Disc type identifier       */
#define ABP_FSI_OA_DISC_FAULT_TOLERANCE_LEVEL 17 /* Disc fault tolerance level */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC File System Interface object specific
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_OA_MAX_INST_DS                     ABP_UINT16_SIZEOF
#define ABP_FSI_OA_DISABLE_VFS_DS                  ABP_BOOL_SIZEOF
#define ABP_FSI_OA_TOTAL_DISC_SIZE_DS              ABP_UINT32_SIZEOF
#define ABP_FSI_OA_FREE_DISC_SIZE_DS               ABP_UINT32_SIZEOF
#define ABP_FSI_OA_DISC_TYPE_DS                    ABP_UINT8_SIZEOF
#define ABP_FSI_OA_DISC_FAULT_TOLERANCE_LEVEL_DS   ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** The File System Interface instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_IA_TYPE                    1    /* Instance type              */
#define ABP_FSI_IA_FILE_SIZE               2    /* File size                  */
#define ABP_FSI_IA_PATH                    3    /* Current instance path      */


/*------------------------------------------------------------------------------
**
** The data size of the Anybus-CC File System Interface object instance
** attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_IA_TYPE_DS                         ABP_UINT8_SIZEOF
#define ABP_FSI_IA_FILE_SIZE_DS                    ABP_UINT32_SIZEOF


/*------------------------------------------------------------------------------
**
** The File System Interface object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_CMD_FILE_OPEN                      0x10
#define ABP_FSI_CMD_FILE_CLOSE                     0x11
#define ABP_FSI_CMD_FILE_DELETE                    0x12
#define ABP_FSI_CMD_FILE_COPY                      0x13
#define ABP_FSI_CMD_FILE_RENAME                    0x14
#define ABP_FSI_CMD_FILE_READ                      0x15
#define ABP_FSI_CMD_FILE_WRITE                     0x16
#define ABP_FSI_CMD_DIRECTORY_OPEN                 0x20
#define ABP_FSI_CMD_DIRECTORY_CLOSE                0x21
#define ABP_FSI_CMD_DIRECTORY_DELETE               0x22
#define ABP_FSI_CMD_DIRECTORY_READ                 0x23
#define ABP_FSI_CMD_DIRECTORY_CREATE               0x24
#define ABP_FSI_CMD_DIRECTORY_CHANGE               0x25
#define ABP_FSI_CMD_FORMAT_DISC                    0x30


/*------------------------------------------------------------------------------
**
** Max number of characters in the path.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_MAX_PATH_LENGTH                 126


/*------------------------------------------------------------------------------
**
** Disc type attribute values.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_DISC_TYPE_UNKNOWN               0
#define ABP_FSI_DISC_TYPE_1                     1
#define ABP_FSI_DISC_TYPE_2                     2


/*------------------------------------------------------------------------------
**
** Disc fault tolerance attribute values.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_DISC_TOLERANCE_LEVEL_NONE          0  /* No failure tolerance   */
#define ABP_FSI_DISC_TOLERANCE_LEVEL_FS_STRUCTURE  1  /* Protect File structure */


/*------------------------------------------------------------------------------
**
** Instance types.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_TYPE_NONE                 0  /* Instance holds nothing        */
#define ABP_FSI_TYPE_FILE                 1  /* Instance holds a file         */
#define ABP_FSI_TYPE_DIRECTORY            2  /* Instance holds a directory    */


/*------------------------------------------------------------------------------
**
** File open command modes.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_FILE_OPEN_READ_MODE       0x00
#define ABP_FSI_FILE_OPEN_WRITE_MODE      0x01
#define ABP_FSI_FILE_OPEN_APPEND_MODE     0x02


/*------------------------------------------------------------------------------
**
** Directory read object flags.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_DIRECTORY_READ_DIRECTORY     0x01
#define ABP_FSI_DIRECTORY_READ_READ_ONLY     0x02
#define ABP_FSI_DIRECTORY_READ_HIDDEN        0x04
#define ABP_FSI_DIRECTORY_READ_SYSTEM        0x08


/*------------------------------------------------------------------------------
**
** File System Interface object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_FSI_ERR_FILE_OPEN_FAILED                  1
#define ABP_FSI_ERR_FILE_CLOSE_FAILED                 2
#define ABP_FSI_ERR_FILE_DELETE_FAILED                3
#define ABP_FSI_ERR_DIRECTORY_OPEN_FAILED             4
#define ABP_FSI_ERR_DIRECTORY_CLOSE_FAILED            5
#define ABP_FSI_ERR_DIRECTORY_CREATE_FAILED           6
#define ABP_FSI_ERR_DIRECTORY_DELETE_FAILED           7
#define ABP_FSI_ERR_DIRECTORY_CHANGE_FAILED           8
#define ABP_FSI_ERR_FILE_COPY_OPEN_READ_FAILED        9
#define ABP_FSI_ERR_FILE_COPY_OPEN_WRITE_FAILED      10
#define ABP_FSI_ERR_FILE_COPY_WRITE_FAILED           11
#define ABP_FSI_ERR_FILE_RENAME_FAILED               12


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp_fsi.h
**
********************************************************************************
*/

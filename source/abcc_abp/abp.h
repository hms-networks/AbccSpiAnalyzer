/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** abp.h
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** ABP - Anybus-CC Protocol Definitions.
**
** This software component contains protocol definitions used by Anybus-CC
** modules as well as applications designed to use such modules.
**
** This file contains the generic portion used by all Anybus-CC modules.
**
** Network specific definitions that may be required by some applications are
** published in the corresponding abp_xxx.h file(s).
**
********************************************************************************
********************************************************************************
**
** Services List
** -------------
**
** Public Services:
**
**    ABP_SetMsgErrorResponse()  - Convert message command to an error response.
**    ABP_SetMsgResponse()       - Convert message command to a response.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2009 HMS Industrial Networks AB                 **
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

#ifndef ABP_H_
#define ABP_H_



/*******************************************************************************
**
** ABCC operating mode constants
**
********************************************************************************
*/

#define ABP_OP_MODE_SPI                1
#define ABP_OP_MODE_SHIFT_REGISTER     2
#define ABP_OP_MODE_16_BIT_PARALLEL    7
#define ABP_OP_MODE_8_BIT_PARALLEL     8
#define ABP_OP_MODE_SERIAL_19_2        9
#define ABP_OP_MODE_SERIAL_57_6        10
#define ABP_OP_MODE_SERIAL_115_2       11
#define ABP_OP_MODE_SERIAL_625         12


/*******************************************************************************
**
** Common telegram and message constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Control register bitmasks.
**
**------------------------------------------------------------------------------
*/

#define ABP_CTRL_T_BIT              0x80
#define ABP_CTRL_M_BIT              0x40
#define ABP_CTRL_R_BIT              0x20
#define ABP_CTRL_A_BIT              0x10


/*------------------------------------------------------------------------------
**
** Status register bitmasks.
**
**------------------------------------------------------------------------------
*/

#define ABP_STAT_T_BIT              0x80
#define ABP_STAT_M_BIT              0x40
#define ABP_STAT_R_BIT              0x20
#define ABP_STAT_A_BIT              0x10
#define ABP_STAT_SUP_BIT            0x08
#define ABP_STAT_S_BITS             0x07


/*------------------------------------------------------------------------------
**
** The maximum amount of process data in a telegram (in bytes).
** Note: Used for ping-pong protocol on both NP30 and NP40.
**------------------------------------------------------------------------------
*/

#define ABP_MAX_PROCESS_DATA        256


/*------------------------------------------------------------------------------
**
** The maximum amount of data in a message data field (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_MAX_MSG_255_DATA_BYTES   255
#define ABP_MAX_MSG_DATA_BYTES      1524


/*------------------------------------------------------------------------------
**
** Message header bitmasks.
**
**------------------------------------------------------------------------------
*/

#define ABP_MSG_HEADER_C_BIT        0x40
#define ABP_MSG_HEADER_E_BIT        0x80
#define ABP_MSG_HEADER_CMD_BITS     0x3F


/*------------------------------------------------------------------------------
**
** Message commands.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_MsgCmdType
{
   ABP_CMD_GET_ATTR              = 0x01,  /* Get attribute                    */
   ABP_CMD_SET_ATTR              = 0x02,  /* Set attribute                    */
   ABP_CMD_CREATE                = 0x03,  /* Create                           */
   ABP_CMD_DELETE                = 0x04,  /* Delete                           */
   ABP_CMD_RESET                 = 0x05,  /* Reset                            */
   ABP_CMD_GET_ENUM_STR          = 0x06,  /* Get enumeration string           */
   ABP_CMD_GET_INDEXED_ATTR      = 0x07,  /* Get indexed attribute            */
   ABP_CMD_SET_INDEXED_ATTR      = 0x08   /* Set indexed attribute            */
}
ABP_MsgCmdType;


/*------------------------------------------------------------------------------
**
** Reset command types.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_ResetType
{
   ABP_RESET_POWER_ON                  = 0x00,  /* Power-on reset             */
   ABP_RESET_FACTORY_DEFAULT           = 0x01,  /* Factory default reset      */
   ABP_RESET_POWER_ON_FACTORY_DEFAULT  = 0x02   /* Power-on + Factory reset   */
}
ABP_ResetType;


/*------------------------------------------------------------------------------
**
** Message error codes.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_MsgErrorCodeType
{
   ABP_ERR_NO_ERROR            = 0x00,    /* No error                         */

   ABP_ERR_INV_MSG_FORMAT      = 0x02,    /* Invalid message format           */
   ABP_ERR_UNSUP_OBJ           = 0x03,    /* Unsupported object               */
   ABP_ERR_UNSUP_INST          = 0x04,    /* Unsupported instance             */
   ABP_ERR_UNSUP_CMD           = 0x05,    /* Unsupported command              */
   ABP_ERR_INV_CMD_EXT_0       = 0x06,    /* Invalid CmdExt[ 0 ]              */
   ABP_ERR_INV_CMD_EXT_1       = 0x07,    /* Invalid CmdExt[ 1 ]              */
   ABP_ERR_ATTR_NOT_SETABLE    = 0x08,    /* Attribute access is not set-able */
   ABP_ERR_ATTR_NOT_GETABLE    = 0x09,    /* Attribute access is not get-able */
   ABP_ERR_TOO_MUCH_DATA       = 0x0A,    /* Too much data in msg data field  */
   ABP_ERR_NOT_ENOUGH_DATA     = 0x0B,    /* Not enough data in msg data field*/
   ABP_ERR_OUT_OF_RANGE        = 0x0C,    /* Out of range                     */
   ABP_ERR_INV_STATE           = 0x0D,    /* Invalid state                    */
   ABP_ERR_NO_RESOURCES        = 0x0E,    /* Out of resources                 */
   ABP_ERR_SEG_FAILURE         = 0x0F,    /* Segmentation failure             */
   ABP_ERR_SEG_BUF_OVERFLOW    = 0x10,    /* Segmentation buffer overflow     */
   ABP_ERR_VAL_TOO_HIGH        = 0x11,    /* Written data value is too high (ABCC40) */
   ABP_ERR_VAL_TOO_LOW         = 0x12,    /* Written data value is too low  (ABCC40) */
   ABP_ERR_CONTROLLED_FROM_OTHER_CHANNEL = 0x13, /* NAK writes to "read process data" mapped attr. (ABCC40) */
   ABP_ERR_MSG_CHANNEL_TOO_SMALL = 0x14,  /* Response does not fit (ABCC40)   */
   ABP_ERR_GENERAL_ERROR       = 0x15,    /* General error (ABCC40)           */
   ABP_ERR_PROTECTED_ACCESS    = 0x16,    /* Protected access (ABCC40)        */
   ABP_ERR_DATA_NOT_AVAILABLE  = 0x17,    /* Data not available (ABCC40)      */
   ABP_ERR_OBJ_SPECIFIC        = 0xFF     /* Object specific error            */
}
ABP_MsgErrorCodeType;


/*------------------------------------------------------------------------------
**
** Application status data type.
**
**------------------------------------------------------------------------------
*/
typedef enum ABP_AppStatusType
{
   ABP_APPSTAT_NO_ERROR           = 0x0000,
   ABP_APPSTAT_NOT_SYNCED         = 0x0001,
   ABP_APPSTAT_SYNC_CFG_ERR       = 0x0002,
   ABP_APPSTAT_READ_PD_CFG_ERR    = 0x0003,
   ABP_APPSTAT_WRITE_PD_CFG_ERR   = 0x0004,
   ABP_APPSTAT_SYNC_LOSS          = 0x0005,
   ABP_APPSTAT_PD_DATA_LOSS       = 0x0006,
   ABP_APPSTAT_OUTPUT_ERR         = 0x0007
}
ABP_AppStatusType;


/*******************************************************************************
**
** Anybus interface constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Address memory offsets.
**
**------------------------------------------------------------------------------
*/
#define ABP_WRPD_ADR_OFFSET            0x0000
#define ABP_RDPD_ADR_OFFSET            0x1000
#define ABP_WRMSG_ADR_OFFSET           0x2000
#define ABP_RDMSG_ADR_OFFSET           0x3000
#define ABP_WRPDM_ADR_OFFSET           0x3800
#define ABP_RDPDM_ADR_OFFSET           0x3900
#define ABP_WRMSG_LEGACY_ADR_OFFSET    0x3B00
#define ABP_RDMSG_LEGACY_ADR_OFFSET    0x3D00

#define ABP_MODCAP_ADR_OFFSET          0x3FF0
#define ABP_LEDSTATUS_ADR_OFFSET       0x3FF2
#define ABP_APPSTATUS_ADR_OFFSET       0x3FF4
#define ABP_ANBSTATUS_ADR_OFFSET       0x3FF6
#define ABP_BUFCTRL_ADR_OFFSET         0x3FF8
#define ABP_INTMASK_ADR_OFFSET         0x3FFA
#define ABP_INTSTATUS_ADR_OFFSET       0x3FFC
#define ABP_CONTROL_ADR_OFFSET         0x3FFE
#define ABP_STATUS_ADR_OFFSET          0x3FFF


/*------------------------------------------------------------------------------
**
** BUFCTRL Register.
**
**------------------------------------------------------------------------------
*/
#define ABP_BUFCTRL_WRPD               0x01
#define ABP_BUFCTRL_RDPD               0x02
#define ABP_BUFCTRL_WRMSG              0x04
#define ABP_BUFCTRL_RDMSG              0x08
#define ABP_BUFCTRL_ANBR               0x10
#define ABP_BUFCTRL_APPR               0x20
#define ABP_BUFCTRL_APPRCLR            0x40


/*------------------------------------------------------------------------------
**
** INT STATUS Register.
**
**------------------------------------------------------------------------------
*/
#define ABP_INTSTATUS_RDPDI            0x01
#define ABP_INTSTATUS_RDMSGI           0x02
#define ABP_INTSTATUS_WRMSGI           0x04
#define ABP_INTSTATUS_ANBRI            0x08
#define ABP_INTSTATUS_STATUSI          0x10
#define ABP_INTSTATUS_PWRI             0x20
#define ABP_INTSTATUS_SYNCI            0x40


/*------------------------------------------------------------------------------
**
** INT MASK Register.
**
**------------------------------------------------------------------------------
*/
#define ABP_INTMASK_RDPDIEN            0x01
#define ABP_INTMASK_RDMSGIEN           0x02
#define ABP_INTMASK_WRMSGIEN           0x04
#define ABP_INTMASK_ANBRIEN            0x08
#define ABP_INTMASK_STATUSIEN          0x10
#define ABP_INTMASK_SYNCIEN            0x40


/*------------------------------------------------------------------------------
**
** SPI control word in MOSI frame
**
**------------------------------------------------------------------------------
*/
#define ABP_SPI_CTRL_WRPD_VALID        0x01
#define ABP_SPI_CTRL_CMDCNT            0x06
#define ABP_SPI_CTRL_M                 0x08
#define ABP_SPI_CTRL_LAST_FRAG         0x10
#define ABP_SPI_CTRL_T                 0x80



/*------------------------------------------------------------------------------
**
** SPI status word in MISO frame
**
**------------------------------------------------------------------------------
*/
#define ABP_SPI_STATUS_WRMSG_FULL      0x01
#define ABP_SPI_STATUS_CMDCNT          0x06
#define ABP_SPI_STATUS_M               0x08
#define ABP_SPI_STATUS_LAST_FRAG       0x10
#define ABP_SPI_STATUS_NEW_PD          0x20


/*******************************************************************************
**
** ABCC module type constants
**
********************************************************************************
*/
#define ABP_MODULE_TYPE_ABCC                 0x0401   /* ABCC30 */
#define ABP_MODULE_TYPE_ABCC_DRIVE_PROFILE   0x0402
#define ABP_MODULE_TYPE_ABCC_40              0x0403


/*******************************************************************************
**
** ABCC module id constants
** MI1   MI0
** 0     0  ( 0)  Active CompactCom 30-series
** 0     1  ( 1 ) Passive CompactCom
** 1     0  ( 2 ) Active CompactCom 40-series
** 1     1  ( 3 ) Customer specific
********************************************************************************
*/
#define ABP_MODULE_ID_ACTIVE_ABCC30       0
#define ABP_MODULE_ID_PASSIVE_ABCC        1
#define ABP_MODULE_ID_ACTIVE_ABCC40       2
#define ABP_MODULE_ID_CUSTOMER_SPECIFIC   3

/*******************************************************************************
**
** ABCC network types
**
********************************************************************************
*/

#define ABP_NW_TYPE_PDPV0                 0x0001  /* PROFIBUS DP-V0 */
#define ABP_NW_TYPE_PDPV1                 0x0005  /* PROFIBUS DP-V1 */
#define ABP_NW_TYPE_COP                   0x0020  /* CANopen */
#define ABP_NW_TYPE_DEV                   0x0025  /* DeviceNet */
#define ABP_NW_TYPE_RTU                   0x0045  /* Modbus-RTU */
#define ABP_NW_TYPE_CNT                   0x0065  /* ControlNet */
#define ABP_NW_TYPE_ETN_1P                0x0080  /* Modbus-TCP */
#define ABP_NW_TYPE_PRT                   0x0084  /* PROFINET RT */
#define ABP_NW_TYPE_EIP_1P                0x0085  /* EtherNet/IP */
#define ABP_NW_TYPE_ECT                   0x0087  /* EtherCAT */
#define ABP_NW_TYPE_PIR                   0x0089  /* PROFINET IRT */
#define ABP_NW_TYPE_CCL                   0x0090  /* CC-Link */
#define ABP_NW_TYPE_ETN_2P                0x0093  /* Modbus-TCP 2-Port */
#define ABP_NW_TYPE_CPN                   0x0095  /* CompoNet */
#define ABP_NW_TYPE_PRT_2P                0x0096  /* PROFINET RT 2-port */
#define ABP_NW_TYPE_SRC3                  0x0098  /* SERCOS III */
#define ABP_NW_TYPE_BMP                   0x0099  /* BACnet MS/TP */
#define ABP_NW_TYPE_BIP                   0x009A  /* BACnet/IP */
#define ABP_NW_TYPE_EIP_2P_BB             0x009B  /* EtherNet/IP 2-Port BB DLR */
#define ABP_NW_TYPE_EIP_2P                0x009C  /* EtherNet/IP 2-Port */
#define ABP_NW_TYPE_PIR_FO                0x009D  /* PROFINET IRT FO */
#define ABP_NW_TYPE_EPL                   0x009F  /* POWERLINK */
#define ABP_NW_TYPE_CFN                   0x009E  /* CC-Link IE Field Network */
#define ABP_NW_TYPE_CET                   0x00A3  /* Common Ethernet */
#define ABP_NW_TYPE_EIP_2P_BB_IIOT        0x00AB  /* EtherNet/IP IIoT */
#define ABP_NW_TYPE_PIR_IIOT              0x00AD  /* PROFINET IRT IIoT */
#define ABP_NW_TYPE_PIR_FO_IIOT           0x00AE  /* PROFINET IRT FO IIoT */
#define ABP_NW_TYPE_CET_IIOT              0x00AF  /* Common Ethernet IIoT */




/*******************************************************************************
**
** Anybus-CC data types.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus-CC data type numbers.
**
**------------------------------------------------------------------------------
*/

#define ABP_BOOL                    0        /* Boolean                       */
#define ABP_SINT8                   1        /* Signed 8 bit integer          */
#define ABP_SINT16                  2        /* Signed 16 bit integer         */
#define ABP_SINT32                  3        /* Signed 32 bit integer         */
#define ABP_UINT8                   4        /* Unsigned 8 bit integer        */
#define ABP_UINT16                  5        /* Unsigned 16 bit integer       */
#define ABP_UINT32                  6        /* Unsigned 32 bit integer       */
#define ABP_CHAR                    7        /* Character                     */
#define ABP_ENUM                    8        /* Enumeration                   */
#define ABP_BITS8                   9        /* 8 bit bitfield (ABCC40)       */
#define ABP_BITS16                  10       /* 16 bit bitfield (ABCC40)      */
#define ABP_BITS32                  11       /* 32 bit bitfield (ABCC40)      */
#define ABP_OCTET                   12       /* 8 bit data (ABCC40)           */

#define ABP_SINT64                  16       /* Signed 64 bit integer         */
#define ABP_UINT64                  17       /* Unsigned 64 bit integer       */
#define ABP_FLOAT                   18       /* Floating point/real number    */

#define ABP_PAD0                    32       /* Padding bitfield (ABCC40)     */
#define ABP_PAD1                    33       /* Padding bitfield (ABCC40)     */
#define ABP_PAD2                    34       /* Padding bitfield (ABCC40)     */
#define ABP_PAD3                    35       /* Padding bitfield (ABCC40)     */
#define ABP_PAD4                    36       /* Padding bitfield (ABCC40)     */
#define ABP_PAD5                    37       /* Padding bitfield (ABCC40)     */
#define ABP_PAD6                    38       /* Padding bitfield (ABCC40)     */
#define ABP_PAD7                    39       /* Padding bitfield (ABCC40)     */
#define ABP_PAD8                    40       /* Padding bitfield (ABCC40)     */
#define ABP_PAD9                    41       /* Padding bitfield (ABCC40)     */
#define ABP_PAD10                   42       /* Padding bitfield (ABCC40)     */
#define ABP_PAD11                   43       /* Padding bitfield (ABCC40)     */
#define ABP_PAD12                   44       /* Padding bitfield (ABCC40)     */
#define ABP_PAD13                   45       /* Padding bitfield (ABCC40)     */
#define ABP_PAD14                   46       /* Padding bitfield (ABCC40)     */
#define ABP_PAD15                   47       /* Padding bitfield (ABCC40)     */
#define ABP_PAD16                   48       /* Padding bitfield (ABCC40)     */

#define ABP_BOOL1                   64       /* 1 bit boolean (ABCC40)        */
#define ABP_BIT1                    65       /* 1 bit bitfield (ABCC40)       */
#define ABP_BIT2                    66       /* 2 bit bitfield (ABCC40)       */
#define ABP_BIT3                    67       /* 3 bit bitfield (ABCC40)       */
#define ABP_BIT4                    68       /* 4 bit bitfield (ABCC40)       */
#define ABP_BIT5                    69       /* 5 bit bitfield (ABCC40)       */
#define ABP_BIT6                    70       /* 6 bit bitfield (ABCC40)       */
#define ABP_BIT7                    71       /* 7 bit bitfield (ABCC40)       */


/*------------------------------------------------------------------------------
**
** The size of the Anybus-CC data types (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_BOOL_SIZEOF             1        /* Boolean                       */
#define ABP_SINT8_SIZEOF            1        /* Signed 8 bit integer          */
#define ABP_SINT16_SIZEOF           2        /* Signed 16 bit integer         */
#define ABP_SINT32_SIZEOF           4        /* Signed 32 bit integer         */
#define ABP_UINT8_SIZEOF            1        /* Unsigned 8 bit integer        */
#define ABP_UINT16_SIZEOF           2        /* Unsigned 16 bit integer       */
#define ABP_UINT32_SIZEOF           4        /* Unsigned 32 bit integer       */
#define ABP_CHAR_SIZEOF             1        /* Character                     */
#define ABP_ENUM_SIZEOF             1        /* Enumeration                   */
#define ABP_BITS8_SIZEOF            1        /* 8 bit bitfield (ABCC40)       */
#define ABP_BITS16_SIZEOF           2        /* 16 bit bitfield (ABCC40)      */
#define ABP_BITS32_SIZEOF           4        /* 32 bit bitfield (ABCC40)      */
#define ABP_OCTET_SIZEOF            1        /* 8 bit data (ABCC40)           */

#define ABP_SINT64_SIZEOF           8        /* Signed 64 bit integer         */
#define ABP_UINT64_SIZEOF           8        /* Unsigned 64 bit integer       */
#define ABP_FLOAT_SIZEOF            4        /* Floating point/real number    */


/*------------------------------------------------------------------------------
**
** The Anybus-CC data type maximum values.
**
**------------------------------------------------------------------------------
*/

#define ABP_BOOL_MAX                !0
#define ABP_SINT8_MAX               0x7F
#define ABP_SINT16_MAX              0x7FFF
#define ABP_SINT32_MAX              0x7FFFFFFFL
#define ABP_UINT8_MAX               0xFFU
#define ABP_UINT16_MAX              0xFFFFU
#define ABP_UINT32_MAX              0xFFFFFFFFLU
#define ABP_CHAR_MAX                0xFFU
#define ABP_ENUM_MAX                0xFFU
#define ABP_BITS8_MAX               0xFFU          /* ABCC40 */
#define ABP_BITS16_MAX              0xFFFFU        /* ABCC40 */
#define ABP_BITS32_MAX              0xFFFFFFFFLU   /* ABCC40 */
#define ABP_OCTET_MAX               0xFFU          /* ABCC40 */

#define ABP_SINT64_MAX              0x7FFFFFFFFFFFFFFFL
#define ABP_UINT64_MAX              0xFFFFFFFFFFFFFFFFLU
#define ABP_FLOAT_MAX               3.402823466E+38F

#define ABP_BOOL1_MAX               0x1            /* ABCC40 */
#define ABP_BITS1_MAX               0x1            /* ABCC40 */
#define ABP_BITS2_MAX               0x3            /* ABCC40 */
#define ABP_BITS3_MAX               0x7            /* ABCC40 */
#define ABP_BITS4_MAX               0xF            /* ABCC40 */
#define ABP_BITS5_MAX               0x1F           /* ABCC40 */
#define ABP_BITS6_MAX               0x3F           /* ABCC40 */
#define ABP_BITS7_MAX               0x7F           /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The Anybus-CC data type minimum values.
**
**------------------------------------------------------------------------------
*/

#define ABP_BOOL_MIN                0
#define ABP_SINT8_MIN               ( - ABP_SINT8_MAX - 1 )
#define ABP_SINT16_MIN              ( - ABP_SINT16_MAX - 1 )
#define ABP_SINT32_MIN              ( - ABP_SINT32_MAX - 1L )
#define ABP_UINT8_MIN               0
#define ABP_UINT16_MIN              0
#define ABP_UINT32_MIN              0
#define ABP_CHAR_MIN                0
#define ABP_ENUM_MIN                0
#define ABP_BITS8_MIN               0 /* ABCC40 */
#define ABP_BITS16_MIN              0 /* ABCC40 */
#define ABP_BITS32_MIN              0 /* ABCC40 */
#define ABP_OCTET_MIN               0 /* ABCC40 */

#define ABP_SINT64_MIN              ( - ABP_SINT64_MAX - 1 )
#define ABP_UINT64_MIN              0
#define ABP_FLOAT_MIN               1.17549435E-38F

#define ABP_BOOL1_MIN               0 /* ABCC40 */
#define ABP_BITS1_MIN               0 /* ABCC40 */
#define ABP_BITS2_MIN               0 /* ABCC40 */
#define ABP_BITS3_MIN               0 /* ABCC40 */
#define ABP_BITS4_MIN               0 /* ABCC40 */
#define ABP_BITS5_MIN               0 /* ABCC40 */
#define ABP_BITS6_MIN               0 /* ABCC40 */
#define ABP_BITS7_MIN               0 /* ABCC40 */


/*******************************************************************************
**
** The languages supported by multilingual objects.
**
********************************************************************************
*/

typedef enum ABP_LangType
{
   ABP_LANG_ENG,                          /* English                          */
   ABP_LANG_DEU,                          /* German                           */
   ABP_LANG_SPA,                          /* Spanish                          */
   ABP_LANG_ITA,                          /* Italian                          */
   ABP_LANG_FRA,                          /* French                           */

   ABP_LANG_NUM_LANG                      /* Number of supported languages    */
}
ABP_LangType;


/*******************************************************************************
**
** Anybus-CC protocol object number pool.
**
** Each object, whether it’s a network specific or a common object, an Anybus
** module or a host application object, must have a unique object number. This
** number list is therefore common to both the application and the Anybus.
**
** Anybus objects are numbered from 1 and up while application objects
** are numbered from 255 and down.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Anybus module objects
**------------------------------------------------------------------------------
*/

#define ABP_OBJ_NUM_ANB               1   /* Anybus                           */
#define ABP_OBJ_NUM_DI                2   /* Diagnostic                       */
#define ABP_OBJ_NUM_NW                3   /* Network                          */
#define ABP_OBJ_NUM_NC                4   /* Network Configuration            */
#define ABP_OBJ_NUM_ADD               5   /* PROFIBUS DP-V1 Additional Diag   */
#define ABP_OBJ_NUM_RSV1              6   /* Reserved                         */
#define ABP_OBJ_NUM_SOC               7   /* Socket Interface                 */
#define ABP_OBJ_NUM_NWCCL             8   /* Network CC-Link                  */
#define ABP_OBJ_NUM_SMTP              9   /* SMTP Client                      */
#define ABP_OBJ_NUM_FSI              10   /* File System Interface            */
#define ABP_OBJ_NUM_NWDPV1           11   /* Network PROFIBUS DP-V1           */
#define ABP_OBJ_NUM_NWETN            12   /* Network Ethernet                 */
#define ABP_OBJ_NUM_CPC              13   /* CIP Port Configuration           */
#define ABP_OBJ_NUM_NWPNIO           14   /* Network PROFINET IO              */
#define ABP_OBJ_NUM_PNIOADD          15   /* PROFINET IO Additional Diag      */
#define ABP_OBJ_NUM_DPV0DI           16   /* PROFIBUS DP-V0 Diagnostic        */
#define ABP_OBJ_NUM_FUSM             17   /* Functional Safety Module         */
#define ABP_OBJ_NUM_NWCFN            18   /* Network CC-Link IE Field Network */


/*------------------------------------------------------------------------------
** Host application objects
**------------------------------------------------------------------------------
*/

#define ABP_OBJ_NUM_MQTT            226   /* MQ Telemetry Transport           */
#define ABP_OBJ_NUM_OPCUA           227   /* OPC Unified Architecture         */
#define ABP_OBJ_NUM_EME             228   /* Energy Measurement               */
#define ABP_OBJ_NUM_PNAM            229   /* PROFINET Asset Management        */
#define ABP_OBJ_NUM_CFN             230   /* CC-Link IE Field Network         */
#define ABP_OBJ_NUM_ER              231   /* Energy Reporting                 */
#define ABP_OBJ_NUM_SAFE            232   /* Functional Safety                */
#define ABP_OBJ_NUM_EPL             233   /* POWERLINK                        */
#define ABP_OBJ_NUM_AFSI            234   /* Application File System Interface*/
#define ABP_OBJ_NUM_ASM             235   /* Assembly mapping object          */
#define ABP_OBJ_NUM_MDD             236   /* Modular device                   */
#define ABP_OBJ_NUM_CIPID           237   /* CIP Identity                     */
#define ABP_OBJ_NUM_SYNC            238   /* Sync                             */
#define ABP_OBJ_NUM_BAC             239   /* BACnet                           */
#define ABP_OBJ_NUM_ECO             240   /* Energy Control                   */
#define ABP_OBJ_NUM_SRC3            241   /* SERCOS III                       */
#define ABP_OBJ_NUM_PRD             242   /* PROFIdrive                       */
#define ABP_OBJ_NUM_CNT             243   /* ControlNet                       */
#define ABP_OBJ_NUM_CPN             244   /* CompoNet                         */
#define ABP_OBJ_NUM_ECT             245   /* EtherCAT                         */
#define ABP_OBJ_NUM_PNIO            246   /* PROFINET IO                      */
#define ABP_OBJ_NUM_CCL             247   /* CC-Link                          */
#define ABP_OBJ_NUM_EIP             248   /* EtherNet/IP                      */
#define ABP_OBJ_NUM_ETN             249   /* Ethernet                         */
#define ABP_OBJ_NUM_MOD             250   /* Modbus                           */
#define ABP_OBJ_NUM_COP             251   /* CANopen                          */
#define ABP_OBJ_NUM_DEV             252   /* DeviceNet                        */
#define ABP_OBJ_NUM_DPV1            253   /* PROFIBUS DP-V1                   */
#define ABP_OBJ_NUM_APPD            254   /* Application Data                 */
#define ABP_OBJ_NUM_APP             255   /* Application                      */


/*******************************************************************************
**
** Common object constants.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The object standard attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_OA_NAME                 1     /* Object name                      */
#define ABP_OA_REV                  2     /* Object revision                  */
#define ABP_OA_NUM_INST             3     /* Current number of instances      */
#define ABP_OA_HIGHEST_INST         4     /* Current highest instance number  */


/*------------------------------------------------------------------------------
**
** The data size of the object standard attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_OA_REV_DS               ABP_UINT8_SIZEOF
#define ABP_OA_NUM_INST_DS          ABP_UINT16_SIZEOF
#define ABP_OA_HIGHEST_INST_DS      ABP_UINT16_SIZEOF


/*------------------------------------------------------------------------------
**
** The object instance number.
**
**------------------------------------------------------------------------------
*/

#define ABP_INST_OBJ                0


/*******************************************************************************
**
** Anybus object constants.
**
** Object revision: 4.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Anybus instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ANB_IA_MODULE_TYPE      1
#define ABP_ANB_IA_FW_VERSION       2
#define ABP_ANB_IA_SERIAL_NUM       3
#define ABP_ANB_IA_WD_TIMEOUT       4
#define ABP_ANB_IA_SETUP_COMPLETE   5
#define ABP_ANB_IA_EXCEPTION        6
#define ABP_ANB_IA_FATAL_EVENT      7
#define ABP_ANB_IA_ERROR_CNTRS      8
#define ABP_ANB_IA_LANG             9
#define ABP_ANB_IA_PROVIDER_ID      10
#define ABP_ANB_IA_PROVIDER_INFO    11
#define ABP_ANB_IA_LED_COLOURS      12
#define ABP_ANB_IA_LED_STATUS       13
#define ABP_ANB_IA_SWITCH_STATUS    14
#define ABP_ANB_IA_AUX_BIT_FUNC     15
#define ABP_ANB_IA_GPIO_CONFIG      16
#define ABP_ANB_IA_VIRTUAL_ATTRS    17  /* ABCC40 */
#define ABP_ANB_IA_BLACK_WHITE_LIST 18  /* ABCC40 */
#define ABP_ANB_IA_NETWORK_TIME     19  /* ABCC40 */
#define ABP_ANB_IA_FW_CUST_VERSION  20  /* ABCC40 */
#define ABP_ANB_IA_ABIP_LICENSE     21  /* Anybus IP */

/*------------------------------------------------------------------------------
**
** The data size of the Anybus instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_ANB_IA_MODULE_TYPE_DS         ABP_UINT16_SIZEOF
#define ABP_ANB_IA_FW_VERSION_DS          ( 3 * ABP_UINT8_SIZEOF )
#define ABP_ANB_IA_SERIAL_NUM_DS          ABP_UINT32_SIZEOF
#define ABP_ANB_IA_WD_TIMEOUT_DS          ABP_UINT16_SIZEOF
#define ABP_ANB_IA_SETUP_COMPLETE_DS      ABP_BOOL_SIZEOF
#define ABP_ANB_IA_EXCEPTION_DS           ABP_ENUM_SIZEOF
#define ABP_ANB_IA_ERROR_CNTRS_DS         ( 4 * ABP_UINT16_SIZEOF )
#define ABP_ANB_IA_LANG_DS                ABP_ENUM_SIZEOF
#define ABP_ANB_IA_PROVIDER_ID_DS         ABP_UINT16_SIZEOF
#define ABP_ANB_IA_PROVIDER_INFO_DS       ABP_UINT16_SIZEOF
#define ABP_ANB_IA_LED_COLOURS_DS         ( 4 * ABP_UINT8_SIZEOF )
#define ABP_ANB_IA_LED_STATUS_DS          ABP_UINT8_SIZEOF
#define ABP_ANB_IA_SWITCH_STATUS_DS       ( 2 * ABP_UINT8_SIZEOF )
#define ABP_ANB_IA_AUX_BIT_FUNC_DS        ABP_UINT8_SIZEOF
#define ABP_ANB_IA_GPIO_CONFIG_DS         ABP_UINT16_SIZEOF
#define ABP_ANB_IA_VIRTUAL_ATTRS_DS       1524                       /* ABCC40 */
#define ABP_ANB_IA_BLACK_WHITE_LIST_DS    ( 12 * ABP_UINT16_SIZEOF ) /* ABCC40 */
#define ABP_ANB_IA_NETWORK_TIME_DS        ABP_UINT64_SIZEOF          /* ABCC40 */
#define ABP_ANB_IA_FW_CUST_VERSION_DS     ABP_UINT8_SIZEOF           /* ABCC40 */
#define ABP_ANB_IA_ABIP_LICENSE_DS        ABP_UINT8_SIZEOF           /* Anybus IP */

/*------------------------------------------------------------------------------
**
** The Anybus object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ANB_ERR_INV_PRD_CFG       0x01  /* Invalid process data config    */
#define ABP_ANB_ERR_INV_DEV_ADDR      0x02  /* Invalid device address         */
#define ABP_ANB_ERR_INV_COM_SETTINGS  0x03  /* Invalid communication settings */


/*------------------------------------------------------------------------------
**
** Anybus state.
**
** The current Anybus state is presented by status bits S[0..2] of the
** status register, transferred to the application in each telegram.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_AnbStateType
{
   ABP_ANB_STATE_SETUP             =  0x00,
   ABP_ANB_STATE_NW_INIT           =  0x01,
   ABP_ANB_STATE_WAIT_PROCESS      =  0x02,
   ABP_ANB_STATE_IDLE              =  0x03,
   ABP_ANB_STATE_PROCESS_ACTIVE    =  0x04,
   ABP_ANB_STATE_ERROR             =  0x05,
   ABP_ANB_STATE_EXCEPTION         =  0x07
}
ABP_AnbStateType;


/*------------------------------------------------------------------------------
**
** Anybus exception codes.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_AnbExceptionCodeType
{
   ABP_ANB_EXCPT_NONE                = 0x00, /* No exception                   */
   ABP_ANB_EXCPT_APP_TO              = 0x01, /* Application timeout            */
   ABP_ANB_EXCPT_INV_DEV_ADDR        = 0x02, /* Invalid device address         */
   ABP_ANB_EXCPT_INV_COM_SETTINGS    = 0x03, /* Invalid communication settings */
   ABP_ANB_EXCPT_MAJ_UNREC_APP_EVNT  = 0x04, /* Major unrecoverable app event  */
   ABP_ANB_EXCPT_WAIT_APP_RESET      = 0x05, /* Waiting for application reset  */
   ABP_ANB_EXCPT_INV_PRD_CFG         = 0x06, /* Invalid process data config    */
   ABP_ANB_EXCPT_INV_APP_RESPONSE    = 0x07, /* Invalid application response   */
   ABP_ANB_EXCPT_NVS_CHECKSUM_ERROR  = 0x08, /* NVS memory checksum error      */
   ABP_ANB_EXCPT_FUSM_ERROR          = 0x09, /* Functional Safety Module error */
   ABP_ANB_EXCPT_INSUFF_APPL_IMPL    = 0x0A, /* Insufficient application impl. */
   ABP_ANB_EXCPT_MISSING_SERIAL_NUM  = 0x0B, /* Missing serial number          */
   ABP_ANB_EXCPT_CORRUPT_FILE_SYSTEM = 0x0C, /* File system is corrupt         */

   ABP_ANB_EXCPT_NUM_CODES                   /* Number of exception codes      */
}
ABP_AnbExceptionCodeType;


/*------------------------------------------------------------------------------
**
** Anybus IP License.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_AbipLicenseType
{
   ABP_ANB_ABIP_LICENSE_NONE      =  0x00,
   ABP_ANB_ABIP_LICENSE_TIME_BOMB =  0x01,
   ABP_ANB_ABIP_LICENSE_STANDARD  =  0x02,
   ABP_ANB_ABIP_LICENSE_EXTENDED  =  0x03
}
ABP_AbipLicenseType;


/*------------------------------------------------------------------------------
**
** LED colour codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ANB_LED_COLOUR_NONE     0x00
#define ABP_ANB_LED_COLOUR_GREEN    0x01
#define ABP_ANB_LED_COLOUR_RED      0x02
#define ABP_ANB_LED_COLOUR_YELLOW   0x03
#define ABP_ANB_LED_COLOUR_ORANGE   0x04
#define ABP_ANB_LED_COLOUR_BLUE     0x05
#define ABP_ANB_LED_COLOUR_WHITE    0x06


/*------------------------------------------------------------------------------
**
** Auxiliary bit functionality codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ANB_AUX_BIT_FUNC_NONE   0x00     /* Not used                      */
#define ABP_ANB_AUX_BIT_FUNC_CDI    0x01     /* Changed data indication       */


/*------------------------------------------------------------------------------
**
** GPIO configuration codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_ANB_GPIO_CONFIG_STD           0x00  /* Standard GPIO              */
#define ABP_ANB_GPIO_CONFIG_EXT_LED       0x01  /* Extended LED functionality */
#define ABP_ANB_GPIO_CONFIG_RMII          0x02  /* RMII functionality         */
#define ABP_ANB_GPIO_CONFIG_THREE_STATE   0x03  /* Three-state GPIO pins      */


/*******************************************************************************
**
** Diagnostic object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Diagnostic object specific object attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_DI_OA_MAX_INST                11    /* Max number of instances          */
#define ABP_DI_OA_SUPPORT_FUNC            12    /* Supported functionality (ABCC40) */

/*------------------------------------------------------------------------------
**
** The data size of the Diagnostic object specific attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_DI_OA_MAX_INST_DS             ABP_UINT16_SIZEOF
#define ABP_DI_OA_SUPPORT_FUNC_DS         ABP_BITS32_SIZEOF	 /* ABCC40 */

/*------------------------------------------------------------------------------
**
** Supported functionality bit masks
**
**------------------------------------------------------------------------------
*/

#define ABP_DI_OA_SUPPORT_FUNC_LATCH_EVENT_BIT  1

/*------------------------------------------------------------------------------
**
** The Diagnostic instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_DI_IA_SEVERITY                1
#define ABP_DI_IA_EVENT_CODE              2
#define ABP_DI_IA_NW_SPEC_EVENT_INFO      3
#define ABP_DI_IA_SLOT                    4	 /* ABCC40 */
#define ABP_DI_IA_ADI                     5	 /* ABCC40 */
#define ABP_DI_IA_ELEMENT                 6	 /* ABCC40 */
#define ABP_DI_IA_BIT                     7	 /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The data size of the Diagnostic instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_DI_IA_SEVERITY_DS             ABP_UINT8_SIZEOF
#define ABP_DI_IA_EVENT_CODE_DS           ABP_UINT8_SIZEOF
#define ABP_DI_IA_SLOT_DS                 ABP_UINT16_SIZEOF	 /* ABCC40 */
#define ABP_DI_IA_ADI_DS                  ABP_UINT16_SIZEOF	 /* ABCC40 */
#define ABP_DI_IA_ELEMENT_DS              ABP_UINT8_SIZEOF   /* ABCC40 */
#define ABP_DI_IA_BIT_DS                  ABP_UINT8_SIZEOF	 /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The Diagnostic object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_DI_ERR_NOT_REMOVED            0x01  /* Event could not be removed */
#define ABP_DI_LATCH_NOT_SUPPORTED        0x02  /* Latching events not supported */
#define ABP_DI_ERR_NW_SPECIFIC            0xFF  /* Network specific error     */


/*------------------------------------------------------------------------------
**
** Diagnostic object event severity.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_DiEventSeverityType
{
   ABP_DI_EVENT_SEVERITY_MINOR_REC   = 0x00, /* Minor, recoverable            */
   ABP_DI_EVENT_SEVERITY_MINOR_UNREC = 0x10, /* Minor, unrecoverable          */
   ABP_DI_EVENT_SEVERITY_MAJOR_REC   = 0x20, /* Major, recoverable            */
   ABP_DI_EVENT_SEVERITY_MAJOR_UNREC = 0x30, /* Major, unrecoverable          */
   ABP_DI_EVENT_SEVERITY_MINOR_LATCH = 0x50, /* Minor, recoverable latching (ABCC40)  */
   ABP_DI_EVENT_SEVERITY_MAJOR_LATCH = 0x60  /* Major, recoverable latching (ABCC40)  */
}
ABP_DiEventSeverityType;

/*------------------------------------------------------------------------------
**
** Diagnostic object Create CmdExt0 bit masks
**
**------------------------------------------------------------------------------
*/
#define ABP_DI_CREATE_CMDEXT0_SEVERITY_BITS     0x70
#define ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT      0x01

/*------------------------------------------------------------------------------
**
** Diagnostic object event codes.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_DiEventCodeType
{
   ABP_DI_EVENT_NONE                  = 0x00, /* No event                     */
   ABP_DI_EVENT_GENERIC_ERROR         = 0x10, /* Generic Error                */
   ABP_DI_EVENT_CURRENT               = 0x20, /* Current                      */
   ABP_DI_EVENT_CURRENT_DEVICE_INPUT  = 0x21, /* Current, device input side   */
   ABP_DI_EVENT_CURRENT_INSIDE        = 0x22, /* Current, inside the device   */
   ABP_DI_EVENT_CURRENT_DEVICE_OUTPUT = 0x23, /* Current, device output side  */
   ABP_DI_EVENT_VOLTAGE               = 0x30, /* Voltage                      */
   ABP_DI_EVENT_MAINS_VOLTAGE         = 0x31, /* Mains Voltage                */
   ABP_DI_EVENT_VOLTAGE_INSIDE_DEVICE = 0x32, /* Voltage inside the device    */
   ABP_DI_EVENT_OUTPUT_VOLTAGE        = 0x33, /* Output Voltage               */
   ABP_DI_EVENT_TEMPERATURE           = 0x40, /* Temperature                  */
   ABP_DI_EVENT_AMBIENT_TEMPERATURE   = 0x41, /* Ambient Temperature          */
   ABP_DI_EVENT_DEVICE_TEMPERATURE    = 0x42, /* Device Temperature           */
   ABP_DI_EVENT_DEVICE_HARDWARE       = 0x50, /* Device Hardware              */
   ABP_DI_EVENT_DEVICE_SOFTWARE       = 0x60, /* Device Software              */
   ABP_DI_EVENT_INTERNAL_SOFTWARE     = 0x61, /* Internal Software            */
   ABP_DI_EVENT_USER_SOFTWARE         = 0x62, /* User Software                */
   ABP_DI_EVENT_DATA_SET              = 0x63, /* Data Set                     */
   ABP_DI_EVENT_ADDITIONAL_MODULES    = 0x70, /* Additional Modules           */
   ABP_DI_EVENT_MONITORING            = 0x80, /* Monitoring                   */
   ABP_DI_EVENT_COMMUNICATION         = 0x81, /* Communication                */
   ABP_DI_EVENT_PROTOCOL_ERROR        = 0x82, /* Protocol Error               */
   ABP_DI_EVENT_EXTERNAL_ERROR        = 0x90, /* External Error               */
   ABP_DI_EVENT_ADDITIONAL_FUNCTIONS  = 0xF0, /* Additional Functions         */
   ABP_DI_EVENT_NW_SPECIFIC           = 0xFF  /* Network specific             */
}
ABP_DiEventCodeType;


/*******************************************************************************
**
** Network object constants.
**
** Object revision: 2.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Network instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_NW_IA_NW_TYPE                 1
#define ABP_NW_IA_NW_TYPE_STR             2
#define ABP_NW_IA_DATA_FORMAT             3
#define ABP_NW_IA_PARAM_SUPPORT           4
#define ABP_NW_IA_WRITE_PD_SIZE           5
#define ABP_NW_IA_READ_PD_SIZE            6
#define ABP_NW_IA_EXCEPTION_INFO          7


/*------------------------------------------------------------------------------
**
** The data size of the Network instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_NW_IA_NW_TYPE_DS              ABP_UINT16_SIZEOF
#define ABP_NW_IA_DATA_FORMAT_DS          ABP_ENUM_SIZEOF
#define ABP_NW_IA_PARAM_SUPPORT_DS        ABP_BOOL_SIZEOF
#define ABP_NW_IA_WRITE_PD_SIZE_DS        ABP_UINT16_SIZEOF
#define ABP_NW_IA_READ_PD_SIZE_DS         ABP_UINT16_SIZEOF
#define ABP_NW_IA_EXCEPTION_INFO_DS       ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** The Network object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_NW_CMD_MAP_ADI_WRITE_AREA        0x10
#define ABP_NW_CMD_MAP_ADI_READ_AREA         0x11
#define ABP_NW_CMD_MAP_ADI_WRITE_EXT_AREA    0x12
#define ABP_NW_CMD_MAP_ADI_READ_EXT_AREA     0x13


/*------------------------------------------------------------------------------
**
** The Network object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_NW_ERR_INVALID_ADI_DATA_TYPE  0x01  /* Invalid ADI data type      */
#define ABP_NW_ERR_INVALID_NUM_ELEMENTS   0x02  /* Invalid number of elements */
#define ABP_NW_ERR_INVALID_TOTAL_SIZE     0x03  /* Invalid total size         */
#define ABP_NW_ERR_MULTIPLE_MAPPING       0x04  /* Multiple mapping           */
#define ABP_NW_ERR_INVALID_ORDER_NUM      0x05  /* Invalid ADI order number   */
#define ABP_NW_ERR_INVALID_MAP_CMD_SEQ    0x06  /* Invalid mapp cmd sequence  */
#define ABP_NW_ERR_INVALID_MAP_CMD        0x07  /* Command impossible to parse */
#define ABP_NW_ERR_BAD_ALIGNMENT          0x08  /* Invalid data alignment     */
#define ABP_NW_ERR_INVALID_ADI_0          0x09  /* Invalid use of ADI 0       */
#define ABP_NW_ERR_NW_SPEC_RESTRICTION    0xFF  /* Network specific restriction */


/*------------------------------------------------------------------------------
**
** Common Network specific exception information codes.
**
** Please see the network specific header file for the remaining codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_NW_EXCPT_INFO_NONE            0x00  /* No information             */


/*------------------------------------------------------------------------------
**
** The Network data format values.
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_NwDataFormatType
{
   ABP_NW_DATA_FORMAT_LSB_FIRST = 0,                /* 'Little endian'        */
   ABP_NW_DATA_FORMAT_MSB_FIRST = 1,                /* 'Big endian'           */
   ABP_NW_DATA_FORMAT_NUM_FORMATS                   /* Number of data formats */
}
ABP_NwDataFormatType;


/*******************************************************************************
**
** Network configuration object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Network configuration instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_NC_VAR_IA_NAME                1
#define ABP_NC_VAR_IA_DATA_TYPE           2
#define ABP_NC_VAR_IA_NUM_ELEM            3
#define ABP_NC_VAR_IA_DESCRIPTOR          4
#define ABP_NC_VAR_IA_VALUE               5
#define ABP_NC_VAR_IA_CONFIG_VALUE        6	 /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The data size of the Network configuration instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_NC_VAR_IA_DATA_TYPE_DS        ABP_UINT8_SIZEOF
#define ABP_NC_VAR_IA_NUM_ELEM_DS         ABP_UINT8_SIZEOF
#define ABP_NC_VAR_IA_DESCRIPTOR_DS       ABP_UINT8_SIZEOF


/*------------------------------------------------------------------------------
**
** The Network configuration instance attribute descriptor bits.
**
**------------------------------------------------------------------------------
*/

#define ABP_NC_DESCR_GET_ACCESS           0x01
#define ABP_NC_DESCR_SET_ACCESS           0x02
#define ABP_NC_DESCR_SHARED_ACCESS        0x04


/*------------------------------------------------------------------------------
**
** The Network configuration instances common to most Anybus-CC modules.
**
** Note:
**    1. Although all Network configuration instances are network specific,
**       the instances listed here are meant to offer some degree of ANY
**       functionality in that they are always of an eight-bit data type and
**       that they may be set by the application during setup.
**
**    2. In case the values of these instances originate from input devices
**       controlled by the end user (DIP switches or similar), the application
**       shall keep these instances updated at all times because some networks
**       require that a changed switch is indicated by the LED’s.
**
**------------------------------------------------------------------------------
*/

#define ABP_NC_INST_NUM_SW1               0x01
#define ABP_NC_INST_NUM_SW2               0x02


/*******************************************************************************
**
** Application data object constants.
**
** Object revision: 3.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** Application data object specific attributes (ABCC40)
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_OA_NR_READ_PD_MAPPABLE_INSTANCES        11
#define ABP_APPD_OA_NR_WRITE_PD_MAPPABLE_INSTANCES       12
#define ABP_APPD_OA_NR_NV_INSTANCES                      13

/*------------------------------------------------------------------------------
**
** The data size of the Diagnostic object specific attrs, in bytes. (ABCC40)
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_OA_NR_READ_PD_MAPPABLE_INSTANCES_DS           ABP_UINT16_SIZEOF
#define ABP_APPD_OA_NR_WRITE_PD_MAPPABLE_INSTANCES_DS          ABP_UINT16_SIZEOF
#define ABP_APPD_OA_NR_NV_INSTANCES_DS                         ABP_UINT16_SIZEOF

/*------------------------------------------------------------------------------
**
** The Application data object instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_IA_NAME                  1
#define ABP_APPD_IA_DATA_TYPE             2
#define ABP_APPD_IA_NUM_ELEM              3
#define ABP_APPD_IA_DESCRIPTOR            4
#define ABP_APPD_IA_VALUE                 5
#define ABP_APPD_IA_MAX_VALUE             6
#define ABP_APPD_IA_MIN_VALUE             7
#define ABP_APPD_IA_DFLT_VALUE            8
#define ABP_APPD_IA_NUM_SUB_ELEM          9
#define ABP_APPD_IA_ELEM_NAME             10


/*------------------------------------------------------------------------------
**
** The Application data object attribute data sizes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_IA_DATA_TYPE_DS          ABP_UINT8_SIZEOF
#define ABP_APPD_IA_NUM_ELEM_DS           ABP_UINT8_SIZEOF
#define ABP_APPD_IA_DESCRIPTOR_DS         ABP_UINT8_SIZEOF
#define ABP_APPD_IA_NUM_SUB_ELEM_DS       ABP_UINT16_SIZEOF

/*------------------------------------------------------------------------------
**
** The Application data instance attribute descriptor bits.
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_DESCR_GET_ACCESS         0x01
#define ABP_APPD_DESCR_SET_ACCESS         0x02
#define ABP_APPD_DESCR_MAPPABLE_WRITE_PD  0x08
#define ABP_APPD_DESCR_MAPPABLE_READ_PD   0x10
#define ABP_APPD_DESCR_NVS_PARAMETER      0x20


/*------------------------------------------------------------------------------
**
** The Application data object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_CMD_GET_INST_BY_ORDER    0x10
#define ABP_APPD_GET_PROFILE_INST_NUMBERS 0x11
#define ABP_APPD_GET_ADI_INFO             0x12   /* ABCC40 deprecated shall not be used */
#define ABP_APPD_REMAP_ADI_WRITE_AREA     0x13
#define ABP_APPD_REMAP_ADI_READ_AREA      0x14
#define ABP_APPD_GET_INSTANCE_NUMBERS     0x15   /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The Application data object specific error codes.
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_ERR_MAPPING_ITEM_NAK      0x01 /* Mapping item NAK           */
#define ABP_APPD_ERR_INVALID_TOTAL_SIZE    0x02 /* Invalid total size         */
#define ABP_APPD_ERR_ATTR_CTRL_FROM_OTHER_CHANNEL 0x03  /* ABCC40 */


/*------------------------------------------------------------------------------
**
** Definitions for the different lists that can be fetched with the
** Get_Instance_Numbers command towards the Application Data Object.
**
**------------------------------------------------------------------------------
*/

#define ABP_APPD_LIST_TYPE_ALL             0x01
#define ABP_APPD_LIST_TYPE_RD_PD_MAPPABLE  0x02
#define ABP_APPD_LIST_TYPE_WR_PD_MAPPABLE  0x03
#define ABP_APPD_LIST_TYPE_NVS_PARAMS      0x04

/*******************************************************************************
**
** Application object constants.
**
** Object revision: 1.
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** The Application instance attributes.
**
**------------------------------------------------------------------------------
*/

#define ABP_APP_IA_CONFIGURED             1
#define ABP_APP_IA_SUP_LANG               2
#define ABP_APP_IA_SER_NUM                3   /* ABCC40 */
#define ABP_APP_IA_PAR_CRTL_SUM           4   /* ABCC40 */
#define ABP_APP_IA_FW_AVAILABLE           5   /* ABCC40 */
#define ABP_APP_IA_HW_CONF_ADDR           6   /* ABCC40 */
#define ABP_APP_IA_MODE                   7   /* ABCC40 */
#define ABP_APP_IA_VENDOR_NAME            8   /* ABCC40 */
#define ABP_APP_IA_PRODUCT_NAME           9   /* ABCC40 */
#define ABP_APP_IA_FW_VERSION             10  /* ABCC40 */
#define ABP_APP_IA_HW_VERSION             11  /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The data size of the Application instance attributes (in bytes).
**
**------------------------------------------------------------------------------
*/

#define ABP_APP_IA_CONFIGURED_DS          ABP_BOOL_SIZEOF
#define ABP_APP_IA_SER_NUM_DS             ABP_UINT32_SIZEOF         /* ABCC40 */
#define ABP_APP_IA_PAR_CRTL_SUM_DS        ( 16 * ABP_UINT8_SIZEOF ) /* ABCC40 */
#define ABP_APP_IA_FW_AVAILABLE_DS        ABP_BOOL_SIZEOF           /* ABCC40 */
#define ABP_APP_IA_HW_CONF_ADDR_DS        ABP_BOOL_SIZEOF           /* ABCC40 */
#define ABP_APP_IA_MODE_DS                ABP_BITS32_SIZEOF         /* ABCC40 */
#define ABP_APP_IA_VENDOR_NAME_MAX_DS     ( 64 * ABP_CHAR_SIZEOF )  /* ABCC40 */
#define ABP_APP_IA_PRODUCT_NAME_MAX_DS    ( 64 * ABP_CHAR_SIZEOF )  /* ABCC40 */
#define ABP_APP_IA_FW_VERSION_DS          ( 3 * ABP_UINT8_SIZEOF )  /* ABCC40 */
#define ABP_APP_IA_HW_VERSION_DS          ABP_UINT16_SIZEOF         /* ABCC40 */


/*------------------------------------------------------------------------------
**
** The Application object specific message commands.
**
**------------------------------------------------------------------------------
*/

#define ABP_APP_CMD_RESET_REQUEST         0x10
#define ABP_APP_CMD_CHANGE_LANG_REQUEST   0x11
#define ABP_APP_CMD_RESET_DIAGNOSTIC      0x12   /* ABCC40 */
#define ABP_APP_CMD_GET_DATA_NOTIF        0x13   /* ABCC40 */


/*------------------------------------------------------------------------------
**
** Definition for different modes for attribute ABP_APP_IA_MODE.
**
**------------------------------------------------------------------------------
*/

#define ABP_APP_MODE_NORMAL_LED          0x00000000
#define ABP_APP_MODE_AIDA_LED            0x00000001


/*------------------------------------------------------------------------------
**
** Definitions of Dataset
**
**------------------------------------------------------------------------------
*/

typedef enum ABP_AppDataset
{
   ABP_APP_DATASET_SINGLEADI       = 0,
   ABP_APP_DATASET_ASSEMBLYMAPPING = 1,
   ABP_APP_DATASET_TRANSPARENT     = 2
}
ABP_AppDatasetType;

#define ABP_APP_NW_CHANNELS_MQTT_BIT 0x0001


/*------------------------------------------------------------------------------
**
** Definitions of NotificationEntry
**
**------------------------------------------------------------------------------
*/

#define ABP_APP_NOTIFENTRY_SUBIDENT_BIT       0x0001
#define ABP_APP_NOTIFENTRY_VALUE_BIT          0x0002
#define ABP_APP_NOTIFENTRY_TIMESTAMP_BIT      0x0004


/*******************************************************************************
**
** Typedefs
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** PACKED_STRUCT
** ABCC_SYS_PACK_ON
** ABCC_SYS_PACK_OFF
**
** Compiler dependent symbols to pack structures for compilers that
** need an in-line directive.
**
**------------------------------------------------------------------------------
*/

#ifndef PACKED_STRUCT
   #if defined( __GNUC__ )
      #define PACKED_STRUCT   __attribute__ ((packed))
   #else
      #define PACKED_STRUCT
   #endif
#endif

#ifndef ABCC_SYS_PACK_ON
   #define ABCC_SYS_PACK_ON
#endif

#ifndef ABCC_SYS_PACK_OFF
   #define ABCC_SYS_PACK_OFF
#endif

/*------------------------------------------------------------------------------
**
** ABP_Msg255HeaderType
**
** Structure describing a message header.
**
**------------------------------------------------------------------------------
*/

ABCC_SYS_PACK_ON
typedef struct ABP_Msg255HeaderType
{
   UINT8    bSourceId;
   UINT8    bDestObj;
   UINT16   iInstance;
   UINT8    bCmd;
   UINT8    bDataSize;
   UINT8    bCmdExt0;
   UINT8    bCmdExt1;
}
PACKED_STRUCT ABP_Msg255HeaderType;
ABCC_SYS_PACK_OFF

/*------------------------------------------------------------------------------
**
** ABP_Msg255Type
**
** Structure describing a message.
**
**------------------------------------------------------------------------------
*/

ABCC_SYS_PACK_ON
typedef struct ABP_Msg255Type
{
   /*
   ** The message header part.
   */

   ABP_Msg255HeaderType sHeader;

   /*
   ** The message data.
   */

   UINT8    abData[ ABP_MAX_MSG_255_DATA_BYTES ];
}
PACKED_STRUCT ABP_Msg255Type;
ABCC_SYS_PACK_OFF

/*------------------------------------------------------------------------------
**
** ABP_MsgHeaderType
**
** Structure describing a message header for an 8 bit char platform
**
**------------------------------------------------------------------------------
*/

ABCC_SYS_PACK_ON
typedef struct ABP_MsgHeaderType
{
   UINT16   iDataSize;
   UINT16   iReserved;
   UINT8    bSourceId;
   UINT8    bDestObj;
   UINT16   iInstance;
   UINT8    bCmd;
   UINT8    bReserved;
   UINT8    bCmdExt0;
   UINT8    bCmdExt1;
}
PACKED_STRUCT ABP_MsgHeaderType;
ABCC_SYS_PACK_OFF

/*------------------------------------------------------------------------------
**
** ABP_MsgHeaderType16
**
** Structure describing a message header for a 16 bit char platform
**
**------------------------------------------------------------------------------
*/

ABCC_SYS_PACK_ON
typedef struct ABP_MsgHeaderType16
{
   UINT16   iDataSize;
   UINT16   iReserved;
   UINT16   iSourceIdDestObj;
   UINT16   iInstance;
   UINT16   iCmdReserved;
   UINT16   iCmdExt0CmdExt1;
}
PACKED_STRUCT ABP_MsgHeaderType16;
ABCC_SYS_PACK_OFF

/*------------------------------------------------------------------------------
**
** ABP_MsgType8
**
** Structure describing a message for an 8 bit char platform
**
**------------------------------------------------------------------------------
*/

ABCC_SYS_PACK_ON
typedef struct ABP_MsgType8
{
   /*
   ** The message header part.
   */

   ABP_MsgHeaderType sHeader;

   /*
   ** The message data.
   */

   UINT8    abData[ ABP_MAX_MSG_DATA_BYTES ];
}
PACKED_STRUCT ABP_MsgType8;
ABCC_SYS_PACK_OFF

/*------------------------------------------------------------------------------
**
** ABP_MsgType16
**
** Structure describing a message for a 16 bit char platform
**
**------------------------------------------------------------------------------
*/

ABCC_SYS_PACK_ON
typedef struct ABP_MsgType16
{
   /*
   ** The message header part.
   */

   ABP_MsgHeaderType16 sHeader;

   /*
   ** The message data.
   */

   UINT16    aiData[ ( ABP_MAX_MSG_DATA_BYTES + 1 ) >> 1 ];
}
PACKED_STRUCT ABP_MsgType16;
ABCC_SYS_PACK_OFF

/*------------------------------------------------------------------------------
**
** ABP_MsgType
**
** Typedef to ABP_MsgType8 or ABP_MsgType16 depending on platform.
**
**------------------------------------------------------------------------------
*/

#ifdef ABCC_SYS_16_BIT_CHAR
typedef struct ABP_MsgType16 ABP_MsgType;
#else
typedef struct ABP_MsgType8 ABP_MsgType;
#endif

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** ABP_BitSize_PADx()
** ABP_BitSize_BITx()
**
** Returns the number of bits occupied by the supplied type.
** NOTE: The macro does not check that the supplied type is of the specific type
** class. The type class has to be verified separately.
**
**------------------------------------------------------------------------------
**
** Inputs:
**    bType                - Type code
**
** Outputs:
**    Returns the number of bits occupied by the supplied type.
**
** Usage:
**    if( ABP_Is_PADx( bType ) )
**      bOccupiedBits = ABP_BitSize_PADx( bType );
**
**------------------------------------------------------------------------------
*/

#define ABP_BitSize_PADx( bType ) ( (bType) & 0x1F )
#define ABP_BitSize_BITx( bType ) ( (bType) & 0x07 )

/*------------------------------------------------------------------------------
**
** ABP_Is_PADx()
** ABP_Is_BITx()
**
** Check if the supplied type is of the specific type class (PADx or BITx).
** Returns logical true if so, and false if not.
**
**------------------------------------------------------------------------------
**
** Inputs:
**    bType                - Type code
**
** Outputs:
**    Returns logical true if supplied type code is of the specific type class.
**
** Usage:
**    if( ABP_Is_PADx( bType ) )
**
**------------------------------------------------------------------------------
*/

#define ABP_Is_BITx( bType )  ( ( (bType) >= ABP_BIT1 ) && ( (bType) <= ABP_BIT7 ) )
#define ABP_Is_PADx( bType )  ( ( (bType) >= ABP_PAD0 ) && ( (bType) <= ABP_PAD16 ) )

/*------------------------------------------------------------------------------
** ABP_IsTypeWithBitAlignment
**
** Check if the supplied type is of a type which is possible to map with bit
** alignment (as opposed to byte alignment).
**------------------------------------------------------------------------------
** Arguments:
**    bType                   - ABCC type specifier
**
** Returns:
**    TRUE if type have bit alignment, else FALSE.
**------------------------------------------------------------------------------
*/
#define ABP_IsTypeWithBitAlignment( bType ) ( ABP_Is_BITx( (bType) ) || ABP_Is_PADx( (bType) ) || \
                                            ( bType == ABP_BOOL1 ) )

/*------------------------------------------------------------------------------
**
** ABP_SetMsg255ErrorResponse()
**
** Converts a message command header into an error response header.
** It clears the C-bit, sets the E-bit and enters the submitted error code.
**
**------------------------------------------------------------------------------
**
** Inputs:
**    psMsg                - Pointer to the message command to convert.
**    bMsgDataSize         - The number of valid message data field bytes.
**    eErr                 - The requested error code (ABP_MsgErrorCodeType).
**
** Outputs:
**    None
**
** Usage:
**    ABP_SetMsg255ErrorResponse( psMsg, bMsgDataSize, eErr );
**
**------------------------------------------------------------------------------
*/

#define ABP_SetMsg255ErrorResponse( psMsg, bMsgDataSize, eErr )                \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.bCmd      &= ~ABP_MSG_HEADER_C_BIT;                        \
   (psMsg)->sHeader.bCmd      |=  ABP_MSG_HEADER_E_BIT;                        \
   (psMsg)->sHeader.bDataSize  =  (bMsgDataSize);                              \
   (psMsg)->abData[ 0 ]        =  (UINT8)(eErr);                               \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsg255ErrorResponse() */

/*------------------------------------------------------------------------------
**
** ABP_SetMsgErrorResponse()
**
** Converts a message command header into an error response header.
** It clears the C-bit, sets the E-bit and enters the submitted error code.
**
**------------------------------------------------------------------------------
**
** Inputs:
**    psMsg                - Pointer to the message command to convert.
**    iMsgDataSize         - The number of valid message data field bytes.
**    eErr                 - The requested error code (ABP_MsgErrorCodeType).
**
** Outputs:
**    None
**
** Usage:
**    ABP_SetMsgErrorResponse( psMsg, iMsgDataSize, eErr );
**
**------------------------------------------------------------------------------
*/

#ifdef ABCC_SYS_BIG_ENDIAN
#define ABP_SetMsgErrorResponse16( psMsg, iMsgDataSize, eErr )                 \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.iCmdReserved   &= ~( (UINT16)ABP_MSG_HEADER_C_BIT << 8 );  \
   (psMsg)->sHeader.iCmdReserved   |=  (UINT16)ABP_MSG_HEADER_E_BIT << 8;      \
   (psMsg)->sHeader.iDataSize       =  ( (UINT16)(iMsgDataSize) << 8 ) | ( (UINT16)(iMsgDataSize) >> 8 ); \
   (psMsg)->aiData[ 0 ]             =  ( (UINT16)(eErr) << 8 ) | ( (UINT16)(eErr) >> 8 ); \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgErrorResponse() */
#else
#define ABP_SetMsgErrorResponse16( psMsg, iMsgDataSize, eErr )                 \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.iCmdReserved   &= ~ABP_MSG_HEADER_C_BIT;                   \
   (psMsg)->sHeader.iCmdReserved   |=  ABP_MSG_HEADER_E_BIT;                   \
   (psMsg)->sHeader.iDataSize       =  (iMsgDataSize);                         \
   (psMsg)->aiData[ 0 ]             =  (UINT16)(eErr);                         \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgErrorResponse() */
#endif

#ifdef ABCC_SYS_BIG_ENDIAN
#define ABP_SetMsgErrorResponse8( psMsg, iMsgDataSize, eErr )                  \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.bCmd      &= ~ABP_MSG_HEADER_C_BIT;                        \
   (psMsg)->sHeader.bCmd      |=  ABP_MSG_HEADER_E_BIT;                        \
   (psMsg)->sHeader.iDataSize  =  ( (UINT16)(iMsgDataSize) << 8 ) | ( (UINT16)(iMsgDataSize) >> 8 ); \
   (psMsg)->abData[ 0 ]        =  (UINT8)(eErr);                               \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgErrorResponse() */
#else
#define ABP_SetMsgErrorResponse8( psMsg, iMsgDataSize, eErr )                  \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.bCmd      &= ~ABP_MSG_HEADER_C_BIT;                        \
   (psMsg)->sHeader.bCmd      |=  ABP_MSG_HEADER_E_BIT;                        \
   (psMsg)->sHeader.iDataSize  =  (iMsgDataSize);                              \
   (psMsg)->abData[ 0 ]        =  (UINT8)(eErr);                               \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgErrorResponse() */
#endif


#ifdef ABCC_SYS_16_BIT_CHAR
#define ABP_SetMsgErrorResponse( psMsg, iMsgDataSize, eErr ) ABP_SetMsgErrorResponse16( psMsg, iMsgDataSize, eErr )
#else
#define ABP_SetMsgErrorResponse( psMsg, iMsgDataSize, eErr ) ABP_SetMsgErrorResponse8( psMsg, iMsgDataSize, eErr )
#endif





/*------------------------------------------------------------------------------
**
** ABP_SetMsg255Response()
**
** Converts a message command header into a response header.
** It clears the C-bit and enters the submitted data size.
**
**------------------------------------------------------------------------------
**
** Inputs:
**    psMsg                - Pointer to the message command to convert.
**    bMsgDataSize         - The number of valid message data field bytes.
**
** Outputs:
**    None
**
** Usage:
**    ABP_SetMsg255Response( psMsg, bMsgDataSize );
**
**------------------------------------------------------------------------------
*/

#define ABP_SetMsg255Response( psMsg, bMsgDataSize )                           \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.bCmd      &= ~ABP_MSG_HEADER_C_BIT;                        \
   (psMsg)->sHeader.bDataSize  =  (bMsgDataSize);                              \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsg255Response() */

/*------------------------------------------------------------------------------
**
** ABP_SetMsgResponse()
**
** Converts a message command header into a response header.
** It clears the C-bit and enters the submitted data size.
**
**------------------------------------------------------------------------------
**
** Inputs:
**    psMsg                - Pointer to the message command to convert.
**    iMsgDataSize         - The number of valid message data field bytes.
**
** Outputs:
**    None
**
** Usage:
**    ABP_SetMsgResponse( psMsg, iMsgDataSize );
**
**------------------------------------------------------------------------------
*/

#ifdef ABCC_SYS_BIG_ENDIAN
#define ABP_SetMsgResponse16( psMsg, iMsgDataSize )                            \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.iCmdReserved &= ~( (UINT16)ABP_MSG_HEADER_C_BIT << 8 );    \
   (psMsg)->sHeader.iDataSize  =  ( (UINT16)(iMsgDataSize) << 8 ) | ( (UINT16)(iMsgDataSize) >> 8 ); \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgResponse() */
#else
#define ABP_SetMsgResponse16( psMsg, iMsgDataSize )                            \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.iCmdReserved &= ~ABP_MSG_HEADER_C_BIT;                     \
   (psMsg)->sHeader.iDataSize     = (iMsgDataSize);                            \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgResponse() */
#endif

#ifdef ABCC_SYS_BIG_ENDIAN
#define ABP_SetMsgResponse8( psMsg, iMsgDataSize )                             \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.bCmd      &= ~ABP_MSG_HEADER_C_BIT;                        \
   (psMsg)->sHeader.iDataSize  =  ( (UINT16)(iMsgDataSize) << 8 ) | ( (UINT16)(iMsgDataSize) >> 8 ); \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgResponse() */
#else
#define ABP_SetMsgResponse8( psMsg, iMsgDataSize )                             \
do                                                                             \
{                                                                              \
   (psMsg)->sHeader.bCmd      &= ~ABP_MSG_HEADER_C_BIT;                        \
   (psMsg)->sHeader.iDataSize  =  (iMsgDataSize);                              \
                                                                               \
}                                                                              \
while( 0 ) /* end of ABP_SetMsgResponse() */
#endif


#ifdef ABCC_SYS_16_BIT_CHAR
#define ABP_SetMsgResponse( psMsg, iMsgDataSize ) ABP_SetMsgResponse16( psMsg, iMsgDataSize )
#else
#define ABP_SetMsgResponse( psMsg, iMsgDataSize ) ABP_SetMsgResponse8( psMsg, iMsgDataSize )
#endif


/*******************************************************************************
**
** Segmentation protocol control bits for CmdExt1
**
********************************************************************************
*/
#define ABP_MSG_CMDEXT1_SEG_FIRST      0x01
#define ABP_MSG_CMDEXT1_SEG_LAST       0x02
#define ABP_MSG_CMDEXT1_SEG_ABORT      0x04

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of abp.h
**
********************************************************************************
*/

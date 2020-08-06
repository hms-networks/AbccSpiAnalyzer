/******************************************************************************
**  Copyright (C) 2015-2019 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerLookup.h
**    Summary: Lookup Table routines for DLL-Results
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include <cstring>
#include <iostream>
#include <sstream>

#include "AnalyzerHelpers.h"
#include "AbccSpiAnalyzerTypes.h"
#include "AbccSpiAnalyzer.h"
#include "AbccSpiAnalyzerResults.h"
#include "AbccSpiAnalyzerSettings.h"
#include "AbccSpiAnalyzerLookup.h"

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#include "abcc_abp/abp_add.h"
#include "abcc_abp/abp_asm.h"
#include "abcc_abp/abp_bac.h"
#include "abcc_abp/abp_ccl.h"
#include "abcc_abp/abp_cfn.h"
#include "abcc_abp/abp_cipid.h"
#include "abcc_abp/abp_cnt.h"
#include "abcc_abp/abp_cop.h"
#include "abcc_abp/abp_cpc.h"
#include "abcc_abp/abp_cpn.h"
#include "abcc_abp/abp_dev.h"
//#include "abcc_abp/abp_dpv0di.h" /* Currently not used */
#include "abcc_abp/abp_dpv1.h"
#include "abcc_abp/abp_eco.h"
#include "abcc_abp/abp_ect.h"
#include "abcc_abp/abp_eip.h"
#include "abcc_abp/abp_eme.h"
#include "abcc_abp/abp_epl.h"
#include "abcc_abp/abp_er.h"
#include "abcc_abp/abp_etn.h"
#include "abcc_abp/abp_fsi.h"
#include "abcc_abp/abp_fusm.h"
#include "abcc_abp/abp_mdd.h"
#include "abcc_abp/abp_mod.h"
#include "abcc_abp/abp_mqtt.h"
#include "abcc_abp/abp_nwccl.h"
#include "abcc_abp/abp_nwcfn.h"
#include "abcc_abp/abp_nwdpv1.h"
#include "abcc_abp/abp_nwetn.h"
#include "abcc_abp/abp_nwpnio.h"
#include "abcc_abp/abp_opcua.h"
#include "abcc_abp/abp_pnam.h"
#include "abcc_abp/abp_pnio.h"
#include "abcc_abp/abp_safe.h"
#include "abcc_abp/abp_smtp.h"
#include "abcc_abp/abp_soc.h"
#include "abcc_abp/abp_src3.h"
#include "abcc_abp/abp_sync.h"

#define IS_CMD_STANDARD(cmd)			(cmd > 0) && (cmd < 9)
#define IS_CMD_OBJECT_SPECIFIC(cmd) 	(((cmd >= 0x10) && (cmd <= 0x30)) || \
										(cmd == 0x3F))

#define NUM_ENTRIES(lut)				( sizeof(lut) / sizeof(LookupTable_t) )

#define ABCC_MSG_SIZE_FIELD_SIZE		2
#define ABCC_MSG_RES1_FIELD_SIZE		2
#define ABCC_MSG_SRC_ID_FIELD_SIZE		1
#define ABCC_MSG_OBJ_FIELD_SIZE			1
#define ABCC_MSG_INST_FIELD_SIZE		2
#define ABCC_MSG_CMD_FIELD_SIZE			1
#define ABCC_MSG_RES2_FIELD_SIZE		1
#define ABCC_MSG_CMDEXT_FIELD_SIZE		2
#define ABCC_MSG_CMDEXT0_FIELD_SIZE		1
#define ABCC_MSG_CMDEXT1_FIELD_SIZE		1
#define ABCC_MSG_DATA_FIELD_SIZE		1

typedef struct AttributeNameTable
{
	U8 object_num;
	U8 num_obj_names;
	U8 num_inst_names;
	const AttrLookupTable_t* obj_names;
	const AttrLookupTable_t* inst_names;
} AttributeNameTable_t;

typedef struct ErrorNameTable
{
	U8 object_num;
	U8 num_err_names;
	const LookupTable_t* err_names;
} ErrorNameTable_t;

typedef struct CommandNameTable
{
	U8 object_num;
	U8 num_cmd_names;
	const CmdLookupTable_t* cmd_names;
} CommandNameTable_t;

typedef struct NcInstNameTable
{
	U8 object_num;
	U8 num_inst_names;
	const LookupTable_t* inst_names;
} NcInstNameTable_t;


/*******************************************************************************
**
** Protocol state lookup tables
**
*******************************************************************************/

const AbccMosiInfo_t asMosiStates[] =
{
	{ AbccMosiStates::Idle,								"",			0 },
	{ AbccMosiStates::SpiControl,						"SPI_CTL",	1 },
	{ AbccMosiStates::Reserved1,						"RES",		1 },
	{ AbccMosiStates::MessageLength,					"MSG_LEN",	2 },
	{ AbccMosiStates::ProcessDataLength,				"PD_LEN",	2 },
	{ AbccMosiStates::ApplicationStatus,				"APP_STS",	1 },
	{ AbccMosiStates::InterruptMask,					"INT_MSK",	1 },
	{ AbccMosiStates::MessageField,						"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Size,				"MSG_SIZE",	ABCC_MSG_SIZE_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Reserved1,			"RES",		ABCC_MSG_RES1_FIELD_SIZE },
	{ AbccMosiStates::MessageField_SourceId,			"SRC_ID",	ABCC_MSG_SRC_ID_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Object,				"OBJ",		ABCC_MSG_OBJ_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Instance,			"INST",		ABCC_MSG_INST_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Command,				"CMD",		ABCC_MSG_CMD_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Reserved2,			"RES",		ABCC_MSG_RES2_FIELD_SIZE },
	{ AbccMosiStates::MessageField_CommandExtension,	"EXT",		ABCC_MSG_CMDEXT_FIELD_SIZE },
	{ AbccMosiStates::MessageField_Data,				"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ AbccMosiStates::WriteProcessData,					"PD",		1 },
	{ AbccMosiStates::Crc32,							"CRC32",	4 },
	{ AbccMosiStates::Pad,								"PAD",		2 },
	{ AbccMosiStates::MessageField_DataNotValid, 		"--",		1 }
};

const AbccMisoInfo_t asMisoStates[] =
{
	{ AbccMisoStates::Idle,								"",			0 },
	{ AbccMisoStates::Reserved1,						"RES",		1 },
	{ AbccMisoStates::Reserved2,						"RES",		1 },
	{ AbccMisoStates::LedStatus,						"LED_STS",	2 },
	{ AbccMisoStates::AnybusStatus,						"ANB_STS",	1 },
	{ AbccMisoStates::SpiStatus,						"SPI_STS",	1 },
	{ AbccMisoStates::NetworkTime,						"TIME",		4 },
	{ AbccMisoStates::MessageField,						"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Size,				"MD_SIZE",	ABCC_MSG_SIZE_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Reserved1,			"RES",		ABCC_MSG_RES1_FIELD_SIZE },
	{ AbccMisoStates::MessageField_SourceId,			"SRC_ID",	ABCC_MSG_SRC_ID_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Object,				"OBJ",		ABCC_MSG_OBJ_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Instance,			"INST",		ABCC_MSG_INST_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Command,				"CMD",		ABCC_MSG_CMD_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Reserved2,			"RES",		ABCC_MSG_RES2_FIELD_SIZE },
	{ AbccMisoStates::MessageField_CommandExtension,	"EXT",		ABCC_MSG_CMDEXT_FIELD_SIZE },
	{ AbccMisoStates::MessageField_Data,				"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ AbccMisoStates::ReadProcessData,					"PD",		1 },
	{ AbccMisoStates::Crc32,							"CRC32",	4 },
	{ AbccMisoStates::MessageField_DataNotValid, 		"--",		1 }

};

const AbccMsgInfo_t asMsgStates[] =
{
	{ AbccMsgField::Size,				"MD_SIZE",	ABCC_MSG_SIZE_FIELD_SIZE },
	{ AbccMsgField::Reserved1,			"RES",		ABCC_MSG_RES1_FIELD_SIZE },
	{ AbccMsgField::SourceId,			"SRC_ID",	ABCC_MSG_SRC_ID_FIELD_SIZE },
	{ AbccMsgField::Object,				"OBJ",		ABCC_MSG_OBJ_FIELD_SIZE },
	{ AbccMsgField::Instance,			"INST",		ABCC_MSG_INST_FIELD_SIZE },
	{ AbccMsgField::Command,			"CMD",		ABCC_MSG_CMD_FIELD_SIZE },
	{ AbccMsgField::Reserved2,			"RES",		ABCC_MSG_RES2_FIELD_SIZE },
	{ AbccMsgField::CommandExtension,	"EXT",		ABCC_MSG_CMDEXT_FIELD_SIZE },
	{ AbccMsgField::Data,				"MD",		ABCC_MSG_DATA_FIELD_SIZE }
};

/*******************************************************************************
**
** Basic SPI field enumeration lookup tables
**
*******************************************************************************/

static const LookupTable_t asAnybusStsNames[] =
{
	{ ABP_ANB_STATE_SETUP,			"SETUP",			NotifEvent::None },
	{ ABP_ANB_STATE_NW_INIT,		"NW_INIT",			NotifEvent::None },
	{ ABP_ANB_STATE_WAIT_PROCESS,	"WAIT_PROCESS",		NotifEvent::None },
	{ ABP_ANB_STATE_IDLE,			"IDLE",				NotifEvent::None },
	{ ABP_ANB_STATE_PROCESS_ACTIVE, "PROCESS_ACTIVE",	NotifEvent::None },
	{ ABP_ANB_STATE_ERROR,			"ERROR",			NotifEvent::Alert },
	{ ABP_ANB_STATE_EXCEPTION,		"EXCEPTION",		NotifEvent::Alert }
};

static const LookupTable_t asApplStsNames[] =
{
	{ ABP_APPSTAT_NO_ERROR,			"No Error",									NotifEvent::None },
	{ ABP_APPSTAT_NOT_SYNCED,		"Not yet synchronized",						NotifEvent::None },
	{ ABP_APPSTAT_SYNC_CFG_ERR,		"Sync configuration error",					NotifEvent::Alert },
	{ ABP_APPSTAT_READ_PD_CFG_ERR,	"Read process data configuration error",	NotifEvent::Alert },
	{ ABP_APPSTAT_WRITE_PD_CFG_ERR,	"Write process data configuration error",	NotifEvent::Alert },
	{ ABP_APPSTAT_SYNC_LOSS,		"Synchronization loss",						NotifEvent::Alert },
	{ ABP_APPSTAT_PD_DATA_LOSS,		"Excessive data loss",						NotifEvent::Alert },
	{ ABP_APPSTAT_OUTPUT_ERR,		"Output error",								NotifEvent::Alert },
	{ ABP_APPSTAT_GENERAL_SYNC_ERR,	"General sync error",						NotifEvent::Alert }
};

static const LookupTable_t asSpiStsNames[] =
{
	{ 0xC0,							"RESERVED",		NotifEvent::Alert }, // No ABP mask exists
	{ ABP_SPI_STATUS_NEW_PD,		"NEW_PD",		NotifEvent::None },
	{ ABP_SPI_STATUS_LAST_FRAG,		"LAST_FRAG",	NotifEvent::None },
	{ ABP_SPI_STATUS_M,				"M",			NotifEvent::None },
	{ ABP_SPI_STATUS_CMDCNT,		"CMDCNT",		NotifEvent::None },
	{ ABP_SPI_STATUS_WRMSG_FULL,	"WRMSG_FULL",	NotifEvent::Alert }
};

static const LookupTable_t asSpiCtrlNames[] =
{
	{ ABP_SPI_CTRL_T,			"TOGGLE",		NotifEvent::None },
	{ 0x60,						"RESERVED",		NotifEvent::Alert }, // No ABP mask exists
	{ ABP_SPI_CTRL_LAST_FRAG,	"LAST_FRAG",	NotifEvent::None },
	{ ABP_SPI_CTRL_M,			"M",			NotifEvent::None },
	{ ABP_SPI_CTRL_CMDCNT,		"CMDCNT",		NotifEvent::None },
	{ ABP_SPI_CTRL_WRPD_VALID,	"WRPD_VALID",	NotifEvent::None }
};

static const LookupTable_t asIntMaskNames[] =
{
	{ ABP_INTMASK_RDPDIEN,		"RDPD",		NotifEvent::None },
	{ ABP_INTMASK_RDMSGIEN,		"RDMSG",	NotifEvent::None },
	{ ABP_INTMASK_WRMSGIEN,		"WRMSG",	NotifEvent::None },
	{ ABP_INTMASK_ANBRIEN,		"ANBR",		NotifEvent::None },
	{ ABP_INTMASK_STATUSIEN,	"STATUS",	NotifEvent::None },
	{ 0x20,						"RESERVED",	NotifEvent::Alert }, // No ABP mask exists
	{ ABP_INTMASK_SYNCIEN,		"SYNC",		NotifEvent::None },
	{ 0x80,						"RESERVED",	NotifEvent::Alert }  // No ABP mask exists
};

static const LookupTable_t asLedStsNames[] =
{
	{ 0x0001, "LED1A",		NotifEvent::None },
	{ 0x0002, "LED1B",		NotifEvent::None },
	{ 0x0004, "LED2A",		NotifEvent::None },
	{ 0x0008, "LED2B",		NotifEvent::None },
	{ 0x0010, "LED3A",		NotifEvent::None },
	{ 0x0020, "LED3B",		NotifEvent::None },
	{ 0x0040, "LED4A",		NotifEvent::None },
	{ 0x0080, "LED4B",		NotifEvent::None },
	{ 0x0100, "LED5A",		NotifEvent::None },
	{ 0x0200, "LED5B",		NotifEvent::None },
	{ 0x0400, "LED6A",		NotifEvent::None },
	{ 0x0800, "LED6B",		NotifEvent::None },
	{ 0xF000, "RESERVED",	NotifEvent::Alert }
};

/*******************************************************************************
**
** Object name lookup table
**
*******************************************************************************/

static const LookupTable_t asObjectNames[] =
{
	/*--------------------------------------------------------------------------
	** Anybus module objects
	**--------------------------------------------------------------------------
	*/
	{ ABP_OBJ_NUM_ANB,		"Anybus",							NotifEvent::None },
	{ ABP_OBJ_NUM_DI,		"Diagnostic",						NotifEvent::None },
	{ ABP_OBJ_NUM_NW,		"Network",							NotifEvent::None },
	{ ABP_OBJ_NUM_NC,		"Network Configuration",			NotifEvent::None },
	{ ABP_OBJ_NUM_ADD,		"PROFIBUS DP-V1 Additional Diag",	NotifEvent::None },
	{ ABP_OBJ_NUM_RSV1,		"Reserved",							NotifEvent::Alert },
	{ ABP_OBJ_NUM_SOC,		"Socket Interface",					NotifEvent::None },
	{ ABP_OBJ_NUM_NWCCL,	"Network CC-Link",					NotifEvent::None },
	{ ABP_OBJ_NUM_SMTP,		"SMTP Client",						NotifEvent::None },
	{ ABP_OBJ_NUM_FSI,		"Anybus File System Interface",		NotifEvent::None },
	{ ABP_OBJ_NUM_NWDPV1,	"Network PROFIBUS DP-V1",			NotifEvent::None },
	{ ABP_OBJ_NUM_NWETN,	"Network Ethernet",					NotifEvent::None },
	{ ABP_OBJ_NUM_CPC,		"CIP Port Configuration",			NotifEvent::None },
	{ ABP_OBJ_NUM_NWPNIO,	"Network PROFINET IO",				NotifEvent::None },
	{ ABP_OBJ_NUM_PNIOADD,	"PROFINET IO Additional Diag",		NotifEvent::None },
	{ ABP_OBJ_NUM_DPV0DI,	"PROFIBUS DP-V0 Diagnostic",		NotifEvent::None },
	{ ABP_OBJ_NUM_FUSM,		"Functional Safety Module",			NotifEvent::None },
	{ ABP_OBJ_NUM_NWCFN,	"Network CC-Link IE Field Network",	NotifEvent::None },
	/*--------------------------------------------------------------------------
	** Host application objects
	**--------------------------------------------------------------------------
	*/
	{ 0x80,					"Host Application Specific",			NotifEvent::None }, /* No abp define exists yet */
	{ ABP_OBJ_NUM_MQTT,		"MQTT",									NotifEvent::None },
	{ ABP_OBJ_NUM_OPCUA,	"OPC Unified Architecture",				NotifEvent::None },
	{ ABP_OBJ_NUM_EME,		"Energy Measurement",					NotifEvent::None },
	{ ABP_OBJ_NUM_PNAM,		"PROFINET Asset Management",			NotifEvent::None },
	{ ABP_OBJ_NUM_CFN,		"CC-Link IE Field Network",				NotifEvent::None },
	{ ABP_OBJ_NUM_ER,		"Energy Reporting",						NotifEvent::None },
	{ ABP_OBJ_NUM_SAFE,		"Functional Safety",					NotifEvent::None },
	{ ABP_OBJ_NUM_EPL,		"POWERLINK",							NotifEvent::None },
	{ ABP_OBJ_NUM_AFSI,		"Application File System Interface",	NotifEvent::None },
	{ ABP_OBJ_NUM_ASM,		"Assembly Mapping",						NotifEvent::None },
	{ ABP_OBJ_NUM_MDD,		"Modular Device",						NotifEvent::None },
	{ ABP_OBJ_NUM_CIPID,	"CIP Identity",							NotifEvent::None },
	{ ABP_OBJ_NUM_SYNC,		"Sync",									NotifEvent::None },
	{ ABP_OBJ_NUM_BAC,		"BACnet",								NotifEvent::None },
	{ ABP_OBJ_NUM_ECO,		"Energy Control",						NotifEvent::None },
	{ ABP_OBJ_NUM_SRC3,		"SERCOS III",							NotifEvent::None },
	{ ABP_OBJ_NUM_PRD,		"PROFIdrive",							NotifEvent::None },
	{ ABP_OBJ_NUM_CNT,		"ControlNet",							NotifEvent::None },
	{ ABP_OBJ_NUM_CPN,		"CompoNet",								NotifEvent::None },
	{ ABP_OBJ_NUM_ECT,		"EtherCAT",								NotifEvent::None },
	{ ABP_OBJ_NUM_PNIO,		"PROFINET IO",							NotifEvent::None },
	{ ABP_OBJ_NUM_CCL,		"CC-Link",								NotifEvent::None },
	{ ABP_OBJ_NUM_EIP,		"EtherNet/IP",							NotifEvent::None },
	{ ABP_OBJ_NUM_ETN,		"Ethernet",								NotifEvent::None },
	{ ABP_OBJ_NUM_MOD,		"Modbus",								NotifEvent::None },
	{ ABP_OBJ_NUM_COP,		"CANopen",								NotifEvent::None },
	{ ABP_OBJ_NUM_DEV,		"DeviceNet",							NotifEvent::None },
	{ ABP_OBJ_NUM_DPV1,		"PROFIBUS DP-V1",						NotifEvent::None },
	{ ABP_OBJ_NUM_APPD,		"Application Data",						NotifEvent::None },
	{ ABP_OBJ_NUM_APP,		"Application",							NotifEvent::None }
};

/* Specified in same order as ABP.h */
const U8 abNetworkTypeValue[] =
{
	0x00,						// Unspecified
	//ABP_NW_TYPE_PDPV0,		// PROFIBUS DP-V0
	ABP_NW_TYPE_PDPV1,			// PROFIBUS DP-V1
	ABP_NW_TYPE_COP,			// CANopen
	ABP_NW_TYPE_DEV,			// DeviceNet
	//ABP_NW_TYPE_RTU,			// Modbus-RTU
	//ABP_NW_TYPE_CNT,			// ControlNet
	//ABP_NW_TYPE_ETN_1P,		// Modbus-TCP
	//ABP_NW_TYPE_PRT,			// PROFINET RT
	//ABP_NW_TYPE_EIP_1P,		// EtherNet/IP
	ABP_NW_TYPE_ECT,			// EtherCAT
	ABP_NW_TYPE_PIR,			// PROFINET IRT
	ABP_NW_TYPE_CCL,			// CC-Link
	ABP_NW_TYPE_ETN_2P,			// Modbus-TCP 2-Port
	//ABP_NW_TYPE_CPN,			// CompoNet
	//ABP_NW_TYPE_PRT_2P,		// PROFINET RT 2-port
	//ABP_NW_TYPE_SRC3,			// SERCOS III
	//ABP_NW_TYPE_BMP,			// BACnet MS/TP
	ABP_NW_TYPE_BIP,			// BACnet/IP
	ABP_NW_TYPE_EIP_2P_BB,		// EtherNet/IP 2-Port BB DLR
	//ABP_NW_TYPE_EIP_2P,		// EtherNet/IP 2-Port
	ABP_NW_TYPE_PIR_FO,			// PROFINET IRT FO
	ABP_NW_TYPE_EPL,			// POWERLINK
	ABP_NW_TYPE_CFN,			// CC-Link IE Field Network
	ABP_NW_TYPE_CET,			// Common Ethernet
	ABP_NW_TYPE_EIP_2P_BB_IIOT,	// EtherNet/IP IIoT
	ABP_NW_TYPE_PIR_IIOT,		// PROFINET IRT IIoT
	ABP_NW_TYPE_PIR_FO_IIOT,	// PROFINET IRT FO IIoT
	ABP_NW_TYPE_CET_IIOT		// Common Ethernet IIoT
};

/*******************************************************************************
**
** Network configuration object instance lookup tables
**
*******************************************************************************/

static const LookupTable_t asBipNcInstNames[] =
{
	{ 3,	"IP Address",											NotifEvent::None },
	{ 4,	"Subnet Mask",											NotifEvent::None },
	{ 5,	"Gateway Address",										NotifEvent::None },
	{ 6,	"DHCP Enable",											NotifEvent::None },
	{ 7,	"Ethernet Communication Settings 1",					NotifEvent::None },
	{ 8,	"Ethernet Communication Settings 2",					NotifEvent::None },
	{ 9,	"DNS 1",												NotifEvent::None },
	{ 10,	"DNS 2",												NotifEvent::None },
	{ 11,	"Host Name",											NotifEvent::None },
	{ 12,	"Domain Name",											NotifEvent::None },
	{ 13,	"SMTP Server",											NotifEvent::None },
	{ 14,	"SMTP User",											NotifEvent::None },
	{ 15,	"SMTP Password",										NotifEvent::None },
	{ 16,	"MDI 1 Settings",										NotifEvent::None },
	{ 17,	"MDI 2 Settings",										NotifEvent::None },
	{ 18,	"Reserved",												NotifEvent::None },
	{ 19,	"Reserved",												NotifEvent::None },
	{ 20,	"Device Instance",										NotifEvent::None },
	{ 21,	"UDP Port",												NotifEvent::None },
	{ 22,	"Process Active Timeout",								NotifEvent::None },
	{ 23,	"Foreign Device Registration IPAddress",				NotifEvent::None },
	{ 24,	"Foreign Device Registration UDP Port",					NotifEvent::None },
	{ 25,	"Foreign Device Registration Time to Live Value",		NotifEvent::None },
};

static const LookupTable_t asCclNcInstNames[] =
{
	{ 1,	"Station Number",						NotifEvent::None },
	{ 3,	"Network Number",						NotifEvent::None }
};

static const LookupTable_t asCetNcInstNames[] =
{
	{ 3,	"IP Address",							NotifEvent::None },
	{ 4,	"Subnet Mask",							NotifEvent::None },
	{ 5,	"Gateway Address",						NotifEvent::None },
	{ 6,	"DHCP Enable",							NotifEvent::None },
	{ 7,	"Ethernet Communication Settings 1",	NotifEvent::None },
	{ 8,	"Ethernet Communication Settings 2",	NotifEvent::None },
	{ 9,	"DNS 1",								NotifEvent::None },
	{ 10,	"DNS 2",								NotifEvent::None },
	{ 11,	"Host Name",							NotifEvent::None },
	{ 12,	"Domain Name",							NotifEvent::None },
	{ 13,	"SMTP Server",							NotifEvent::None },
	{ 14,	"SMTP User",							NotifEvent::None },
	{ 15,	"SMTP Password",						NotifEvent::None },
	{ 16,	"MDI 1 Settings",						NotifEvent::None },
	{ 17,	"MDI 2 Settings",						NotifEvent::None },
	{ 18,	"Reserved",								NotifEvent::None },
	{ 19,	"Reserved",								NotifEvent::None },
};

static const LookupTable_t asCopNcInstNames[] =
{
	{ 1,	"Device Address",	NotifEvent::None },
	{ 2,	"Baud Rate",		NotifEvent::None },
};

static const LookupTable_t asDevNcInstNames[] =
{
	{ 1,	"Node Address",						NotifEvent::None },
	{ 2,	"Baud Rate",						NotifEvent::None },
	{ 3,	"QuickConnect",						NotifEvent::None }
};

static const LookupTable_t asEctNcInstNames[] =
{
	{ 1,	"Device ID",							NotifEvent::None },
	{ 3,	"IP Address",							NotifEvent::None },
	{ 4,	"Subnet Mask",							NotifEvent::None },
	{ 5,	"Gateway Address",						NotifEvent::None },
	{ 6,	"DHCP Enable",							NotifEvent::None },
	{ 9,	"DNS 1",								NotifEvent::None },
	{ 10,	"DNS 2",								NotifEvent::None },
	{ 11,	"Host Name",							NotifEvent::None },
	{ 12,	"Domain Name",							NotifEvent::None },
	{ 13,	"SMTP Server",							NotifEvent::None },
	{ 14,	"SMTP User",							NotifEvent::None },
	{ 15,	"SMTP Password",						NotifEvent::None }
};

static const LookupTable_t asEipNcInstNames[] =
{
	{ 3,	"IP Address",							NotifEvent::None },
	{ 4,	"Subnet Mask",							NotifEvent::None },
	{ 5,	"Gateway Address",						NotifEvent::None },
	{ 6,	"DHCP Enable",							NotifEvent::None },
	{ 7,	"Ethernet Communication Settings 1",	NotifEvent::None },
	{ 8,	"Ethernet Communication Settings 2",	NotifEvent::None },
	{ 9,	"DNS 1",								NotifEvent::None },
	{ 10,	"DNS 2",								NotifEvent::None },
	{ 11,	"Host Name",							NotifEvent::None },
	{ 12,	"Domain Name",							NotifEvent::None },
	{ 13,	"SMTP Server",							NotifEvent::None },
	{ 14,	"SMTP User",							NotifEvent::None },
	{ 15,	"SMTP Password",						NotifEvent::None },
	{ 16,	"MDI 1 Settings",						NotifEvent::None },
	{ 17,	"MDI 2 Settings",						NotifEvent::None },
	{ 18,	"Reserved",								NotifEvent::None },
	{ 19,	"Reserved",								NotifEvent::None },
	{ 20,	"QuickConnect",							NotifEvent::None }
};

static const LookupTable_t asEplNcInstNames[] =
{
	{ 1,	"Node ID",	NotifEvent::None }
};

static const LookupTable_t asEtnNcInstNames[] =
{
	{ 3,	"IP Address",							NotifEvent::None },
	{ 4,	"Subnet Mask",							NotifEvent::None },
	{ 5,	"Gateway Address",						NotifEvent::None },
	{ 6,	"DHCP Enable",							NotifEvent::None },
	{ 7,	"Ethernet Communication Settings 1",	NotifEvent::None },
	{ 8,	"Ethernet Communication Settings 2",	NotifEvent::None },
	{ 9,	"DNS 1",								NotifEvent::None },
	{ 10,	"DNS 2",								NotifEvent::None },
	{ 11,	"Host Name",							NotifEvent::None },
	{ 12,	"Domain Name",							NotifEvent::None },
	{ 13,	"SMTP Server",							NotifEvent::None },
	{ 14,	"SMTP User",							NotifEvent::None },
	{ 15,	"SMTP Password",						NotifEvent::None },
	{ 16,	"MDI 1 Settings",						NotifEvent::None },
	{ 17,	"MDI 2 Settings",						NotifEvent::None },
	{ 18,	"Reserved",								NotifEvent::None },
	{ 19,	"Reserved",								NotifEvent::None },
	{ 20,	"Modbus Connection Timeout",			NotifEvent::None },
	{ 21,	"Process Active Timeout",				NotifEvent::None },
	{ 22,	"Word Order",							NotifEvent::None }
};

static const LookupTable_t asPirNcInstNames[] =
{
	{ 3,	"IP Address",							NotifEvent::None },
	{ 4,	"Subnet Mask",							NotifEvent::None },
	{ 5,	"Gateway Address",						NotifEvent::None },
	{ 6,	"DHCP Enable",							NotifEvent::None },
	{ 9,	"DNS 1",								NotifEvent::None },
	{ 10,	"DNS 2",								NotifEvent::None },
	{ 11,	"Host Name",							NotifEvent::None },
	{ 12,	"Domain Name",							NotifEvent::None },
	{ 13,	"SMTP Server",							NotifEvent::None },
	{ 14,	"SMTP User",							NotifEvent::None },
	{ 15,	"SMTP Password",						NotifEvent::None },
	{ 16,	"Reserved",								NotifEvent::None },
	{ 17,	"Reserved",								NotifEvent::None },
	{ 18,	"Reserved",								NotifEvent::None },
	{ 19,	"Reserved",								NotifEvent::None },
	{ 20,	"Station Name",							NotifEvent::None },
	{ 21,	"F-Address",							NotifEvent::None }
};

NcInstNameTable_t asNcInstNameTables[] =
{
	{ ABP_NW_TYPE_BIP,				NUM_ENTRIES(asBipNcInstNames),	asBipNcInstNames },
	{ ABP_NW_TYPE_CCL,				NUM_ENTRIES(asCclNcInstNames),	asCclNcInstNames },

	{ ABP_NW_TYPE_CET,				NUM_ENTRIES(asCetNcInstNames),	asCetNcInstNames },
	{ ABP_NW_TYPE_CET_IIOT,			NUM_ENTRIES(asCetNcInstNames),	asCetNcInstNames },

	{ ABP_NW_TYPE_COP,				NUM_ENTRIES(asCopNcInstNames),	asCopNcInstNames },
	{ ABP_NW_TYPE_DEV,				NUM_ENTRIES(asDevNcInstNames),	asDevNcInstNames },
	{ ABP_NW_TYPE_ECT,				NUM_ENTRIES(asEctNcInstNames),	asEctNcInstNames },

	{ ABP_NW_TYPE_EIP_1P,			NUM_ENTRIES(asEipNcInstNames),	asEipNcInstNames },
	{ ABP_NW_TYPE_EIP_2P_BB,		NUM_ENTRIES(asEipNcInstNames),	asEipNcInstNames },
	{ ABP_NW_TYPE_EIP_2P,			NUM_ENTRIES(asEipNcInstNames),	asEipNcInstNames },
	{ ABP_NW_TYPE_EIP_2P_BB_IIOT,	NUM_ENTRIES(asEipNcInstNames),	asEipNcInstNames },

	{ ABP_NW_TYPE_EPL,				NUM_ENTRIES(asEplNcInstNames),	asEplNcInstNames },

	{ ABP_NW_TYPE_ETN_1P,			NUM_ENTRIES(asEtnNcInstNames),	asEtnNcInstNames },
	{ ABP_NW_TYPE_ETN_2P,			NUM_ENTRIES(asEtnNcInstNames),	asEtnNcInstNames },

	{ ABP_NW_TYPE_PIR,				NUM_ENTRIES(asPirNcInstNames),	asPirNcInstNames },
	{ ABP_NW_TYPE_PIR_FO,			NUM_ENTRIES(asPirNcInstNames),	asPirNcInstNames },
	{ ABP_NW_TYPE_PIR_FO_IIOT,		NUM_ENTRIES(asPirNcInstNames),	asPirNcInstNames },
	{ ABP_NW_TYPE_PIR_IIOT,			NUM_ENTRIES(asPirNcInstNames),	asPirNcInstNames },
};

/*******************************************************************************
**
** Common object attribute lookup table
**
*******************************************************************************/

static const AttrLookupTable_t asObjAttrNames[] =
{
	{ ABP_OA_NAME,			"Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_OA_REV,			"Revision",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_OA_NUM_INST,		"Number of Instances",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_OA_HIGHEST_INST,	"Highest Instance Number",	BaseType::Numeric,		NotifEvent::None }
};

/*******************************************************************************
**
** Object-specific object/instance attribute lookup table
**
*******************************************************************************/

static const AttrLookupTable_t asAddObjAttrNames[] =
{
	{ ABP_ADD_OA_MAX_INST,			"Max Instance",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ADD_OA_EXT_DIAG_OVERFLOW,	"Ext Diag Overflow",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ADD_OA_STATIC_DIAG,		"Static Diag",			BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asAddInstAttrNames[] =
{
	{ ABP_ADD_IA_MODULE_NUMBER,		"Module Number",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ADD_IA_IO_TYPE,			"IO Type",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ADD_IA_CHANNEL_NUMBER,	"Channel Number",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ADD_IA_CHANNEL_TYPE,		"Channel Type",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_ADD_IA_ERROR_TYPE,		"Error Type",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asAnbInstAttrNames[] =
{
	{ ABP_ANB_IA_MODULE_TYPE,		"Module Type",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_FW_VERSION,		"Firmware Version",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_SERIAL_NUM,		"Serial Number",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_WD_TIMEOUT,		"Watchdog Timeout",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_SETUP_COMPLETE,	"Setup Complete",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_EXCEPTION,			"Exception",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_FATAL_EVENT,		"Fatal Event",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_ERROR_CNTRS,		"Error Counters",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_LANG,				"Language",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_PROVIDER_ID,		"Provider ID",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_PROVIDER_INFO,		"Provider Info",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_LED_COLOURS,		"LED Colors",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_LED_STATUS,		"LED Status",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_SWITCH_STATUS,		"Switch Status",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_AUX_BIT_FUNC,		"Auxilary Bit Function",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_GPIO_CONFIG,		"GPIO Configuration",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_VIRTUAL_ATTRS,		"Virtual Attributes",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_BLACK_WHITE_LIST,	"Black/White List",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_NETWORK_TIME,		"Network Time",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_FW_CUST_VERSION,	"FW Custom Version",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_ANB_IA_ABIP_LICENSE,		"Anybus IP License",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asAppInstAttrNames[] =
{
	{ ABP_APP_IA_CONFIGURED,	"Configured",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_SUP_LANG,		"Supported Languages",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_SER_NUM,		"Serial Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_PAR_CRTL_SUM,	"Parameter Control Sum",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_FW_AVAILABLE,	"Candidate Firmware Available",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_HW_CONF_ADDR,	"Hardware Configurable Address",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_MODE,			"Mode",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_VENDOR_NAME,	"Vendor Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_APP_IA_PRODUCT_NAME,	"Product Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_APP_IA_FW_VERSION,	"FW Version",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_IA_HW_VERSION,	"HW Version",						BaseType::Numeric,		NotifEvent::None },
};

static const AttrLookupTable_t asAppdObjAttrNames[] =
{
	{ ABP_APPD_OA_NR_READ_PD_MAPPABLE_INSTANCES,	"No. of RD PD Mappable Instances",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_APPD_OA_NR_WRITE_PD_MAPPABLE_INSTANCES,	"No. of WR PD Mappable Instances",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_APPD_OA_NR_NV_INSTANCES,					"No. of Non-Volatile Instances",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asAppdInstAttrNames[] =
{
	{ ABP_APPD_IA_NAME,			"Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_APPD_IA_DATA_TYPE,	"Data Type",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_NUM_ELEM,		"Number of Elements",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_DESCRIPTOR,	"Descriptor",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_VALUE,		"Value(s)",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_MAX_VALUE,	"Max Value",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_MIN_VALUE,	"Min Value",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_DFLT_VALUE,	"Default Value",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_NUM_SUB_ELEM,	"Number of SubElements",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_APPD_IA_ELEM_NAME,	"Element Name",				BaseType::Character,	NotifEvent::None }
};

static const AttrLookupTable_t asAsmObjAttrNames[] =
{
	{ ABP_ASM_OA_WRITE_PD_INST_LIST,	"Write PD Instance List",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ASM_OA_READ_PD_INST_LIST,		"Read PD Instance List",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asAsmInstAttrNames[] =
{
	{ ABP_ASM_IA_DESCRIPTOR,		"Assembly Descriptor",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 0,	"ADI Map 0",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 1,	"ADI Map 1",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 2,	"ADI Map 2",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 3,	"ADI Map 3",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 4,	"ADI Map 4",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 5,	"ADI Map 5",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 6,	"ADI Map 6",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 7,	"ADI Map 7",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 8,	"ADI Map 8",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 9,	"ADI Map 9",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_ADI_MAP_XX + 10,	"ADI Map 10",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ASM_IA_NAME,				"Assembly Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_ASM_IA_MAX_NUM_ADI_MAPS,	"Max Number of ADI Mappings",	BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asBacInstAttrNames[] =
{
	{ ABP_BAC_IA_OBJECT_NAME,			"Object Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_BAC_IA_VENDOR_NAME,			"Vendor Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_BAC_IA_VENDOR_IDENTIFIER,		"Vendor Identifier",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_BAC_IA_MODEL_NAME,			"Model Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_BAC_IA_FIRMWARE_REVISION,		"Firmware Revision",		BaseType::Character,	NotifEvent::None },
	{ ABP_BAC_IA_APP_SOFTWARE_VERSION,	"Software Revision",		BaseType::Character,	NotifEvent::None },
	{ ABP_BAC_IA_SUPPORT_ADV_MAPPING,	"Support Advanced Mapping",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_BAC_IA_CURRENT_DATE_AND_TIME,	"Current Date and Time",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_BAC_IA_PASSWORD,				"Password",					BaseType::Character,	NotifEvent::None }
};

static const AttrLookupTable_t asCclInstAttrNames[] =
{
	{ ABP_CCL_IA_VENDOR_CODE,			"Vendor Code",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_CCL_IA_SOFTWARE_VERSION,		"Software Version",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_CCL_IA_MODEL_CODE,			"Model Code",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_CCL_IA_NETWORK_SETTINGS,		"Network Settings",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_CCL_IA_SYS_AREA_HANDLER,		"System Area Handler",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_CCL_IA_HOLD_CLEAR_SETTING,	"Hold Clear Setting",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asCfnInstAttrNames[] =
{
	{ ABP_CFN_IA_VENDOR_CODE,		"Vendor Code",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CFN_IA_VENDOR_NAME,		"Vendor Name",			BaseType::Character,	NotifEvent::None },
	{ ABP_CFN_IA_MODEL_TYPE,		"Model Type",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CFN_IA_MODEL_NAME,		"Model Name",			BaseType::Character,	NotifEvent::None },
	{ ABP_CFN_IA_MODEL_CODE,		"Model Code",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CFN_IA_SW_VERSION,		"SW Version",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CFN_IA_ENABLE_SLMP,		"Enable SLMP",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CFN_IA_ENA_SLMP_FORWARD,	"Enable SLMP Forward",	BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asCipIdInstAttrNames[] =
{
	{ ABP_CIPID_IA_VENDOR_ID,		"Vendor ID",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_CIPID_IA_DEVICE_TYPE,		"Device Type",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_CIPID_IA_PRODUCT_CODE,	"Product Code",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_CIPID_IA_REVISION,		"Revision",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_CIPID_IA_STATUS,			"Status",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_CIPID_IA_SERIAL_NUMBER,	"Serial Number",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_CIPID_IA_PRODUCT_NAME,	"Product Name",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asCntInstAttrNames[] =
{
	{ ABP_CNT_IA_VENDOR_ID,					"Vendor ID",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_DEVICE_TYPE,				"Device Type",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_PRODUCT_CODE,				"Product Code",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_REVISION,					"Revision",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_SERIAL_NUMBER,				"Serial Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_PRODUCT_NAME,				"Product Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_CNT_IA_PROD_INSTANCE,				"Producing Instance",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_CONS_INSTANCE,				"Consuming Instance",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_ENABLE_APP_CIP_OBJECTS,	"Enable Application CIP Objects",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_ENABLE_PARAM_OBJECT,		"Enable Parameter Object",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CNT_IA_CONFIG_INSTANCE,			"Configuration Instance",			BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asCopInstAttrNames[] =
{
	{ ABP_COP_IA_VENDOR_ID,				"Vendor ID",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_COP_IA_PRODUCT_CODE,			"Product Code",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_COP_IA_MAJOR_REV,				"Major Revision",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_COP_IA_MINOR_REV,				"Minor Revision",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_COP_IA_SERIAL_NUMBER,			"Serial Number",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_COP_IA_MANF_DEV_NAME,			"Manufacturer Device Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_COP_IA_MANF_HW_VER,			"Manufacturer HW Version",				BaseType::Character,	NotifEvent::None },
	{ ABP_COP_IA_MANF_SW_VER,			"Manufacturer SW Version",				BaseType::Character,	NotifEvent::None },
	{ ABP_COP_IA_DEVICE_TYPE,			"Device Type",							BaseType::Character,	NotifEvent::None },
	{ ABP_COP_IA_DEF_PDO_MAP_CONF,		"Default PDO Mapping Configuration",	BaseType::Character,	NotifEvent::None },
	{ ABP_COP_IA_READ_PD_BUF_INIT_VAL,	"Read PD Buffer Init Value",			BaseType::Character,	NotifEvent::None }
};

static const AttrLookupTable_t asCpcObjAttrNames[] =
{
	{ ABP_CPC_OA_MAX_INST, "Maximum Number of Instances",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asCpcInstAttrNames[] =
{
	{ ABP_CPC_IA_PORT_TYPE,					"Port Type",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPC_IA_PORT_NUMBER,				"Port Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPC_IA_LINK_PATH,					"Link Path",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPC_IA_PORT_NAME,					"Port Name",					BaseType::Character,	NotifEvent::None },
	{ ABP_CPC_IA_NODE_ADDRESS,				"Node Address",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPC_IA_PORT_NODE_RANGE,			"Port Node Range",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPC_IA_PORT_ROUTING_CAPABILITIES,	"Port Routing Capabilities",	BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asCpnInstAttrNames[] =
{
	{ ABP_CPN_IA_VENDOR_ID,					"Vendor ID",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_DEVICE_TYPE,				"Device Type",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_PRODUCT_CODE,				"Product Code",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_REVISION,					"Revision",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_SERIAL_NUMBER,				"Serial Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_PRODUCT_NAME,				"Product Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_CPN_IA_PROD_INSTANCE,				"Producing Instance",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_CONS_INSTANCE,				"Consuming Instance",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_ENABLE_APP_CIP_OBJECTS,	"Enable Application CIP Objects",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_ENABLE_PARAM_OBJECT,		"Enable Parameter Object",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_CPN_IA_BIT_SLAVE,					"Bit Slave",						BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asDevInstAttrNames[] =
{
	{ ABP_DEV_IA_VENDOR_ID,					"Vendor ID",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_DEVICE_TYPE,				"Device Type",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_PRODUCT_CODE,				"Product Code",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_REVISION,					"Revision",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_SERIAL_NUMBER,				"Serial Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_PRODUCT_NAME,				"Product Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_DEV_IA_PROD_INSTANCE,				"Producing Instance",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_CONS_INSTANCE,				"Consuming Instance",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_ADDRESS_FROM_NET,			"Address From Network",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_BAUD_RATE_FROM_NET,		"Baud Rate From Network",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_ENABLE_APP_CIP_OBJECTS,	"Enable Application CIP Objects",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_ENABLE_PARAM_OBJECT,		"Enable Parameter Object",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_ENABLE_QUICK_CONNECT,		"Enable QuickConnect",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_PREPEND_PRODUCING,			"Prepend Producing",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_PREPEND_CONSUMING,			"Prepend Consuming",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_ABCC_ADI_OBJECT,			"ABCC ADI Object",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_PROD_INSTANCE_LIST,		"Producing Instance List",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_DEV_IA_CONS_INSTANCE_LIST,		"Consuming Instance List",			BaseType::Numeric,		NotifEvent::None },
};

static const AttrLookupTable_t asDiObjAttrNames[] =
{
	{ ABP_DI_OA_MAX_INST,		"Maximum Number of Instances",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_OA_SUPPORT_FUNC,	"Supported Functionality",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asDiInstAttrNames[] =
{
	{ ABP_DI_IA_SEVERITY,			"Severity",						BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_IA_EVENT_CODE,			"Event Code",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_IA_NW_SPEC_EVENT_INFO,	"Network Specific Event Info",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_IA_SLOT,				"Slot",							BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_IA_ADI,				"ADI",							BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_IA_ELEMENT,			"Element",						BaseType::Numeric,	NotifEvent::None },
	{ ABP_DI_IA_BIT,				"Bit",							BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asDpv1InstAttrNames[] =
{
	{ ABP_DPV1_IA_IDENT_NUMBER,			"Identity Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_PRM_DATA,				"Parameter Data",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_EXPECTED_CFG_DATA,	"Expected Configuration Data",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_SSA_ENABLED,			"SSA Enabled",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_SIZEOF_ID_REL_DIAG,	"Size of ID Related Diagnostic",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_BUFFER_MODE,			"Buffer Mode",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_ALARM_SETTINGS,		"Alarm Setting",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_MANUFACTURER_ID,		"Manufacturer ID",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_ORDER_ID,				"Order ID",							BaseType::Character,	NotifEvent::None },
	{ ABP_DPV1_IA_SERIAL_NO,			"Serial Number",					BaseType::Character,	NotifEvent::None },
	{ ABP_DPV1_IA_HW_REV,				"Hardware Revision",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_SW_REV,				"Software Revision",				BaseType::Character,	NotifEvent::None },
	{ ABP_DPV1_IA_REV_COUNTER,			"Revision Counter",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_PROFILE_ID,			"Profile ID",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_PROFILE_SPEC_TYPE,	"Profile Specific Type",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_IM_VERSION,			"I&M Version",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_IM_SUPPORTED,			"I&M Supported",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_IM_HEADER,			"I&M Header",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_CHK_CFG_BEHAVIOR,		"Check Configuration Behavior",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_DPV1_IA_RESERVED,				"Reserved",							BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asEcoObjAttrNames[] =
{
	{ ABP_ECO_OA_CURRENT_ENERGY_SAVING_MODE,		"Current Energy Saving Mode",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_OA_REMAINING_TIME_TO_DEST,			"Remaining Time to Destination",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_OA_ENERGY_CONSUMP_TO_DEST,			"Energy Consumption to Destination",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_OA_TRANSITION_TO_POWER_OFF_SUPPORTED,	"Transition To Power Off Supported",	BaseType::Numeric,	NotifEvent::None },
};

static const AttrLookupTable_t asEcoInstAttrNames[] =
{
	{ ABP_ECO_IA_MODE_ATTRIBUTES,			"Mode Attributes",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_TIME_MIN_PAUSE,			"Time Min Pause",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_TIME_TO_PAUSE,				"Time To Pause",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_TIME_TO_OPERATE,			"Time To Operate",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_TIME_MIN_LENGTH_OF_STAY,	"Time Min Length Of Stay",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_TIME_MAX_LENGTH_OF_STAY,	"Time Max Length Of Stay",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_MODE_POWER_CONSUMP,		"Mode Power Consumption",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_ENERGY_CONSUMP_TO_PAUSE,	"Energy Consumption To Pause",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_ENERGY_CONSUMP_TO_OPERATE,	"Energy Consumption To Operate",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_AVAILABILITY,				"Availability",						BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_IA_POWER_CONSUMPTION,			"Power Consumption",				BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asEipInstAttrNames[] =
{
	{ ABP_EIP_IA_VENDOR_ID,						"Vendor ID",										BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_DEVICE_TYPE,					"Device Type",										BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_PRODUCT_CODE,					"Product Code",										BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_REVISION,						"Revision",											BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_SERIAL_NUMBER,					"Serial Number",									BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_PRODUCT_NAME,					"Product Name",										BaseType::Character,	NotifEvent::None },
	{ ABP_EIP_IA_PROD_INSTANCE,					"Producing Instance Number",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_CONS_INSTANCE,					"Consuming Instance Number",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_COMM_SETTINGS_FROM_NET,		"Enable Communication Settings From Net",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS,		"Enable CIP Forwarding",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ENABLE_PARAM_OBJECT,			"Enable Parameter Object",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_INPUT_INSTANCE_OBJECT,			"Input-Only Heartbeat Instance Number",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_LISTEN_INSTANCE_OBJECT,		"Listen-Only Heartbeat Instance Number",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_CONFIG_INSTANCE,				"Assembly Object Configuration Instance Number",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_DISABLE_STRICT_IO_MATCH,		"Disable Strict I/O Match",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ENABLE_UNCONNECTED_SEND,		"Enable Unconnected Routing",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_INPUT_EXT_INSTANCE_OBJECT,		"Input-Only Extended Heartbeat Instance Number",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_LISTEN_EXT_INSTANCE_OBJECT,	"Listen-Only Extended Heartbeat Instance Number",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_IF_LABEL_PORT_1,				"Interface Label Port 1",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_IF_LABEL_PORT_2,				"Interface Label Port 2",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_IF_LABEL_PORT_INT,				"Interface Label Internal Port",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS_EXT,	"Enable Application CIP Object Extended",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_PREPEND_PRODUCING,				"Prepend Producing",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_PREPEND_CONSUMING,				"Prepend Consuming",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ENABLE_EIP_QC,					"Enable EtherNet/IP QuickConnect",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_PROD_INSTANCE_MAP,				"Producing Instance Mapping",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_CONS_INSTANCE_MAP,				"Consuming Instance Mapping",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_IGNORE_SEQ_COUNT_CHECK,		"Ignore Sequence Count Check",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ABCC_ADI_OBJECT,				"ABCC ADI Object Number",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ABCC_ENABLE_DLR,				"ABCC Enable DLR",									BaseType::Numeric,		NotifEvent::None },
	{ ABP_EIP_IA_ABCC_ENABLE_CIP_SYNC,			"ABCC Enable CIP Sync",								BaseType::Numeric,		NotifEvent::None },
};

static const AttrLookupTable_t asEmeInstAttrNames[] =
{
	{ ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL,			"Voltage Phase Neutral",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_MIN,		"Voltage Phase Neutral Min",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_NEUTRAL_MAX,		"Voltage Phase Neutral Max",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_PHASE,			"Voltage Phase Phase",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_PHASE_MIN,		"Voltage Phase Phase Min",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_PHASE_MAX,		"Voltage Phase Phase Max",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_GROUND,			"Voltage Phase Ground",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_GROUND_MIN,		"Voltage Phase Ground Min",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_VOLTAGE_PHASE_GROUND_MAX,		"Voltage Phase Ground Max",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_CURRENT,						"Current",						BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_CURRENT_MIN,					"Current Min",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_CURRENT_MAX,					"Current Max",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_APPARENT_POWER,				"Apparent Power",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_APPARENT_POWER_MIN,			"Apparent Power Min",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_APPARENT_POWER_MAX,			"Apparent Power Max",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_ACTIVE_POWER,					"Active Power",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_ACTIVE_POWER_MIN,				"Active Power Min",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_ACTIVE_POWER_MAX,				"Active Power Max",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_REACTIVE_POWER,				"Reactive Power",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_REACTIVE_POWER_MIN,			"Reactive Power Min",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_REACTIVE_POWER_MAX,			"Reactive Power Max",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_POWER_FACTOR,					"Power Factor",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_POWER_FACTOR_MIN,				"Power Factor Min",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_POWER_FACTOR_MAX,				"Power Factor Max",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_FREQUENCY,						"Frequency",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_FREQUENCY_MIN,					"Frequency Min",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_FREQUENCY_MAX,					"Frequency Max",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_FIELD_ROTATION,				"Field Rotation",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_TOTAL_ACTIVE_ENERGY,			"Total Active Energy",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_TOTAL_REACTIVE_ENERGY,			"Total Reactive Engery",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_EME_IA_TOTAL_APPARENT_ENERGY,			"Total Apparent Energy",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asEplInstAttrNames[] =
{
	{ ABP_EPL_IA_VENDOR_ID,			"Vendor ID",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_IA_PRODUCT_CODE,		"Product Code",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_IA_MAJOR_REV,			"Revision High Word",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_IA_MINOR_REV,			"Revision Low Word",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_IA_SERIAL_NUMBER,		"Serial Number",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_IA_MANF_DEV_NAME,		"Manufacturer Device Name",			BaseType::Character,	NotifEvent::None },
	{ ABP_EPL_IA_MANF_HW_VER,		"Manufacturer Hardware Version",	BaseType::Character,	NotifEvent::None },
	{ ABP_EPL_IA_MANF_SW_VER,		"Manufacturer Software Version",	BaseType::Character,	NotifEvent::None },
	{ ABP_EPL_IA_DEVICE_TYPE,		"Device Type",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_IA_MANF_NAME,			"Manufacturer Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_EPL_ENABLE_IT_FUNC,		"Enable IT Functionality",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_SDO_IT_FRAME_RATIO,	"SDO/IT Frame Ratio",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_EPL_APP_SW_DATE_AND_TIME,	"Application SW Date and Time",		BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asErInstAttrNames[] =
{
	{ ABP_ER_IA_ENERGY_READING,				"Energy Reading",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_ER_IA_DIRECTION,					"Direction",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_ER_IA_ACCURACY,					"Accuracy",						BaseType::Numeric,	NotifEvent::None },
	{ ABP_ER_IA_CURRENT_POWER_CONSUMPTION,	"Current Power Consumption",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ER_IA_NOMINAL_POWER_CONSUMPTION,	"Nominal Power Consumption",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asEtcInstAttrNames[] =
{
	{ ABP_ECT_IA_VENDOR_ID,				"Vendor ID",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_PRODUCT_CODE,			"Product Code",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_MAJOR_REV,				"Major Revision",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_MINOR_REV,				"Minor Revision",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_SERIAL_NUMBER,			"Serial Number",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_MANF_DEV_NAME,			"Manufacturer Device Name",					BaseType::Character,	NotifEvent::None },
	{ ABP_ECT_IA_MANF_HW_VER,			"Manufacturer Hardware Version",			BaseType::Character,	NotifEvent::None },
	{ ABP_ECT_IA_MANF_SW_VER,			"Manufacturer Software Version",			BaseType::Character,	NotifEvent::None },
	{ ABP_ECT_IA_ENUM_ADIS,				"ENUM ADIs",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_DEVICE_TYPE,			"Device Type",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_WR_PD_ASSY_INST_TRANS,	"Write PD Assembly Instance Translation",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_RD_PD_ASSY_INST_TRANS,	"Read PD Assembly Instance Translation",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_ADI_TRANS,				"ADI Translation",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_OBJ_SUB_TRANS,			"Object SubIndex Translation",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_ENABLE_FOE,			"Enable FoE",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_ENABLE_EOE,			"Enable EoE",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_CHANGE_SR_SWITCH,		"Change Shift Register Switch",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_SET_DEV_ID_AS_CSA,		"Set Device ID as Configured Station Alias",BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_ETHERCAT_STATE,		"EtherCAT State",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_STATE_TIMEOUTS,		"State Timeouts",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_COMP_IDENT_LISTS,		"Compare Identity Lists",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_FSOE_STATUS_IND,		"FSoE Status Indication",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_CLEAR_IDENT_AL_STS,	"Clear Identity AL_Status",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_SII_ORDER_NUM,			"SII Order Number",							BaseType::Character,	NotifEvent::None },
	{ ABP_ECT_IA_SII_DEV_NAME,			"SII Device Name",							BaseType::Character,	NotifEvent::None },
	{ ABP_ECT_IA_FOEDATA_ACK_DELAY,		"FoE Data ACK Delay",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_DEF_TXPDO_ASSIGN,		"Default TxPDO Assign",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_DEF_RXPDO_ASSIGN,		"Default RxPDO Assign",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_IA_SII_COE_DETAILS,		"SII CoE Details",							BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asEtnInstAttrNames[] =
{
	{ ABP_ETN_IA_MAC_ADDRESS,					"MAC Address",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_HICP,					"Enable HICP",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_WEB,					"Enable Web Server",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_MOD_TCP ,				"Enable Modbus TCP",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_WEB_ADI_ACCESS,			"Enable Web ADI Access",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_FTP,					"Enable FTP Server",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_ADMIN_MODE,				"Enable Admin Mode",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_NETWORK_STATUS,				"Network Status",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_PORT1_MAC_ADDRESS,				"Port 1 MAC Address",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_PORT2_MAC_ADDRESS,				"Port 2 MAC Address",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_ACD,					"Enable ACD",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_PORT1_STATE,					"Port 1 State",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_PORT2_STATE,					"Port 2 State",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_WEB_UPDATE,				"Enable Web Update",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_HICP_RESET,				"Enable Reset From HICP",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_IP_CONFIGURATION,				"IP Configuration",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_IP_ADDRESS_BYTE_0_2,			"IP Address Byte 0-2",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ETH_PHY_CONFIG,				"PHY Duplex Fallback Config",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_SNMP_READ_ONLY,				"SNMP Read-Only",					BaseType::Character,	NotifEvent::None },
	{ ABP_ETN_IA_SNMP_READ_WRITE,				"SNMP Read-Write",					BaseType::Character,	NotifEvent::None },
	{ ABP_ETN_IA_DHCP_OPTION_61_SOURCE,			"DHCP Option 61 Source",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_DHCP_OPTION_61_GENERIC_STR,	"DHCP Option 61 Generic String",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_DHCP_CLIENT,			"Enable DHCP Client",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_ETN_IA_ENABLE_WEBDAV,					"Enable WebDAV",					BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asFsiObjAttrNames[] =
{
	{ ABP_FSI_OA_MAX_INST,						"Max Number of Instances",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_FSI_OA_DISABLE_VFS,					"Disable Virtual File System",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_FSI_OA_TOTAL_DISC_SIZE,				"Total Disc Size",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_FSI_OA_FREE_DISC_SIZE,				"Free Disc Size",				BaseType::Numeric,	NotifEvent::None },
	{ 15,										"Disc CRC",						BaseType::Numeric,	NotifEvent::None },
	{ ABP_FSI_OA_DISC_TYPE,						"Disc Type",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_FSI_OA_DISC_FAULT_TOLERANCE_LEVEL,	"Disc Fault Tolerance Level",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asFsiInstAttrNames[] =
{
	{ ABP_FSI_IA_TYPE,		"Instance Type",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_IA_FILE_SIZE,	"File Size",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_IA_PATH,		"Current Instance Path",	BaseType::Character,	NotifEvent::None }
};

static const AttrLookupTable_t asFusmInstAttrNames[] =
{
	{ ABP_FUSM_IA_STATE,		"State",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_VENDOR_ID,	"Vendor ID",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_MODULE_ID,	"I/O Channel ID",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_FW_VERSION,	"Firmware Version",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_SERIAL_NUM,	"Serial Number",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_DATA_OUT,		"Output Data",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_DATA_IN,		"Input Data",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_ERROR_CNTRS,	"Error Counters",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_FATAL_EVENT,	"Event Log",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_EXCPT_INFO,	"Exception Information",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_BL_VERSION,	"Bootloader Version",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_FUSM_IA_VENDOR_BLK_1,	"Vendor Block Safe uC1",	BaseType::Character,	NotifEvent::None },
	{ ABP_FUSM_IA_VENDOR_BLK_2,	"Vendor Block Safe uC2",	BaseType::Character,	NotifEvent::None }
};

static const AttrLookupTable_t asMddObjAttrNames[] =
{
	{ ABP_MDD_OA_NUM_SLOTS,			"Number of Slots",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_MDD_OA_NUM_ADIS_PER_SLOT,	"Number of ADIs Per Slot",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asModInstAttrNames[] =
{
	{ ABP_MOD_IA_VENDOR_NAME,			"Vendor Name",									BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_PRODUCT_CODE,			"Product Code",									BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_REVISION,				"Major Minor Revision",							BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_VENDOR_URL,			"Vendor URL",									BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_PRODUCT_NAME,			"Product Name",									BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_MODEL_NAME,			"Model Name",									BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_USER_APP_NAME,			"User Application Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_MOD_IA_DEVICE_ID,				"Device ID",									BaseType::Numeric,		NotifEvent::None },
	{ ABP_MOD_IA_ADI_INDEXING_BITS,		"No. of ADI indexing bits",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_MOD_IA_MESSAGE_FORWARDING,	"Enable Modbus message forwarding",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_MOD_IA_RW_OFFSET,				"Modbus read/write registers command offset",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_MOD_IA_DISABLE_DEVICE_ID_FC,	"Disable Device ID Function Code",				BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asNcInstAttrNames[] =
{
	{ ABP_NC_VAR_IA_NAME,			"Name",					BaseType::Character,	NotifEvent::None },
	{ ABP_NC_VAR_IA_DATA_TYPE,		"Data Type",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_NC_VAR_IA_NUM_ELEM,		"Number of Elements",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_NC_VAR_IA_DESCRIPTOR,		"Descriptor",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_NC_VAR_IA_VALUE,			"Value",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_NC_VAR_IA_CONFIG_VALUE,	"Configured Value",		BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asNwInstAttrNames[] =
{
	{ ABP_NW_IA_NW_TYPE,		"Network Type",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_NW_IA_NW_TYPE_STR,	"Network Type String",		BaseType::Character,	NotifEvent::None },
	{ ABP_NW_IA_DATA_FORMAT,	"Data Format",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_NW_IA_PARAM_SUPPORT,	"Parameter Support",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_NW_IA_WRITE_PD_SIZE,	"Write Process Data Size",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_NW_IA_READ_PD_SIZE,	"Read Process Data Size",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_NW_IA_EXCEPTION_INFO,	"Exception Information",	BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asNwCclInstAttrNames[] =
{
	{ ABP_NWCCL_IA_NETWORK_SETTINGS,	"Network Settings",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_IA_SYSTEM_AREA_HANDLER,	"System Area Handler",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_IA_ERROR_CODE_POSITION,	"Error Code Position",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_IA_LAST_MAPPING_INFO,	"Last Mapping Info",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_IA_CCL_CONF_TEST_MODE,	"CCL Conformance Test Mode",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_IA_ERROR_INFO,			"Error Information",			BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asNwCfnInstAttrNames[] =
{
	{ ABP_NWCFN_IA_IO_DATA_SIZES,	"IO Data Sizes",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCFN_IA_APP_OP_STATUS,	"Application OP Status",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCFN_IA_SLMP_REC_LOCK,	"SLMP Reception Lock",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asNwDpv1InstAttrNames[] =
{
   { ABP_DPV1_IA_IDENT_NUMBER,			"PNO Identity Number",		BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_PRM_DATA,				"Parameterization Data",	BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_EXPECTED_CFG_DATA,		"Expected Configuration",	BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_SSA_ENABLED,			"SSA Enabled",				BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_SIZEOF_ID_REL_DIAG,	"Reserved",					BaseType::Numeric,		NotifEvent::Alert },
   { ABP_DPV1_IA_BUFFER_MODE,			"Reserved",					BaseType::Numeric,		NotifEvent::Alert },
   { ABP_DPV1_IA_ALARM_SETTINGS,		"Reserved",					BaseType::Numeric,		NotifEvent::Alert },
   { ABP_DPV1_IA_MANUFACTURER_ID,		"Manufacturer ID",			BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_ORDER_ID,				"Order ID",					BaseType::Character,	NotifEvent::None },
   { ABP_DPV1_IA_SERIAL_NO,				"Serial Number ID",			BaseType::Character,	NotifEvent::None },
   { ABP_DPV1_IA_HW_REV,				"Hardware Revision",		BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_SW_REV,				"Software Revision",		BaseType::Character,	NotifEvent::None }, /* The value here is more 'numeric' in nature, but the first byte is a character. */
   { ABP_DPV1_IA_REV_COUNTER,			"Revision Counter",			BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_PROFILE_ID,			"Profile ID",				BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_PROFILE_SPEC_TYPE,		"Profile Specific Type",	BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_IM_VERSION,			"Reserved",					BaseType::Numeric,		NotifEvent::Alert },
   { ABP_DPV1_IA_IM_SUPPORTED,			"Reserved",					BaseType::Numeric,		NotifEvent::Alert },
   { ABP_DPV1_IA_IM_HEADER,				"IM Header",				BaseType::Numeric,		NotifEvent::None },
   { ABP_DPV1_IA_CHK_CFG_BEHAVIOR,		"Reserved",					BaseType::Numeric,		NotifEvent::Alert },
   { ABP_DPV1_IA_RESERVED,				"Reserved",					BaseType::Numeric,		NotifEvent::Alert }
};

static const AttrLookupTable_t asNwEtnInstAttrNames[] =
{
	{ ABP_NWETN_IA_MAC_ID,				"MAC Address",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWETN_IA_PORT1_MAC_ID,		"Port 1 MAC Address",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWETN_IA_PORT2_MAC_ID,		"Port 2 MAC Address",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWETN_IA_MAC_ADDRESS,			"MAC Address",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWETN_IA_INTERFACE_COUNTERS,	"Interface Counters",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWETN_IA_MEDIA_COUNTERS,		"Media Counters",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asNwPnioInstAttrNames[] =
{
	{ ABP_NWPNIO_IA_ONLINE_TRANS,			"Number of on-line transitions",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_OFFLINE_TRANS,			"Number of off-line transitions",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_OFFLINE_REASON_CODE,	"Reason code of last off-line",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_ABORT_REASON_CODE,		"Last abort reason code",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_ADDED_APIS,				"Number of added APIs",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_API_LIST,				"List of the added APIs",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_EST_ARS,				"Number of established ARs",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_AR_LIST,				"List of established ARs (handles)",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_PNIO_INIT_ERR_CODE,		"Error code PROFINET IO stack init",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_PORT1_MAC_ADDRESS,		"PROFINET IO port 1 MAC address",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_IA_PORT2_MAC_ADDRESS,		"PROFINET IO port 2 MAC address",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asMqttInstAttrNames[] =
{
	{ ABP_MQTT_IA_MODE,			"Mode",				BaseType::Numeric,	NotifEvent::None },
	{ APB_MQTT_IA_LAST_WILL,	"Last Will",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asOpcuaInstAttrNames[] =
{
	{ ABP_OPCUA_IA_MODEL,					"Model",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_OPCUA_IA_APPLICATION_URI,			"Application URI",		BaseType::Character,	NotifEvent::None },
	{ ABP_OPCUA_IA_VENDOR_NAMESPACE_URI,	"Vendor Namespace URI",	BaseType::Character,	NotifEvent::None },
	{ ABP_OPCUA_IA_DEVICE_TYPE_NAME,		"Device Type Name",		BaseType::Character,	NotifEvent::None },
	{ ABP_OPCUA_IA_DEVICE_INST_NAME,		"Device Instance Name",	BaseType::Character,	NotifEvent::None },
	{ ABP_OPCUA_IA_PRODUCT_URI,				"Product URI",			BaseType::Character,	NotifEvent::None },
	{ ABP_OPCUA_IA_LIMITS,					"Limits",				BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asPnamInstAttrNames[] =
{
	{ ABP_PNAM_IA_INFO_TYPE,		"Info Type",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_UNIQUE_ID,		"Unique ID",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_LOCATION_TYPE,	"Location Type",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_LOCATION_LT,		"Location Level Tree",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_LOCATION_SS,		"Location Slot Subslot",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_ANNOTATION,		"Annotation",							BaseType::Character,	NotifEvent::None },
	{ ABP_PNAM_IA_ORDER_ID,			"Order ID",								BaseType::Character,	NotifEvent::None },
	{ ABP_PNAM_IA_SERIAL,			"Serial",								BaseType::Character,	NotifEvent::None },
	{ ABP_PNAM_IA_DEVICE_ID,		"Device ID",							BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_TYPE_ID,			"Type ID",								BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNAM_IA_AM_SW_REV,		"Asset Management Software Revision",	BaseType::Character,	NotifEvent::None },
	{ ABP_PNAM_IA_IM_SW_REV,		"I&M Software Revision",				BaseType::Character,	NotifEvent::None },
	{ ABP_PNAM_IA_AM_HW_REV,		"Asset Management Hardware Revision",	BaseType::Character,	NotifEvent::None },
	{ ABP_PNAM_IA_IM_HW_REV,		"I&M Hardware Revision",				BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asPnioInstAttrNames[] =
{
	{ ABP_PNIO_IA_DEVICE_ID,				"Device ID",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_VENDOR_ID,				"Vendor ID (I&M Manufacturer ID)",	BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_STATION_TYPE,				"Station Type",						BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_MAX_AR,					"MaxAr",							BaseType::Numeric,		NotifEvent::None },
	{ 0x05,									"Reserved",							BaseType::Numeric,		NotifEvent::Alert },
	{ 0x06,									"Reserved",							BaseType::Numeric,		NotifEvent::Alert },
	{ ABP_PNIO_IA_RTM,						"Record Data Mode",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_ORDER_ID,				"I&M Order ID",						BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_IM_SERIAL_NBR,			"I&M Serial Number",				BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_IM_HW_REV,				"I&M Hardware Revision",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_SW_REV,				"I&M Software Revision",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_REV_CNT,				"I&M Revision Counter",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_PROFILE_ID,			"I&M Profile ID",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_PROFILE_SPEC_TYPE,		"I&M Profile Specific Type",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_VER,					"I&M Version",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_IM_SUPPORTED,				"I&M Supported",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_PORT1_MAC_ADDRESS,		"Port 1 MAC Address",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_PORT2_MAC_ADDRESS,		"Port 2 MAC Address",				BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_SYSTEM_DESCRIPTION,		"System Description",				BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_INTERFACE_DESCRIPTION,	"Interface Description",			BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_MOD_ID_ASSIGN_MODE,		"Module Id Assignment Mode",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_SYSTEM_CONTACT,			"System Contact",					BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_PROFIENERGY_FUNC,			"PROFIenergy Functionality",		BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_CUSTOM_STATION_NAME,		"Custom Station Name",				BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_IM_MODULE_ORDER_ID,		"I&M Module Order ID",				BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_IM_ANNOTATION,			"I&M Annotation",					BaseType::Character,	NotifEvent::None },
	{ ABP_PNIO_IA_IM5_ENABLED,				"I&M5 Enabled",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_S2_ENABLED,				"S2 Enabled",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_PNIO_IA_S2_PRIMARY_AR_HANDLE,		"S2 Primary AR Handle",				BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asSafeInstAttrNames[] =
{
	{ ABP_SAFE_IA_SAFETY_ENABLED,			"Safety Enabled",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SAFE_IA_BAUD_RATE,				"Baud Rate",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_SAFE_IA_IO_CONFIG,				"I/O Configuration",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_SAFE_IA_CYCLE_TIME,				"Cycle Time",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_SAFE_IA_FW_UPGRADE_IN_PROGRESS,	"FW Upgrade In Progress",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asSmtpObjAttrNames[] =
{
	{ ABP_SMTP_OA_MAX_INST,		"Maximum Number of Instances",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SMTP_OA_EMAILS_SENT,	"Emails Sent",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_SMTP_OA_EMAIL_FAILED,	"Emails Failed to Send",		BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asSmtpInstAttrNames[] =
{
	{ ABP_SMTP_IA_FROM,		"From Address",		BaseType::Character,	NotifEvent::None },
	{ ABP_SMTP_IA_TO,		"To Address",		BaseType::Character,	NotifEvent::None },
	{ ABP_SMTP_IA_SUBJECT,	"Message Subject",	BaseType::Character,	NotifEvent::None },
	{ ABP_SMTP_IA_MESSAGE,	"Message Body",		BaseType::Character,	NotifEvent::None }
};

static const AttrLookupTable_t asSocObjAttrNames[] =
{
	{ ABP_SOC_OA_MAX_INST, "Maximum Number of Instances",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asSocInstAttrNames[] =
{
	{ ABP_SOC_IA_SOCK_TYPE,			"Socket Type",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_LOCAL_PORT,		"Local Port",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_HOST_IP,			"Host IP Address",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_HOST_PORT,			"Host Port",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_TCP_STATE,			"TCP State",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_RX_BYTES,			"Bytes in RX Buffer",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_TX_BYTES,			"Bytes in TX Buffer",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_SO_REUSE_ADDR,		"Reuse Address Option",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_SO_KEEP_ALIVE,		"Keep Alive Option",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_IP_MULT_TTL,		"IP Multicast TTL",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_IP_MULT_LOOP,		"IP Multicast Loop",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_TCP_ACKDELAYTIME,	"TCP Ack Delay Time",	BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_TCP_NODELAY,		"TCP No Delay",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SOC_IA_TCP_CONNTIMEO,		"TCP Connect Timeout",	BaseType::Numeric,	NotifEvent::None }
};

static const AttrLookupTable_t asSrc3InstAttrNames[] =
{
	{ ABP_SRC3_IA_COMPONENT_NAME,		"Component Name",					BaseType::Character,	NotifEvent::None },
	{ ABP_SRC3_IA_VENDOR_CODE,			"Vendor Code",						BaseType::Numeric,		NotifEvent::None },
	{ ABP_SRC3_IA_DEVICE_NAME,			"Device Name",						BaseType::Character,	NotifEvent::None },
	{ ABP_SRC3_IA_VENDOR_DEVICE_ID,		"Vendor Device ID",					BaseType::Character,	NotifEvent::None },
	{ ABP_SRC3_IA_SOFTWARE_REVISION,	"Software Revision",				BaseType::Character,	NotifEvent::None },
	{ ABP_SRC3_IA_SERIAL_NUMBER,		"Serial Number",					BaseType::Character,	NotifEvent::None },
	{ ABP_SRC3_IA_MAJOR_EVT_LATCHING,	"Major Diagnostic Event Latching",	BaseType::Numeric,		NotifEvent::None }
};

static const AttrLookupTable_t asSyncInstAttrNames[] =
{
	{ ABP_SYNC_IA_CYCLE_TIME,			"Cycle Time",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_OUTPUT_VALID,			"Output Valid",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_INPUT_CAPTURE,		"Input Capture",				BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_OUTPUT_PROCESSING,	"Output Processing Time",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_INPUT_PROCESSING,		"Input Processing Time",		BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_MIN_CYCLE_TIME,		"Minimum Cycle Time",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_SYNC_MODE,			"Sync Mode",					BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_SUPPORTED_SYNC_MODES,	"Supported Sync Modes",			BaseType::Numeric,	NotifEvent::None },
	{ ABP_SYNC_IA_CONTROL_CYCLE_FACTOR,	"Control task cycle factor",	BaseType::Numeric,	NotifEvent::None },
};


// Entries contained in this table shall be sorted by object number.
// { Object, Object names pointer, Num object names, Instance names pointer, Num instance names } */
AttributeNameTable_t asAttributeNameTables[] = {
	{ ABP_OBJ_NUM_ANB,		0,									NUM_ENTRIES(asAnbInstAttrNames),	nullptr,			asAnbInstAttrNames },
	{ ABP_OBJ_NUM_DI,		NUM_ENTRIES(asDiObjAttrNames),		NUM_ENTRIES(asDiInstAttrNames),		asDiObjAttrNames,	asDiInstAttrNames },
	{ ABP_OBJ_NUM_NW,		0,									NUM_ENTRIES(asNwInstAttrNames),		nullptr,			asNwInstAttrNames },
	{ ABP_OBJ_NUM_NC,		0,									NUM_ENTRIES(asNcInstAttrNames),		nullptr,			asNcInstAttrNames },
	{ ABP_OBJ_NUM_ADD,		NUM_ENTRIES(asAddObjAttrNames),		NUM_ENTRIES(asAddInstAttrNames),	asAddObjAttrNames,	asAddInstAttrNames },
	{ ABP_OBJ_NUM_SOC,		NUM_ENTRIES(asSocObjAttrNames),		NUM_ENTRIES(asSocInstAttrNames),	asSocObjAttrNames,	asSocInstAttrNames },
	{ ABP_OBJ_NUM_NWCCL,	0,									NUM_ENTRIES(asNwCclInstAttrNames),	nullptr,			asNwCclInstAttrNames },
	{ ABP_OBJ_NUM_SMTP,		NUM_ENTRIES(asSmtpObjAttrNames),	NUM_ENTRIES(asSmtpInstAttrNames),	asSmtpObjAttrNames,	asSmtpInstAttrNames },
	{ ABP_OBJ_NUM_FSI,		NUM_ENTRIES(asFsiObjAttrNames),		NUM_ENTRIES(asFsiInstAttrNames),	asFsiObjAttrNames,	asFsiInstAttrNames },
	{ ABP_OBJ_NUM_NWDPV1,	0,									NUM_ENTRIES(asNwDpv1InstAttrNames),	nullptr,			asNwDpv1InstAttrNames },
	{ ABP_OBJ_NUM_NWETN,	0,									NUM_ENTRIES(asNwEtnInstAttrNames),	nullptr,			asNwEtnInstAttrNames },
	{ ABP_OBJ_NUM_CPC,		NUM_ENTRIES(asCpcObjAttrNames),		NUM_ENTRIES(asCpcInstAttrNames),	asCpcObjAttrNames,	asCpcInstAttrNames },
	{ ABP_OBJ_NUM_NWPNIO,	0,									NUM_ENTRIES(asNwPnioInstAttrNames),	nullptr,			asNwPnioInstAttrNames },
	{ ABP_OBJ_NUM_FUSM,		0,									NUM_ENTRIES(asFusmInstAttrNames),	nullptr,			asFusmInstAttrNames },
	{ ABP_OBJ_NUM_NWCFN,	0,									NUM_ENTRIES(asNwCfnInstAttrNames),	nullptr,			asNwCfnInstAttrNames },

	{ ABP_OBJ_NUM_MQTT,		0,									NUM_ENTRIES(asMqttInstAttrNames),	nullptr,			asMqttInstAttrNames },
	{ ABP_OBJ_NUM_OPCUA,	0,									NUM_ENTRIES(asOpcuaInstAttrNames),	nullptr,			asOpcuaInstAttrNames },
	{ ABP_OBJ_NUM_EME,		0,									NUM_ENTRIES(asEmeInstAttrNames),	nullptr,			asEmeInstAttrNames },
	{ ABP_OBJ_NUM_PNAM,		0,									NUM_ENTRIES(asPnamInstAttrNames),	nullptr,			asPnamInstAttrNames },
	{ ABP_OBJ_NUM_CFN,		0,									NUM_ENTRIES(asCfnInstAttrNames),	nullptr,			asCfnInstAttrNames },
	{ ABP_OBJ_NUM_ER,		0,									NUM_ENTRIES(asErInstAttrNames),		nullptr,			asErInstAttrNames },
	{ ABP_OBJ_NUM_SAFE,		0,									NUM_ENTRIES(asSafeInstAttrNames),	nullptr,			asSafeInstAttrNames },
	{ ABP_OBJ_NUM_EPL,		0,									NUM_ENTRIES(asEplInstAttrNames),	nullptr,			asEplInstAttrNames },
	{ ABP_OBJ_NUM_AFSI,		NUM_ENTRIES(asFsiObjAttrNames),		NUM_ENTRIES(asFsiInstAttrNames),	asFsiObjAttrNames,	asFsiInstAttrNames }, // AFSI has the same functionality has FSI.
	{ ABP_OBJ_NUM_ASM,		NUM_ENTRIES(asAsmObjAttrNames),		NUM_ENTRIES(asAsmInstAttrNames),	asAsmObjAttrNames,	asAsmInstAttrNames },
	{ ABP_OBJ_NUM_MDD,		NUM_ENTRIES(asMddObjAttrNames),		0,									asMddObjAttrNames,	nullptr },
	{ ABP_OBJ_NUM_CIPID,	0,									NUM_ENTRIES(asCipIdInstAttrNames),	nullptr,			asCipIdInstAttrNames },
	{ ABP_OBJ_NUM_SYNC,		0,									NUM_ENTRIES(asSyncInstAttrNames),	nullptr,			asSyncInstAttrNames },
	{ ABP_OBJ_NUM_BAC,		0,									NUM_ENTRIES(asBacInstAttrNames),	nullptr,			asBacInstAttrNames },
	{ ABP_OBJ_NUM_ECO,		NUM_ENTRIES(asEcoObjAttrNames),		NUM_ENTRIES(asEcoInstAttrNames),	asEcoObjAttrNames,	asEcoInstAttrNames },
	{ ABP_OBJ_NUM_SRC3,		0,									NUM_ENTRIES(asSrc3InstAttrNames),	nullptr,			asSrc3InstAttrNames },
	{ ABP_OBJ_NUM_CNT,		0,									NUM_ENTRIES(asCntInstAttrNames),	nullptr,			asCntInstAttrNames },
	{ ABP_OBJ_NUM_CPN,		0,									NUM_ENTRIES(asCpnInstAttrNames),	nullptr,			asCpnInstAttrNames },
	{ ABP_OBJ_NUM_ECT,		0,									NUM_ENTRIES(asEtcInstAttrNames),	nullptr,			asEtcInstAttrNames },
	{ ABP_OBJ_NUM_PNIO,		0,									NUM_ENTRIES(asPnioInstAttrNames),	nullptr,			asPnioInstAttrNames },
	{ ABP_OBJ_NUM_CCL,		0,									NUM_ENTRIES(asCclInstAttrNames),	nullptr,			asCclInstAttrNames },
	{ ABP_OBJ_NUM_EIP,		0,									NUM_ENTRIES(asEipInstAttrNames),	nullptr,			asEipInstAttrNames },
	{ ABP_OBJ_NUM_ETN,		0,									NUM_ENTRIES(asEtnInstAttrNames),	nullptr,			asEtnInstAttrNames },
	{ ABP_OBJ_NUM_MOD,		0,									NUM_ENTRIES(asModInstAttrNames),	nullptr,			asModInstAttrNames },
	{ ABP_OBJ_NUM_COP,		0,									NUM_ENTRIES(asCopInstAttrNames),	nullptr,			asCopInstAttrNames },
	{ ABP_OBJ_NUM_DEV,		0,									NUM_ENTRIES(asDevInstAttrNames),	nullptr,			asDevInstAttrNames },
	{ ABP_OBJ_NUM_DPV1,		0,									NUM_ENTRIES(asDpv1InstAttrNames),	nullptr,			asDpv1InstAttrNames },
	{ ABP_OBJ_NUM_APPD,		NUM_ENTRIES(asAppdObjAttrNames),	NUM_ENTRIES(asAppdInstAttrNames),	asAppdObjAttrNames,	asAppdInstAttrNames },
	{ ABP_OBJ_NUM_APP,		0,									NUM_ENTRIES(asAppInstAttrNames),	nullptr,			asAppInstAttrNames },
};

/*******************************************************************************
**
** Common command lookup table
**
*******************************************************************************/

static const CmdLookupTable_t asCmdNames[] =
{
	{ ABP_CMD_GET_ATTR,			"Get_Attribute",			BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None }, // NOTE: BaseType values here are not used by plugin
	{ ABP_CMD_SET_ATTR,			"Set_Attribute",			BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None }, // NOTE: BaseType values here are not used by plugin
	{ ABP_CMD_CREATE,			"Create",					BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_CMD_DELETE,			"Delete",					BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_CMD_RESET,			"Reset",					BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_CMD_GET_ENUM_STR,		"Get_Enum_String",			BaseType::Numeric,	BaseType::Character,	NotifEvent::None },
	{ ABP_CMD_GET_INDEXED_ATTR,	"Get_Indexed_Attribute",	BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None }, // NOTE: BaseType values here are not used by plugin
	{ ABP_CMD_SET_INDEXED_ATTR,	"Set_Indexed_Attribute",	BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None }  // NOTE: BaseType values here are not used by plugin
};

/*******************************************************************************
**
** Object-specific command lookup tables
**
*******************************************************************************/

static const CmdLookupTable_t asAddCmdNames[] =
{
	{ ABP_ADD_CMD_ALARM_NOTIFICATION,	"Alarm_Notification",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::Alert }
};

static const CmdLookupTable_t asAppCmdNames[] =
{
	{ ABP_APP_CMD_RESET_REQUEST,		"Reset_Request",			BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_CMD_CHANGE_LANG_REQUEST,	"Change_Language_Request",	BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_CMD_RESET_DIAGNOSTIC,		"Reset_Diagnostic",			BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_APP_CMD_GET_DATA_NOTIF,		"Get_Data_Notification",	BaseType::Numeric,	BaseType::Character,	NotifEvent::None }
};

static const CmdLookupTable_t asAppdCmdNames[] =
{
	{ ABP_APPD_CMD_GET_INST_BY_ORDER,		"Get_Instance_Number_By_Order",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_APPD_GET_PROFILE_INST_NUMBERS,	"Get_Profile_Inst_Numbers",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_APPD_GET_ADI_INFO,				"Get_ADI_Info (Deprecated)",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::Alert }, // ABCC40 deprecated shall not be used
	{ ABP_APPD_REMAP_ADI_WRITE_AREA,		"Remap_ADI_Write_Area",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_APPD_REMAP_ADI_READ_AREA,			"Remap_Adi_Read_Area",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_APPD_GET_INSTANCE_NUMBERS,		"Get_Instance_Numbers",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asAsmCmdNames[] =
{
	{ ABP_ASM_CMD_WRITE_ASSEMBLY_DATA,	"Write_Assembly_Data",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ASM_CMD_READ_ASSEMBLY_DATA,	"Read_Assembly_Data",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asBacCmdNames[] =
{
	{ ABP_BAC_CMD_GET_ADI_BY_BACNET_OBJ_INST,		"Get_ADI_By_BacNet_Obj_Inst",		BaseType::Numeric,		BaseType::Numeric,	NotifEvent::None },
	{ ABP_BAC_CMD_GET_ADI_BY_BACNET_OBJ_INST_NAME,	"Get_ADI_By_BacNet_Obj_Inst_Name",	BaseType::Character,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_BAC_CMD_GET_ALL_BACNET_OBJ_INSTANCES,		"Get_All_BacNet_Obj_Instances",		BaseType::Numeric,		BaseType::Numeric,	NotifEvent::None },
	{ ABP_BAC_CMD_GET_BACNET_OBJ_INST_BY_ADI,		"Get_BacNet_Obj_Inst_By_ADI",		BaseType::Numeric,		BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asCclCmdNames[] =
{
	{ ABP_CCL_CMD_INITIAL_DATA_SETTING_NOTIFICATION,				"Initial_Data_Setting_Notification",				BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_CCL_CMD_INITIAL_DATA_PROCESSING_COMPLETED_NOTIFICATION,	"Initial_Data_Processing_Completed_Notification",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asCfnCmdNames[] =
{
	{ ABP_CFN_CMD_BUF_SIZE_NOTIF,	"Buf_Size_Notif",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_CFN_CMD_SLMP_SERVER_REQ,	"SLMP_Server_Req",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asCntCmdNames[] =
{
	{ ABP_CNT_CMD_PROCESS_CIP_OBJ_REQUEST,	"Process_CIP_Obj_Request",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_CNT_CMD_SET_CONFIG_DATA,			"Set_Config_Data",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_CNT_CMD_GET_CONFIG_DATA,			"Get_Config_Data",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
};

static const CmdLookupTable_t asCpnCmdNames[] =
{
	{ ABP_CPN_CMD_PROCESS_CIP_OBJ_REQUEST,	"Process_CIP_Obj_Request",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asDevCmdNames[] =
{
	{ ABP_DEV_CMD_PROCESS_CIP_OBJ_REQUEST,	"Process_CIP_Obj_Request",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asDpv1CmdNames[] =
{
	{ ABP_DPV1_CMD_GET_IM_RECORD,	"Get_IM_Record",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_DPV1_CMD_SET_IM_RECORD,	"Set_IM_Record",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_DPV1_CMD_ALARM_ACK,		"Alarm_Ack",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_DPV1_CMD_GET_RECORD,		"Get_Record",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_DPV1_CMD_SET_RECORD,		"Set_Record",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asEcoCmdNames[] =
{
	{ ABP_ECO_CMD_START_PAUSE,			"Start_Pause",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_CMD_END_PAUSE,			"End_Pause",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_ECO_CMD_PREVIEW_PAUSE_TIME,	"Preview_Pause_Time",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asEctCmdNames[] =
{
	{ ABP_ECT_CMD_GET_OBJECT_DESC,		"Get_Object_Description",	BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_CMD_GET_OBJECT_ACCESS,	"Get_Object_Access",		BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_CMD_GET_DATA_TYPE,		"Get_Data_Type",			BaseType::Numeric,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_ECT_CMD_GET_ENUM_DATA,		"Get_Enum_Data",			BaseType::Numeric,	BaseType::Character,	NotifEvent::None }
};

static const CmdLookupTable_t asEipCmdNames[] =
{
	{ ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST,		"Process_CIP_Obj_Request",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_EIP_CMD_SET_CONFIG_DATA,				"Set_Config_Data",				BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_EIP_CMD_PROCESS_CIP_ROUTING_REQUEST,	"Process_CIP_Routing_Request",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_EIP_CMD_GET_CONFIG_DATA,				"Get_Config_Data",				BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST_EXT,	"Process_CIP_Obj_Request_Ext",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asEmeCmdNames[] =
{
	{ ABP_EME_CMD_GET_ATTRIBUTE_MEASUREMENT_LIST,	"Get_Attribute_Measurement_List",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asFsiCmdNames[] =
{
	{ ABP_FSI_CMD_FILE_OPEN,			"File_Open",				BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_FILE_CLOSE,			"File_Close",				BaseType::Numeric,		BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_FILE_DELETE,			"File_Delete",				BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_FILE_COPY,			"File_Copy",				BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_FILE_RENAME,			"File_Rename",				BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_FILE_READ,			"File_Read",				BaseType::Numeric,		BaseType::Character,	NotifEvent::None },
	{ ABP_FSI_CMD_FILE_WRITE,			"File_Write",				BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_DIRECTORY_OPEN,		"Directory_Open",			BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_DIRECTORY_CLOSE,		"Directory_Close",			BaseType::Numeric,		BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_DIRECTORY_DELETE,		"Directory_Delete",			BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_DIRECTORY_READ,		"Directory_Read",			BaseType::Numeric,		BaseType::Character,	NotifEvent::None },
	{ ABP_FSI_CMD_DIRECTORY_CREATE,		"Directory_Create",			BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_DIRECTORY_CHANGE,		"Directory_Change",			BaseType::Character,	BaseType::Numeric,		NotifEvent::None },
	{ ABP_FSI_CMD_FORMAT_DISC,			"Format_Disc (deprecated)",	BaseType::Numeric,		BaseType::Numeric,		NotifEvent::Alert }
};

static const CmdLookupTable_t asFusmCmdNames[] =
{
	{ ABP_FUSM_CMD_ERROR_CONFIRMATION,		"Error_Confirmation",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::Alert },
	{ ABP_FUSM_CMD_SET_IO_CFG_STRING,		"Set_IO_Cfg_String",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_FUSM_CMD_GET_SAFETY_OUTPUT_PDU,	"Get_Safety_Output_PDU",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_FUSM_CMD_GET_SAFETY_INPUT_PDU,	"Get_Safety_Input_PDU",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asMddCmdNames[] =
{
	{ ABP_MDD_CMD_GET_LIST, "Get_List",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asModCmdNames[] =
{
	{ ABP_MOD_CMD_PROCESS_MODBUS_MESSAGE,	"Process_Modbus_Message",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asMqttCmdNames[] =
{
	{ ABP_MQTT_CMD_GET_PUBLISH_CONFIGURATION,	"Get_Publish_Configuration",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asNwCmdNames[] =
{
	{ ABP_NW_CMD_MAP_ADI_WRITE_AREA,		"Map_ADI_Write_Area",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NW_CMD_MAP_ADI_READ_AREA,			"Map_ADI_Read_Area",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NW_CMD_MAP_ADI_WRITE_EXT_AREA,	"Map_ADI_Write_Ext_Area",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NW_CMD_MAP_ADI_READ_EXT_AREA,		"Map_ADI_Read_Ext_Area",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asNwCclCmdNames[] =
{
	{ ABP_NWCCL_CMD_MAP_ADI_SPEC_WRITE_AREA,	"Map_ADI_Spec_Write_Area",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_CMD_MAP_ADI_SPEC_READ_AREA,		"Map_ADI_Spec_Read_Area",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWCCL_CMD_CCL_CONF_TEST_MODE,			"CCL_Conf_Test_Mode",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asNwCfnCmdNames[] =
{
	{ ABP_NWCFN_CMD_EXT_LOOPBACK,	"Ext_Loopback",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asNwDpv1CmdNames[] =
{
	{ ABP_NWDPV1_CMD_MAP_ADI_WRITE_AREA,	"Map_ADI_Write_Area",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWDPV1_CMD_MAP_ADI_READ_AREA,		"Map_ADI_Read_Area",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asNwPnioCmdNames[] =
{
	{ ABP_NWPNIO_CMD_PLUG_MODULE,			"Plug_Module",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_PLUG_SUB_MODULE,		"Plug_Submodule",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_PULL_MODULE,			"Pull_Module",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_PULL_SUB_MODULE,		"Pull_Submodule",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_API_ADD,				"API_Add",				BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_APPL_STATE_READY,		"Appl_State_Ready",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_AR_ABORT,				"AR_Abort",				BaseType::Numeric,	BaseType::Numeric,	NotifEvent::Alert },
	{ ABP_NWPNIO_CMD_ADD_SAFETY_MODULE,		"Add_Safety_Module",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_IM_OPTIONS,			"IM_Options",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_PLUG_SUB_MODULE_EXT,	"Plug_Submodule_Ext",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_NWPNIO_CMD_IDENT_CHANGE_DONE,		"Ident_Change_Done",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asPnioCmdNames[] =
{
	{ ABP_PNIO_CMD_GET_RECORD,			"Get_Record",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_SET_RECORD,			"Set_Record",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_GET_IM_RECORD,		"Get_IM_Record",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_SET_IM_RECORD,		"Set_IM_Record",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_AR_CHECK_IND,		"AR_Check_Ind",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_CFG_MISMATCH_IND,	"Cfg_Mismatch_Ind",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_AR_INFO_IND,			"AR_Info_Ind",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_END_OF_PRM_IND,		"End_Of_Prm_Ind",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_AR_ABORT_IND,		"AR_Abort_Ind",			BaseType::Numeric,	BaseType::Numeric,	NotifEvent::Alert }, //TODO check if alert is appropriate here
	{ ABP_PNIO_CMD_PLUG_SUB_FAILED,		"Plug_Sub_Failed",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::Alert }, //TODO check if alert is appropriate here
	{ ABP_PNIO_CMD_EXPECTED_IDENT_IND,	"Expected_Ident_Ind",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_SAVE_IP_SUITE,		"Save_IP_Suite",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_SAVE_STATION_NAME,	"Save_Station_Name",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_INDICATE_DEVICE,		"Indicate_Device",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None },
	{ ABP_PNIO_CMD_PRM_BEGIN_IND,		"Prm_Begin_Ind",		BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

static const CmdLookupTable_t asSrc3CmdNames[] =
{
	{ ABP_SRC3_CMD_RESET_DIAGNOSTIC,	"Reset_Diagnostic",	BaseType::Numeric,	BaseType::Numeric,	NotifEvent::None }
};

CommandNameTable_t asCommandNameTables[] =
{
	{ ABP_OBJ_NUM_ADD,		NUM_ENTRIES(asAddCmdNames),		asAddCmdNames },
	{ ABP_OBJ_NUM_NW,		NUM_ENTRIES(asNwCmdNames),		asNwCmdNames },
	{ ABP_OBJ_NUM_NWCCL,	NUM_ENTRIES(asNwCclCmdNames),	asNwCclCmdNames },
	{ ABP_OBJ_NUM_FUSM,		NUM_ENTRIES(asFusmCmdNames),	asFusmCmdNames },
	{ ABP_OBJ_NUM_FSI,		NUM_ENTRIES(asFsiCmdNames),		asFsiCmdNames },
	{ ABP_OBJ_NUM_AFSI,		NUM_ENTRIES(asFsiCmdNames),		asFsiCmdNames },
	{ ABP_OBJ_NUM_ASM,		NUM_ENTRIES(asAsmCmdNames),		asAsmCmdNames },
	{ ABP_OBJ_NUM_BAC,		NUM_ENTRIES(asBacCmdNames),		asBacCmdNames },
	{ ABP_OBJ_NUM_CCL,		NUM_ENTRIES(asCclCmdNames),		asCclCmdNames },
	{ ABP_OBJ_NUM_CFN,		NUM_ENTRIES(asCfnCmdNames),		asCfnCmdNames },
	{ ABP_OBJ_NUM_CNT,		NUM_ENTRIES(asCntCmdNames),		asCntCmdNames },
	{ ABP_OBJ_NUM_CPN,		NUM_ENTRIES(asCpnCmdNames),		asCpnCmdNames },
	{ ABP_OBJ_NUM_DEV,		NUM_ENTRIES(asDevCmdNames),		asDevCmdNames },
	{ ABP_OBJ_NUM_DPV1,		NUM_ENTRIES(asDpv1CmdNames),	asDpv1CmdNames },
	{ ABP_OBJ_NUM_ECO,		NUM_ENTRIES(asEcoCmdNames),		asEcoCmdNames },
	{ ABP_OBJ_NUM_MOD,		NUM_ENTRIES(asModCmdNames),		asModCmdNames },
	{ ABP_OBJ_NUM_MDD,		NUM_ENTRIES(asMddCmdNames),		asMddCmdNames },
	{ ABP_OBJ_NUM_MQTT,		NUM_ENTRIES(asMqttCmdNames),	asMqttCmdNames },
	{ ABP_OBJ_NUM_PNIO,		NUM_ENTRIES(asPnioCmdNames),	asPnioCmdNames },
	{ ABP_OBJ_NUM_EIP,		NUM_ENTRIES(asEipCmdNames),		asEipCmdNames },
	{ ABP_OBJ_NUM_ECT,		NUM_ENTRIES(asEctCmdNames),		asEctCmdNames },
	{ ABP_OBJ_NUM_APPD,		NUM_ENTRIES(asAppdCmdNames),	asAppdCmdNames },
	{ ABP_OBJ_NUM_APP,		NUM_ENTRIES(asAppCmdNames),		asAppCmdNames },
	{ ABP_OBJ_NUM_NWCFN,	NUM_ENTRIES(asNwCfnCmdNames),	asNwCfnCmdNames },
	{ ABP_OBJ_NUM_NWPNIO,	NUM_ENTRIES(asNwPnioCmdNames),	asNwPnioCmdNames },
	{ ABP_OBJ_NUM_NWDPV1,	NUM_ENTRIES(asNwDpv1CmdNames),	asNwDpv1CmdNames },
	{ ABP_OBJ_NUM_EME,		NUM_ENTRIES(asEmeCmdNames),		asEmeCmdNames },
	{ ABP_OBJ_NUM_SRC3,		NUM_ENTRIES(asSrc3CmdNames),	asSrc3CmdNames },
};

/*******************************************************************************
**
** Common Error response lookup table
**
*******************************************************************************/

static const LookupTable_t asErrorRspNames[] =
{
	{ ABP_ERR_INV_MSG_FORMAT,					"Invalid message format",							NotifEvent::Alert },
	{ ABP_ERR_UNSUP_OBJ,						"Unsupported object",								NotifEvent::Alert },
	{ ABP_ERR_UNSUP_INST,						"Unsupported instance",								NotifEvent::Alert },
	{ ABP_ERR_UNSUP_CMD,						"Unsupported command",								NotifEvent::Alert },
	{ ABP_ERR_INV_CMD_EXT_0,					"Invalid CmdExt0",									NotifEvent::Alert },
	{ ABP_ERR_INV_CMD_EXT_1,					"Invalid CmdExt1",									NotifEvent::Alert },
	{ ABP_ERR_ATTR_NOT_SETABLE,					"Attribute not settable",							NotifEvent::Alert },
	{ ABP_ERR_ATTR_NOT_GETABLE,					"Attribute not gettable",							NotifEvent::Alert },
	{ ABP_ERR_TOO_MUCH_DATA,					"Too much data",									NotifEvent::Alert },
	{ ABP_ERR_NOT_ENOUGH_DATA,					"Not enough data",									NotifEvent::Alert },
	{ ABP_ERR_OUT_OF_RANGE,						"Out of range",										NotifEvent::Alert },
	{ ABP_ERR_INV_STATE,						"Invalid state",									NotifEvent::Alert },
	{ ABP_ERR_NO_RESOURCES,						"Out of resources",									NotifEvent::Alert },
	{ ABP_ERR_SEG_FAILURE,						"Segmentation failure",								NotifEvent::Alert },
	{ ABP_ERR_SEG_BUF_OVERFLOW,					"Segmentation buffer overflow",						NotifEvent::Alert },
	{ ABP_ERR_VAL_TOO_HIGH,						"Value too high",									NotifEvent::Alert },
	{ ABP_ERR_VAL_TOO_LOW,						"Value too low",									NotifEvent::Alert },
	{ ABP_ERR_CONTROLLED_FROM_OTHER_CHANNEL,	"NAK writes to \"read process data\" mapped attr.",	NotifEvent::Alert },
	{ ABP_ERR_MSG_CHANNEL_TOO_SMALL,			"Response does not fit",							NotifEvent::Alert },
	{ ABP_ERR_GENERAL_ERROR,					"General error",									NotifEvent::Alert },
	{ ABP_ERR_PROTECTED_ACCESS,					"Protected access",									NotifEvent::Alert },
	{ ABP_ERR_DATA_NOT_AVAILABLE,				"Data not available",								NotifEvent::Alert },
	{ ABP_ERR_OBJ_SPECIFIC,						"Object specific error",							NotifEvent::Alert }
};

/*******************************************************************************
**
** Object specific error response lookup tables
**
*******************************************************************************/

static const LookupTable_t asAnbErrNames[] =
{
	{ ABP_ANB_ERR_INV_PRD_CFG,		"Invalid process data config",		NotifEvent::Alert },
	{ ABP_ANB_ERR_INV_DEV_ADDR,		"Invalid device address",			NotifEvent::Alert },
	{ ABP_ANB_ERR_INV_COM_SETTINGS,	"Invalid communication settings",	NotifEvent::Alert }
};

static const LookupTable_t asAppdErrNames[] =
{
	{ ABP_APPD_ERR_MAPPING_ITEM_NAK,				"Mapping item NAK",				NotifEvent::Alert },
	{ ABP_APPD_ERR_INVALID_TOTAL_SIZE,				"Invalid total size",			NotifEvent::Alert },
	{ ABP_APPD_ERR_ATTR_CTRL_FROM_OTHER_CHANNEL,	"Attr ctrl from other channel",	NotifEvent::Alert }
};

static const LookupTable_t asDiErrNames[] =
{
	{ ABP_DI_ERR_NOT_REMOVED,		"Event could not be removed",		NotifEvent::Alert },
	{ ABP_DI_LATCH_NOT_SUPPORTED,	"Latching events not supported",	NotifEvent::Alert },
	{ ABP_DI_ERR_NW_SPECIFIC,		"Network specific error",			NotifEvent::Alert }
};

static const LookupTable_t asPirDiErrNames[] =
{
	{ 0x03,	"API does not exist",								NotifEvent::Alert },
	{ 0x04,	"No module inserted in the specified slot",			NotifEvent::Alert },
	{ 0x05,	"No submodule inserted in the specified subslot",	NotifEvent::Alert },
	{ 0x06,	"Slot number specified is out-of-range",			NotifEvent::Alert },
	{ 0x07,	"Subslot number specified is out-of-range",			NotifEvent::Alert },
	{ 0x08,	"Failed to add the channel diagnostic entry",		NotifEvent::Alert },
	{ 0x09,	"Failed to send the channel diagnostic alarm",		NotifEvent::Alert },
	{ 0x0A,	"Channel number out-of-range",						NotifEvent::Alert },
	{ 0x0B,	"ChannelPropType out-of-range",						NotifEvent::Alert },
	{ 0x0C,	"ChannelPropDir out-of-range",						NotifEvent::Alert },
	{ 0x0D,	"ChannelPropAcc out-of-range",						NotifEvent::Alert },
	{ 0x0E,	"ChannelPropMaintReq out-of-range",					NotifEvent::Alert },
	{ 0x0F,	"ChannelPropMaintDem out-of-range",					NotifEvent::Alert },
	{ 0x10,	"UserStructIdent out-of-range",						NotifEvent::Alert },
	{ 0x11,	"ChannelErrType out-of-range",						NotifEvent::Alert },
	{ 0xFF,	"Unknown error",									NotifEvent::Alert }
};

static const LookupTable_t asEipErrNames[] =
{
	{ ABP_EIP_ERR_OWNERSHIP_CONFLICT,	"Ownership conflict",		NotifEvent::Alert },
	{ ABP_EIP_ERR_INVALID_CONFIG,		"Invalid configuration",	NotifEvent::Alert }
};

static const LookupTable_t asFsiErrNames[] =
{
	{ ABP_FSI_ERR_FILE_OPEN_FAILED,				"File_Open Failed",			NotifEvent::Alert },
	{ ABP_FSI_ERR_FILE_CLOSE_FAILED,			"File_Close Failed",		NotifEvent::Alert },
	{ ABP_FSI_ERR_FILE_DELETE_FAILED,			"File_Delete Failed",		NotifEvent::Alert },
	{ ABP_FSI_ERR_DIRECTORY_OPEN_FAILED,		"Directory_Open Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_DIRECTORY_CLOSE_FAILED,		"Directory_Close Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_DIRECTORY_CREATE_FAILED,		"Directory_Create Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_DIRECTORY_DELETE_FAILED,		"Directory_Delete Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_DIRECTORY_CHANGE_FAILED,		"Directory_Change Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_FILE_COPY_OPEN_READ_FAILED,	"Copy Open Read Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_FILE_COPY_OPEN_WRITE_FAILED,	"Copy Open Write Failed",	NotifEvent::Alert },
	{ ABP_FSI_ERR_FILE_COPY_WRITE_FAILED,		"Copy Write Failed",		NotifEvent::Alert },
	{ ABP_FSI_ERR_FILE_RENAME_FAILED,			"File_Rename Failed",		NotifEvent::Alert }
};

static const LookupTable_t asFusmErrNames[] =
{
	{ ABP_FUSM_ERR_REJECT_BY_MODULE,	"Rejected by module",			NotifEvent::Alert },
	{ ABP_FUSM_ERR_MODULE_RSP_FAULTY,	"Module response is faulty",	NotifEvent::Alert }
};

static const LookupTable_t asModErrNames[] =
{
	{ ABP_MOD_NW_EXCPT_MISSING_MAC_ADDRESS,	"Missing MAC Address",	NotifEvent::Alert }
};

static const LookupTable_t asNwErrNames[] =
{
	{ ABP_NW_ERR_INVALID_ADI_DATA_TYPE,	"Invalid ADI data type",		NotifEvent::Alert },
	{ ABP_NW_ERR_INVALID_NUM_ELEMENTS,	"Invalid number of elements",	NotifEvent::Alert },
	{ ABP_NW_ERR_INVALID_TOTAL_SIZE,	"Invalid total size",			NotifEvent::Alert },
	{ ABP_NW_ERR_MULTIPLE_MAPPING,		"Multiple mapping",				NotifEvent::Alert },
	{ ABP_NW_ERR_INVALID_ORDER_NUM,		"Invalid ADI order number",		NotifEvent::Alert },
	{ ABP_NW_ERR_INVALID_MAP_CMD_SEQ,	"Invalid map cmd sequence",		NotifEvent::Alert },
	{ ABP_NW_ERR_INVALID_MAP_CMD,		"Command impossible to parse",	NotifEvent::Alert },
	{ ABP_NW_ERR_BAD_ALIGNMENT,			"Invalid data alignment",		NotifEvent::Alert },
	{ ABP_NW_ERR_INVALID_ADI_0,			"Invalid use of ADI 0",			NotifEvent::Alert },
	{ ABP_NW_ERR_NW_SPEC_RESTRICTION,	"Network specific restriction",	NotifEvent::Alert }
};

static const LookupTable_t asNwCclErrNames[] =
{
	{ ABP_NWCCL_ERR_INVALID_ADI_DATA_TYPE,	"Invalid ADI data type",		NotifEvent::Alert },
	{ ABP_NWCCL_ERR_INVALID_NUM_ELEMENTS,	"Invalid number of elements",	NotifEvent::Alert },
	{ ABP_NWCCL_ERR_INVALID_TOTAL_SIZE,		"Invalid total size",			NotifEvent::Alert },
	{ ABP_NWCCL_ERR_INVALID_ORDER_NUM,		"Invalid ADI order number",		NotifEvent::Alert },
	{ ABP_NWCCL_ERR_INVALID_MAP_CMD_SEQ,	"Invalid map cmd sequence",		NotifEvent::Alert },
	{ ABP_NWCCL_ERR_INVALID_CCL_AREA,		"Invalid CCL area",				NotifEvent::Alert },
	{ ABP_NWCCL_ERR_INVALID_OFFSET,			"Invalid offset",				NotifEvent::Alert },
	{ ABP_NWCCL_ERR_DATA_OVERLAPPING,		"Data overlapping",				NotifEvent::Alert }
};

static const LookupTable_t asNwDpv1ErrNames[] =
{
	{ ABP_NWDPV1_ERR_INVALID_ADI_DATA_TYPE,		"Invalid ADI data type",		NotifEvent::Alert },
	{ ABP_NWDPV1_ERR_INVALID_NUM_ELEMENTS,		"Invalid number of elements",	NotifEvent::Alert },
	{ ABP_NWDPV1_ERR_INVALID_TOTAL_SIZE,		"Invalid total size",			NotifEvent::Alert },
	{ ABP_NWDPV1_ERR_INVALID_ORDER_NUM,			"Invalid ADI order number",		NotifEvent::Alert },
	{ ABP_NWDPV1_ERR_INVALID_MAP_CMD_SEQ,		"Invalid map cmd sequence",		NotifEvent::Alert },
	{ ABP_NWDPV1_ERR_INVALID_CFG_DATA,			"Invalid configuration data",	NotifEvent::Alert },
	{ ABP_NWDPV1_ERR_TOO_MUCH_TOTAL_CFG_DATA,	"Too much total config data",	NotifEvent::Alert }
};

static const LookupTable_t asNwPnioErrNames[] =
{
	{ ABP_NWPNIO_ERR_ADI_WRITE_NOT_MAPPED,		"ADI not mapped via Map_ADI_Write_Area",	NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_ADI_READ_NOT_MAPPED,		"ADI not mapped via Map_ADI_Read_Area",		NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_ADI_ELEM_NOT_PRESENT,		"ADI element not present",					NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_ADI_ALREADY_MAPPED,		"ADI/Element is already mapped",			NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_API_0_NOT_ADDED,			"API 0 must be added first",				NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_API_NOT_PRESENT,			"API does not exist",						NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_API_ALREADY_PRESENT,		"API already present",						NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_API_CANNOT_BE_ADDED,		"API cannot be added",						NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_NO_IO_IN_SLOT_0,			"Slot 0 cannot have any I/O data",			NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_SLOT_0_NOT_PROP_PLUGGED,	"Slot 0 not properly plugged",				NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_SLOT_OCCUPIED,				"Slot occupied",							NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_SUBSLOT_OCCUPIED,			"Subslot occupied",							NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_NO_MODULE_SPECIFIED_SLOT,	"No module in the specified slot",			NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_NO_SUBMOD_SPECIFIED_SLOT,	"No submodule in the specified slot",		NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_SLOT_OUT_OF_RANGE,			"Slot number specified is out of range",	NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_SUBSLOT_OUT_OF_RANGE,		"Subslot number specified is out of range",	NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_AR_NOT_VALID,				"AR handle provided is not valid",			NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_NO_PEND_APPL_READY,		"No pending application ready",				NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_UNKNOWN_STACK_ERROR,		"Unknown stack error",						NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_MAX_NBR_OF_PLUGGED_SUBMOD,	"Max number of plugged submodules",			NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_SAFETY_NOT_ENABLED,		"Safety module has not been plugged",		NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_ADI_DATATYPE_CONSTRAINT,	"ADI datatype constraint",					NotifEvent::Alert },
	{ ABP_NWPNIO_ERR_ASM_ALREADY_PLUGGED,		"Safety module already plugged",			NotifEvent::Alert }
};

static const LookupTable_t asSmtpErrNames[] =
{
	{ ABP_SMTP_NO_EMAIL_SERVER,			"No e-mail server",			NotifEvent::Alert },
	{ ABP_SMTP_SERVER_NOT_READY,		"Server not ready",			NotifEvent::Alert },
	{ ABP_SMTP_AUTHENTICATION_ERROR,	"Authentication error",		NotifEvent::Alert },
	{ ABP_SMTP_SOCKET_ERROR,			"Socket error",				NotifEvent::Alert },
	{ ABP_SMTP_SSI_SCAN_ERROR,			"SSI scan error",			NotifEvent::Alert },
	{ ABP_SMTP_FILE_ERROR,				"File error",				NotifEvent::Alert },
	{ ABP_SMTP_OTHER,					"Other",					NotifEvent::Alert }
};


static const LookupTable_t asSocErrNames[] =
{
	{ SOC_ERR_ENOBUFS,			"ENOBUFS",			NotifEvent::Alert },
	{ SOC_ERR_ETIMEDOUT,		"ETIMEDOUT",		NotifEvent::Alert },
	{ SOC_ERR_EISCONN,			"EISCONN",			NotifEvent::Alert },
	{ SOC_ERR_EOPNOTSUPP,		"EOPNOTSUPP",		NotifEvent::Alert },
	{ SOC_ERR_ECONNABORTED,		"ECONNABORTED",		NotifEvent::Alert },
	{ SOC_ERR_EWOULDBLOCK,		"EWOULDBLOCK",		NotifEvent::Alert },
	{ SOC_ERR_ECONNREFUSED,		"ECONNREFUSED",		NotifEvent::Alert },
	{ SOC_ERR_ECONNRESET,		"ECONNRESET",		NotifEvent::Alert },
	{ SOC_ERR_ENOTCONN,			"ENOTCONN",			NotifEvent::Alert },
	{ SOC_ERR_EALREADY,			"EALREADY",			NotifEvent::Alert },
	{ SOC_ERR_EINVAL,			"EINVAL",			NotifEvent::Alert },
	{ SOC_ERR_EMSGSIZE,			"EMSGSIZE",			NotifEvent::Alert },
	{ SOC_ERR_EPIPE,			"EPIPE",			NotifEvent::Alert },
	{ SOC_ERR_EDESTADDRREQ,		"EDESTADDRREQ",		NotifEvent::Alert },
	{ SOC_ERR_ESHUTDOWN,		"ESHUTDOWN",		NotifEvent::Alert },
	{ SOC_ERR_EHAVEOOB,			"EHAVEOOB",			NotifEvent::Alert },
	{ SOC_ERR_ENOMEM,			"ENOMEM",			NotifEvent::Alert },
	{ SOC_ERR_EADDRNOTAVAIL,	"EADDRNOTAVAIL",	NotifEvent::Alert },
	{ SOC_ERR_EADDRINUSE,		"EADDRINUSE",		NotifEvent::Alert },
	{ SOC_ERR_EINPROGRESS,		"EINPROGRESS",		NotifEvent::Alert },
	{ SOC_ERR_ETOOMANYREFS,		"ETOOMANYREFS",		NotifEvent::Alert },
	{ SOC_ERR_CMD_ABORTED,		"CMD_ABORTED",		NotifEvent::Alert },
	{ SOC_ERR_DNS_NAME,			"DNS_NAME",			NotifEvent::Alert },
	{ SOC_ERR_DNS_TIMEOUT,		"DNS_TIMEOUT",		NotifEvent::Alert },
	{ SOC_ERR_DNS_CMD_FAILED,	"DNS_CMD_FAILED",	NotifEvent::Alert }
};

ErrorNameTable_t asErrorNameTables[] =
{
	{ ABP_OBJ_NUM_FSI,		NUM_ENTRIES(asFsiErrNames),		asFsiErrNames },
	{ ABP_OBJ_NUM_AFSI,		NUM_ENTRIES(asFsiErrNames),		asFsiErrNames },
	{ ABP_OBJ_NUM_FUSM,		NUM_ENTRIES(asFusmErrNames),	asFusmErrNames },
	{ ABP_OBJ_NUM_ANB,		NUM_ENTRIES(asAnbErrNames),		asAnbErrNames },
	// NOTE: ABP_OBJ_NUM_DI is handled in a unique way
	{ ABP_OBJ_NUM_MOD,		NUM_ENTRIES(asModErrNames),		asModErrNames },
	{ ABP_OBJ_NUM_NW,		NUM_ENTRIES(asNwErrNames),		asNwErrNames },
	{ ABP_OBJ_NUM_NWCCL,	NUM_ENTRIES(asNwCclErrNames),	asNwCclErrNames },
	{ ABP_OBJ_NUM_NWDPV1,	NUM_ENTRIES(asNwDpv1ErrNames),	asNwDpv1ErrNames },

	{ ABP_OBJ_NUM_NWPNIO,	NUM_ENTRIES(asNwPnioErrNames),	asNwPnioErrNames },
	{ ABP_OBJ_NUM_APPD,		NUM_ENTRIES(asAppdErrNames),	asAppdErrNames },
	{ ABP_OBJ_NUM_SMTP,		NUM_ENTRIES(asSmtpErrNames),	asSmtpErrNames },

	{ ABP_OBJ_NUM_SOC,		NUM_ENTRIES(asSocErrNames),		asSocErrNames },
	{ ABP_OBJ_NUM_EIP,		NUM_ENTRIES(asEipErrNames),		asEipErrNames }
};

static const CmdLookupTable_t* FindCmdEntryInTable(U8 cmd,
	const CmdLookupTable_t* command_names, U8 num_commands);

static const AttrLookupTable_t* FindAttrEntryInTable(U16 inst, U8 attr,
	const AttrLookupTable_t* obj_names, U8 num_obj_names,
	const AttrLookupTable_t* inst_names, U8 num_inst_names);

static const CmdLookupTable_t* LookupCmdEntry(U8 obj, U8 cmd);
static const AttrLookupTable_t* LookupAttrEntry(U8 obj, U16 inst, U8 attr);

void GetNumberString(U64 number, DisplayBase display_base, U32 num_data_bits, char* result_string, U32 result_string_max_length, BaseType base_type )
{
	if (base_type == BaseType::Numeric)
	{
		if ((display_base == DisplayBase::ASCII) || (display_base == DisplayBase::AsciiHex))
		{
			display_base = DisplayBase::Hexadecimal;
		}
	}

	AnalyzerHelpers::GetNumberString(number, display_base, num_data_bits, result_string, result_string_max_length);
}

NotifEvent_t GetSpiCtrlString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	bool firstFlag = true;
	NotifEvent_t notification = NotifEvent::None;
	const char separatorStr[] = " | ";
	display_base = display_base; // Resolve compiler warning

	for (U8 i = 0; i < NUM_ENTRIES(asSpiCtrlNames); i++)
	{
		if (((asSpiCtrlNames[i].value & val) != 0) || (asSpiCtrlNames[i].value == ABP_SPI_CTRL_CMDCNT))
		{
			if (!firstFlag)
			{
				SNPRINTF(str, max_str_len, separatorStr);
				str += strlen(separatorStr);
				max_str_len -= (U16)strlen(separatorStr);
			}

			firstFlag = false;

			if (notification == NotifEvent::None)
			{
				notification = asSpiCtrlNames[i].notification;
			}

			if (asSpiCtrlNames[i].value == ABP_SPI_CTRL_CMDCNT)
			{
				// Special handling for command count
				SNPRINTF(str, max_str_len, "%s%d", asSpiCtrlNames[i].name, (val & ABP_SPI_CTRL_CMDCNT) >> 1);
				str += (strlen(asSpiCtrlNames[i].name) + 1);
				max_str_len -= (U16)(strlen(asSpiCtrlNames[i].name) + 1);
			}
			else
			{
				SNPRINTF(str, max_str_len, asSpiCtrlNames[i].name);
				str += strlen(asSpiCtrlNames[i].name);
				max_str_len -= (U16)strlen(asSpiCtrlNames[i].name);
			}
		}
	}

	return notification;
}

NotifEvent_t GetSpiStsString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	bool firstFlag = true;
	NotifEvent_t notification = NotifEvent::None;
	const char separatorStr[] = " | ";

	display_base = display_base; // Resolve compiler warning

	for (U8 i = 0; i < NUM_ENTRIES(asSpiStsNames); i++)
	{
		if (((asSpiStsNames[i].value & val) != 0) || (asSpiStsNames[i].value == ABP_SPI_STATUS_CMDCNT))
		{
			if (!firstFlag)
			{
				SNPRINTF(str, max_str_len, separatorStr);
				str += strlen(separatorStr);
				max_str_len -= (U16)strlen(separatorStr);
			}

			firstFlag = false;

			if (notification == NotifEvent::None)
			{
				notification = asSpiStsNames[i].notification;
			}

			if (asSpiStsNames[i].value == ABP_SPI_STATUS_CMDCNT)
			{
				// Special handling for command count
				SNPRINTF(str, max_str_len, "%s%d", asSpiStsNames[i].name, (val & ABP_SPI_STATUS_CMDCNT) >> 1);
				str += (strlen(asSpiStsNames[i].name) + 1);
				max_str_len -= ((U16)strlen(asSpiStsNames[i].name) + 1);
			}
			else
			{
				SNPRINTF(str, max_str_len, asSpiStsNames[i].name);
				str += strlen(asSpiStsNames[i].name);
				max_str_len -= (U16)strlen(asSpiStsNames[i].name);
			}
		}
	}

	return notification;
}

NotifEvent_t GetApplStsString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	bool found = false;

	for (U8 i = 0; i < NUM_ENTRIES(asApplStsNames); i++)
	{
		if (asApplStsNames[i].value == val)
		{
			SNPRINTF(str, max_str_len, asApplStsNames[i].name);
			notification = asApplStsNames[i].notification;
			found = true;
			break;
		}
	}

	if (!found)
	{
		GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::ApplicationStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
		SNPRINTF(str, max_str_len, "Reserved: %s", numberStr);
		notification = NotifEvent::Alert;
	}

	return notification;
}

NotifEvent_t GetAbccStatusString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	bool found = false;
	char tmpstr[FORMATTED_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < NUM_ENTRIES(asAnybusStsNames); i++)
	{
		if (asAnybusStsNames[i].value == (val & (ABCC_STATUS_CODE_MASK | ABCC_STATUS_RESERVED_MASK)))
		{
			SNPRINTF(tmpstr, sizeof(tmpstr), asAnybusStsNames[i].name);
			notification = asAnybusStsNames[i].notification;
			found = true;
			break;
		}
	}

	if (!found)
	{
		GetNumberString(val, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::AnybusStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
		SNPRINTF(tmpstr, sizeof(tmpstr), "Reserved: %s", numberStr);
		notification = NotifEvent::Alert;
	}

	if ((val & ABCC_STATUS_SUP_MASK) == ABCC_STATUS_SUP_MASK)
	{
		SNPRINTF(str, max_str_len, "%s | SUP", tmpstr);
	}
	else
	{
		SNPRINTF(str, max_str_len, "%s", tmpstr);
	}

	return notification;
}

NotifEvent_t GetErrorRspString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < NUM_ENTRIES(asErrorRspNames); i++)
	{
		if (asErrorRspNames[i].value == val)
		{
			SNPRINTF(str, max_str_len, asErrorRspNames[i].name);
			return asErrorRspNames[i].notification;
		}
	}

	GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
	SNPRINTF(str, max_str_len, "Reserved: %s", numberStr);

	return NotifEvent::Alert;
}

NotifEvent_t GetObjSpecificErrString(U8 val, char* str, U16 max_str_len, const LookupTable_t* pasErrNames, U8 bNoErrors, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < bNoErrors; i++)
	{
		if (pasErrNames[i].value == val)
		{
			SNPRINTF(str, max_str_len, pasErrNames[i].name);
			return pasErrNames[i].notification;
		}
	}

	GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
	SNPRINTF(str, max_str_len, "Unknown: %s", numberStr);

	return NotifEvent::Alert;
}

NotifEvent_t GetErrorRspString(U8 nw_type_idx, U8 obj, U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	// Special handling is needed for ABP_OBJ_NUM_DI.
	if (obj == ABP_OBJ_NUM_DI)
	{
		if (nw_type_idx == 0)
		{
			// Use object-specific lookup table
			GetObjSpecificErrString(val, str, max_str_len, &asDiErrNames[0],
				NUM_ENTRIES(asDiErrNames), display_base);
		}
		else
		{
			// Use network-specific lookup table
			switch (abNetworkTypeValue[nw_type_idx])
			{
			case ABP_NW_TYPE_PRT:
			case ABP_NW_TYPE_PRT_2P:
			case ABP_NW_TYPE_PIR:
			case ABP_NW_TYPE_PIR_FO:
			case ABP_NW_TYPE_PIR_FO_IIOT:
			case ABP_NW_TYPE_PIR_IIOT:
				// Profinet
				GetObjSpecificErrString(val, str, max_str_len, &asPirDiErrNames[0],
					NUM_ENTRIES(asPirDiErrNames), display_base);
				break;
			default:
				GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, max_str_len, BaseType::Numeric);
				SNPRINTF(str, max_str_len, "Unknown: %s", numberStr);
				break;
			}
		}
	}
	else
	{
		bool objFound = false;

		for (U8 i = 0; i < sizeof(asErrorNameTables) / sizeof(ErrorNameTable_t); i++)
		{
			if (asErrorNameTables[i].object_num == obj)
			{
				GetObjSpecificErrString(
					val,
					str,
					max_str_len,
					asErrorNameTables[i].err_names,
					asErrorNameTables[i].num_err_names,
					display_base);

				break;
			}
		}

		if (!objFound)
		{
			GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, max_str_len, BaseType::Numeric);
			SNPRINTF(str, max_str_len, "Unknown: 0x%02X, %s", obj, numberStr);
		}
	}

	return NotifEvent::Alert;
}

NotifEvent_t GetIntMaskString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	bool firstFlag = true;
	NotifEvent_t notification = NotifEvent::None;
	const char separatorStr[] = " | ";

	display_base = display_base; // Resolve compiler warning

	for (U8 i = 0; i < NUM_ENTRIES(asIntMaskNames); i++)
	{
		if ((asIntMaskNames[i].value & val) != 0)
		{
			if (!firstFlag)
			{
				SNPRINTF(str, max_str_len, separatorStr);
				str += (U16)strlen(separatorStr);
				max_str_len -= (U16)strlen(separatorStr);
			}

			firstFlag = false;

			if (notification == NotifEvent::None)
			{
				notification = asIntMaskNames[i].notification;
			}

			SNPRINTF(str, max_str_len, asIntMaskNames[i].name);
			str += (U16)strlen(asIntMaskNames[i].name);
			max_str_len -= (U16)strlen(asIntMaskNames[i].name);
		}
	}

	if (firstFlag)
	{
		SNPRINTF(str, max_str_len, "None");
	}

	return notification;
}

NotifEvent_t GetLedStatusString(U16 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	bool firstFlag = true;
	NotifEvent_t notification = NotifEvent::None;
	const char separatorStr[] = " | ";

	display_base = display_base; // Resolve compiler warning

	for (U8 i = 0; i < NUM_ENTRIES(asLedStsNames); i++)
	{
		if ((asLedStsNames[i].value & val) != 0)
		{
			if (!firstFlag)
			{
				SNPRINTF(str, max_str_len, separatorStr);
				str += (U16)strlen(separatorStr);
				max_str_len -= (U16)strlen(separatorStr);
			}

			firstFlag = false;

			if (notification == NotifEvent::None)
			{
				notification = asLedStsNames[i].notification;
			}

			SNPRINTF(str, max_str_len, asLedStsNames[i].name);
			str += (U16)strlen(asLedStsNames[i].name);
			max_str_len -= (U16)strlen(asLedStsNames[i].name);
		}
	}

	if (firstFlag)
	{
		SNPRINTF(str, max_str_len, "None");
	}

	return notification;
}

NotifEvent_t GetNamedInstString(
	U16 val,
	char* str, U16 max_str_len,
	DisplayBase /* display_base */,
	const LookupTable_t* inst_names, U8 num_inst_names)
{
	NotifEvent_t notification = NotifEvent::None;

	if (val == 0)
	{
		SNPRINTF(str, max_str_len, "Object");
	}
	else
	{
		bool found = false;

		for (U8 i = 0; i < num_inst_names; i++)
		{
			if (inst_names[i].value == val)
			{
				SNPRINTF(str, max_str_len, inst_names[i].name);
				notification = inst_names[i].notification;
				found = true;
				break;
			}
		}

		if (!found)
		{
			SNPRINTF(str, max_str_len, "Unknown: %d (0x%04X)", val, val);
			notification = NotifEvent::Alert;
		}
	}

	return notification;
}

NotifEvent_t GetNamedAttrString(
	U16 inst, U8 val,
	char* str, U16 max_str_len,
	DisplayBase display_base,
	const AttrLookupTable_t* obj_names, U8 num_obj_names,
	const AttrLookupTable_t* inst_names, U8 num_inst_names)
{
	const AttrLookupTable_t* entryPtr = nullptr;
	U8 numAttrs = 0;
	NotifEvent_t notification = NotifEvent::Alert;
	bool found = false;

	if (inst == ABP_INST_OBJ)
	{
		if (val <= asObjAttrNames[NUM_ENTRIES(asObjAttrNames) - 1].value)
		{
			entryPtr = &asObjAttrNames[0];
			numAttrs = NUM_ENTRIES(asObjAttrNames);
		}
		else
		{
			if (obj_names != nullptr)
			{
				entryPtr = obj_names;
				numAttrs = num_obj_names;
			}
		}
	}
	else
	{
		if (num_inst_names != 0)
		{
			entryPtr = inst_names;
			numAttrs = num_inst_names;
		}
	}

	if (entryPtr != nullptr)
	{
		for (U8 i = 0; i < numAttrs; i++)
		{
			if (entryPtr[i].value == val)
			{
				SNPRINTF(str, max_str_len, entryPtr[i].name);
				notification = entryPtr[i].notification;
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
		GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
		SNPRINTF(str, max_str_len, "Unknown: %s", numberStr);
	}

	return notification;
}

NotifEvent_t GetObjectString(U8 val, char* str, U16 max_str_len, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < NUM_ENTRIES(asObjectNames); i++)
	{
		if (asObjectNames[i].value == val)
		{
			SNPRINTF(str, max_str_len, asObjectNames[i].name);

			return asObjectNames[i].notification;
		}
	}

	GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
	SNPRINTF(str, max_str_len, "Unknown: %s", numberStr);

	return NotifEvent::Alert;
}

NotifEvent_t GetObjSpecificCmdString(U8 val, char* str, U16 max_str_len, const CmdLookupTable_t* command_names, U8 num_commands, DisplayBase display_base)
{
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < num_commands; i++)
	{
		if (command_names[i].value == val)
		{
			SNPRINTF(str, max_str_len, command_names[i].name);

			return command_names[i].notification;
		}
	}

	GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
	SNPRINTF(str, max_str_len, "Unknown: %s", numberStr);

	return NotifEvent::Alert;
}

NotifEvent_t GetCmdString(U8 val, U8 obj, char* str, U16 max_str_len, DisplayBase display_base)
{
	const CmdLookupTable_t* entryPtr = LookupCmdEntry(obj, val);
	NotifEvent_t notification;

	U8 cmd = (val & ABP_MSG_HEADER_CMD_BITS);

	if (entryPtr == nullptr)
	{
		notification = NotifEvent::Alert;
		GetNumberString(cmd, display_base, SIZE_IN_BITS(val), str, max_str_len, BaseType::Numeric);
	}
	else
	{
		notification = entryPtr->notification;
		SNPRINTF(str, max_str_len, entryPtr->name);
	}

	return notification;
}

bool GetInstString(U8 nw_type_idx, U8 obj, U16 val, char* str, U16 max_str_len, NotifEvent_t* notif_ptr, DisplayBase display_base)
{
	bool objFound = false;

	if (obj == ABP_OBJ_NUM_NC)
	{
		for (U8 i = 0; i < sizeof(asNcInstNameTables) / sizeof(NcInstNameTable_t); i++)
		{
			if (asNcInstNameTables[i].object_num == obj)
			{
				objFound = true;
				*notif_ptr = GetNamedInstString(
					(U8)val,
					&str[0],
					max_str_len,
					display_base,
					asNcInstNameTables[i].inst_names,
					asNcInstNameTables[i].num_inst_names);

				break;
			}
		}
	}

	return objFound;
}

bool GetAttrString(U8 obj, U16 inst, U16 val, char* str, U16 max_str_len, AttributeAccessMode_t access_mode, NotifEvent_t* notif_ptr, DisplayBase display_base)
{
	bool entryFound = true;
	U8 offset = 0;
	const AttrLookupTable_t* entryPtr = LookupAttrEntry(obj, inst, (U8)val);

	if (access_mode == AttributeAccessMode::Indexed)
	{
		SNPRINTF(str, max_str_len, "Element %d, ", (U8)(val >> 8));
		offset = (U8)strlen(str);
		max_str_len -= offset;
	}

	if (entryPtr == nullptr)
	{
		char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
		entryFound = false;
		*notif_ptr = NotifEvent::Alert;
		GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
		SNPRINTF(&str[offset], max_str_len, "Unknown: %s", numberStr);
	}
	else
	{
		*notif_ptr = entryPtr->notification;
		SNPRINTF(&str[offset], max_str_len, entryPtr->name);
	}

	return entryFound;
}

BaseType GetAttrBaseType(U8 obj, U16 inst, U8 attr)
{
	BaseType type;
	const AttrLookupTable_t* entryPtr = LookupAttrEntry(obj, inst, attr);

	if (entryPtr != nullptr)
	{
		type = entryPtr->msgDataType;
	}
	else
	{
		type = BaseType::Numeric;
	}

	return type;
}

BaseType GetCmdBaseType(U8 obj, U8 cmd)
{
	BaseType type;
	const CmdLookupTable_t* entryPtr = LookupCmdEntry(obj, cmd);

	if (entryPtr != nullptr)
	{
		if ((cmd & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
		{
			type = entryPtr->commandType;
		}
		else
		{
			type = entryPtr->responseType;
		}
	}
	else
	{
		type = BaseType::Numeric;
	}

	return type;
}

static const AttrLookupTable_t* FindAttrEntryInTable(
	U16 inst, U8 attr,
	const AttrLookupTable_t* obj_names, U8 num_obj_names,
	const AttrLookupTable_t* inst_names, U8 num_inst_names)
{
	const AttrLookupTable_t* tablePtr = nullptr;
	const AttrLookupTable_t* entryPtr = nullptr;
	U8 numAttrs = 0;

	if (inst == ABP_INST_OBJ)
	{
		if (attr <= asObjAttrNames[NUM_ENTRIES(asObjAttrNames) - 1].value)
		{
			tablePtr = &asObjAttrNames[0];
			numAttrs = NUM_ENTRIES(asObjAttrNames);
		}
		else
		{
			if (obj_names != nullptr)
			{
				tablePtr = obj_names;
				numAttrs = num_obj_names;
			}
		}
	}
	else
	{
		if (num_inst_names != 0)
		{
			tablePtr = inst_names;
			numAttrs = num_inst_names;
		}
	}

	if (tablePtr != nullptr)
	{
		for (U8 i = 0; i < numAttrs; i++)
		{
			if (tablePtr[i].value == attr)
			{
				entryPtr = &tablePtr[i];
				break;
			}
		}
	}

	return entryPtr;
}

static const AttrLookupTable_t* LookupAttrEntry(U8 obj, U16 inst, U8 attr)
{
	const AttrLookupTable_t* entryPtr = nullptr;

	for (U8 i = 0; i < sizeof(asAttributeNameTables) / sizeof(AttributeNameTable_t); i++)
	{
		if (asAttributeNameTables[i].object_num == obj)
		{
			entryPtr = FindAttrEntryInTable(
				inst,
				attr,
				asAttributeNameTables[i].obj_names,
				asAttributeNameTables[i].num_obj_names,
				asAttributeNameTables[i].inst_names,
				asAttributeNameTables[i].num_inst_names);

			break;
		}
	}

	return entryPtr;
}

static const CmdLookupTable_t* FindCmdEntryInTable(U8 cmd, const CmdLookupTable_t* command_names, U8 num_commands)
{
	const CmdLookupTable_t* entryPtr = nullptr;

	for (U8 i = 0; i < num_commands; i++)
	{
		if (command_names[i].value == cmd)
		{
			entryPtr = &command_names[i];
			break;
		}
	}

	return entryPtr;
}

static const CmdLookupTable_t* LookupCmdEntry(U8 obj, U8 cmd)
{
	const CmdLookupTable_t* entryPtr = nullptr;

	cmd &= ABP_MSG_HEADER_CMD_BITS;

	if (IS_CMD_STANDARD(cmd))
	{
		for (U8 i = 0; i < NUM_ENTRIES(asCmdNames); i++)
		{
			if (asCmdNames[i].value == cmd)
			{
				entryPtr = &asCmdNames[i];
				break;
			}
		}
	}
	else if (IS_CMD_OBJECT_SPECIFIC(cmd))
	{
		for (U8 i = 0; i < sizeof(asCommandNameTables) / sizeof(CommandNameTable_t); i++)
		{
			if (asCommandNameTables[i].object_num == obj)
			{
				entryPtr = FindCmdEntryInTable(
					cmd,
					asCommandNameTables[i].cmd_names,
					asCommandNameTables[i].num_cmd_names);

				break;
			}
		}
	}

	return entryPtr;
}

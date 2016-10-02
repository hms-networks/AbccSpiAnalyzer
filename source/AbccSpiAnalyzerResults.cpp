/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerResults.cpp
**    Summary: DLL-Results source
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include "AbccSpiAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "AbccSpiAnalyzer.h"
#include "AbccSpiAnalyzerSettings.h"
#include <iostream>
#include <sstream>

#include "abcc_td.h"
#include "abcc_abp/abp.h"
#include "abcc_abp/abp_fsi.h"
#include "abcc_abp/abp_asm.h"
#include "abcc_abp/abp_cpc.h"
#include "abcc_abp/abp_cipid.h"
#include "abcc_abp/abp_fusm.h"
#include "abcc_abp/abp_mdd.h"
#include "abcc_abp/abp_sync.h"
#include "abcc_abp/abp_soc.h"
#include "abcc_abp/abp_smtp.h"
#include "abcc_abp/abp_safe.h"

#include "abcc_abp/abp_etn.h"
#include "abcc_abp/abp_epl.h"
#include "abcc_abp/abp_eip.h"
#include "abcc_abp/abp_ect.h"
#include "abcc_abp/abp_mod.h"
#include "abcc_abp/abp_nwetn.h"
#include "abcc_abp/abp_nwpnio.h"
#include "abcc_abp/abp_pnio.h"

#define IS_MISO_FRAME(frame) 			((frame.mFlags & SPI_MOSI_FLAG)!=SPI_MOSI_FLAG)
#define IS_MOSI_FRAME(frame)			((frame.mFlags & SPI_MOSI_FLAG)==SPI_MOSI_FLAG)
#define IS_CMD_STANDARD(cmd)			(cmd > 0) && (cmd < 9)
#define IS_CMD_OBJECT_SPECIFIC(cmd) 	(((cmd >= 0x10) && (cmd <= 0x30)) || \
										(cmd == 0x3F))

extern tAbccMosiInfo asMosiStates[];
extern tAbccMisoInfo asMisoStates[];
extern tAbccMsgInfo asMsgStates[];

tValueName asAnbInstAttrNames[] =
{
	{ ABP_ANB_IA_MODULE_TYPE,		"Module Type",				false },
	{ ABP_ANB_IA_FW_VERSION,		"Firmware Version",			false },
	{ ABP_ANB_IA_SERIAL_NUM,		"Serial Number",			false },
	{ ABP_ANB_IA_WD_TIMEOUT,		"Watchdog Timeout",			false },
	{ ABP_ANB_IA_SETUP_COMPLETE,	"Setup Complete",			false },
	{ ABP_ANB_IA_EXCEPTION,			"Exception",				false },
	{ ABP_ANB_IA_FATAL_EVENT,		"Fatal Event",				false },
	{ ABP_ANB_IA_ERROR_CNTRS,		"Error Counters",			false },
	{ ABP_ANB_IA_LANG,				"Language",					false },
	{ ABP_ANB_IA_PROVIDER_ID,		"Provider ID",				false },
	{ ABP_ANB_IA_PROVIDER_INFO,		"Provider Info",			false },
	{ ABP_ANB_IA_LED_COLOURS,		"LED Colors",				false },
	{ ABP_ANB_IA_LED_STATUS,		"LED Status",				false },
	{ ABP_ANB_IA_SWITCH_STATUS,		"Switch Status",			false },
	{ ABP_ANB_IA_AUX_BIT_FUNC,		"Auxilery Bit Function",	false },
	{ ABP_ANB_IA_GPIO_CONFIG,		"GPIO Configuration",		false },
	{ ABP_ANB_IA_VIRTUAL_ATTRS,		"Virtual Attributes",		false },
	{ ABP_ANB_IA_BLACK_WHITE_LIST,	"Black/White List",			false },
	{ ABP_ANB_IA_NETWORK_TIME,		"Network Time",				false },
	{ ABP_ANB_IA_FW_CUST_VERSION,	"FW Custom Version",		false },
	{ ABP_ANB_IA_ABIP_LICENSE,		"Anybus IP License",		false }
};

tValueName asDiObjAttrNames[] =
{
	{ ABP_DI_OA_MAX_INST,		"Maximum Number of Instances",	false },
	{ ABP_DI_OA_SUPPORT_FUNC,	"Supported Functionality",		false }
};

tValueName asDiInstAttrNames[] =
{
	{ ABP_DI_IA_SEVERITY,			"Severity",						false },
	{ ABP_DI_IA_EVENT_CODE,			"Event Code",					false },
	{ ABP_DI_IA_NW_SPEC_EVENT_INFO,	"Network Specific Event Info",	false },
	{ ABP_DI_IA_SLOT,				"Slot",							false },
	{ ABP_DI_IA_ADI,				"ADI",							false },
	{ ABP_DI_IA_ELEMENT,			"Element",						false },
	{ ABP_DI_IA_BIT,				"Bit",							false }
};

tValueName asNwInstAttrNames[] =
{
	{ ABP_NW_IA_NW_TYPE,		"Network Type",				false },
	{ ABP_NW_IA_NW_TYPE_STR,	"Network Type String",		false },
	{ ABP_NW_IA_DATA_FORMAT,	"Data Format",		    	false },
	{ ABP_NW_IA_PARAM_SUPPORT,	"Parameter Support",		false },
	{ ABP_NW_IA_WRITE_PD_SIZE,	"Write Process Data Size",	false },
	{ ABP_NW_IA_READ_PD_SIZE,	"Read Process Data Size",	false },
	{ ABP_NW_IA_EXCEPTION_INFO,	"Exception Information",	false }
};

tValueName asSocObjAttrNames[] =
{
	{ ABP_SOC_OA_MAX_INST, "Maximum Number of Instances",	false }
};

tValueName asSocInstAttrNames[] =
{
	{ ABP_SOC_IA_SOCK_TYPE,			"Socket Type",			false },
	{ ABP_SOC_IA_LOCAL_PORT,		"Local Port",			false },
	{ ABP_SOC_IA_HOST_IP,			"Host IP Address",		false },
	{ ABP_SOC_IA_HOST_PORT,			"Host Port",			false },
	{ ABP_SOC_IA_TCP_STATE,			"TCP State",			false },
	{ ABP_SOC_IA_RX_BYTES,			"Bytes in RX Buffer",	false },
	{ ABP_SOC_IA_TX_BYTES,			"Bytes in TX Buffer",	false },
	{ ABP_SOC_IA_SO_REUSE_ADDR,		"Reuse Address Option",	false },
	{ ABP_SOC_IA_SO_KEEP_ALIVE,		"Keep Alive Option",	false },
	{ ABP_SOC_IA_IP_MULT_TTL,		"IP Multicast TTL",		false },
	{ ABP_SOC_IA_IP_MULT_LOOP,		"IP Multicast Loop",	false },
	{ ABP_SOC_IA_TCP_ACKDELAYTIME,	"TCP Ack Delay Time",	false },
	{ ABP_SOC_IA_TCP_NODELAY,		"TCP No Delay",			false },
	{ ABP_SOC_IA_TCP_CONNTIMEO,		"TCP Connect Timeout",	false }
};

tValueName asSmtpObjAttrNames[] =
{
	{ ABP_SMTP_OA_MAX_INST,		"Maximum Number of Instances",	false },
	{ ABP_SMTP_OA_EMAILS_SENT,	"Emails Sent",					false },
	{ ABP_SMTP_OA_EMAIL_FAILED,	"Emails Failed to Send",		false }
};

tValueName asSmtpInstAttrNames[] =
{
	{ ABP_SMTP_IA_FROM,		"From Address",		false },
	{ ABP_SMTP_IA_TO,		"To Address",		false },
	{ ABP_SMTP_IA_SUBJECT,	"Message Subject",  false },
	{ ABP_SMTP_IA_MESSAGE,	"Message Body",		false }
};

tValueName asNcInstAttrNames[] =
{
	{ ABP_NC_VAR_IA_NAME,			"Name",					false },
	{ ABP_NC_VAR_IA_DATA_TYPE,		"Data Type",			false },
	{ ABP_NC_VAR_IA_NUM_ELEM,		"Number of Elements",	false },
	{ ABP_NC_VAR_IA_DESCRIPTOR,		"Descriptor",			false },
	{ ABP_NC_VAR_IA_VALUE,			"Value",				false },
	{ ABP_NC_VAR_IA_CONFIG_VALUE,	"Configured Value",		false }
};

tValueName asNwEtnInstAttrNames[] =
{
	{ ABP_ETN_IA_MAC_ADDRESS,			"MAC Address",					false },
	{ ABP_ETN_IA_ENABLE_HICP,			"Enable HICP",					false },
	{ ABP_ETN_IA_ENABLE_WEB,			"Enable Web Server",			false },
	{ ABP_ETN_IA_ENABLE_MOD_TCP ,		"Enable Modbus TCP",			false },
	{ ABP_ETN_IA_ENABLE_WEB_ADI_ACCESS,	"Enable Web ADI Access",		false },
	{ ABP_ETN_IA_ENABLE_FTP,			"Enable FTP Server",			false },
	{ ABP_ETN_IA_ENABLE_ADMIN_MODE,		"Enable Admin Mode",			false },
	{ ABP_ETN_IA_NETWORK_STATUS,		"Network Status",				false },
	{ ABP_ETN_IA_PORT1_MAC_ADDRESS,		"Port 1 MAC Address",			false },
	{ ABP_ETN_IA_PORT2_MAC_ADDRESS,		"Port 2 MAC Address",			false },
	{ ABP_ETN_IA_ENABLE_ACD,			"Enable ACD",					false },
	{ ABP_ETN_IA_PORT1_STATE,			"Port 1 State",					false },
	{ ABP_ETN_IA_PORT2_STATE,			"Port 2 State",					false },
	{ ABP_ETN_IA_ENABLE_WEB_UPDATE,		"Enable Web Update",			false },
	{ ABP_ETN_IA_ENABLE_HICP_RESET,		"Enable Reset From HICP",		false },
	{ ABP_ETN_IA_IP_CONFIGURATION,		"IP Configuration",				false },
	{ ABP_ETN_IA_IP_ADDRESS_BYTE_0_2,	"IP Address Byte 0-2",			false },
	{ ABP_ETN_IA_ETH_PHY_CONFIG,		"PHY Duplex Fallback Config",	false }
};

tValueName asCpcObjAttrNames[] =
{
	{ ABP_CPC_OA_MAX_INST, "Maximum Number of Instances",	false }
};

tValueName asCpcInstAttrNames[] =
{
	{ ABP_CPC_IA_PORT_TYPE,			"Port Type",		false },
	{ ABP_CPC_IA_PORT_NUMBER,		"Port Number",		false },
	{ ABP_CPC_IA_LINK_PATH,			"Link Path",		false },
	{ ABP_CPC_IA_PORT_NAME,			"Port Name",		false },
	{ ABP_CPC_IA_NODE_ADDRESS,		"Node Address",		false },
	{ ABP_CPC_IA_PORT_NODE_RANGE,	"Port Node Range",	false }
};

tValueName asCipIdInstAttrNames[] =
{
	{ ABP_CIPID_IA_VENDOR_ID,		"Vendor ID",		false },
	{ ABP_CIPID_IA_DEVICE_TYPE,		"Device Type",		false },
	{ ABP_CIPID_IA_PRODUCT_CODE,	"Product Code",		false },
	{ ABP_CIPID_IA_REVISION,		"Revision",			false },
	{ ABP_CIPID_IA_STATUS,			"Status",			false },
	{ ABP_CIPID_IA_SERIAL_NUMBER,	"Serial Number",	false },
	{ ABP_CIPID_IA_PRODUCT_NAME,	"Product Name",		false }
};

tValueName asEplInstAttrNames[] =
{
	{ ABP_EPL_IA_VENDOR_ID,		"Vendor ID",						false },
	{ ABP_EPL_IA_PRODUCT_CODE,	"Product Code",						false },
	{ ABP_EPL_IA_MAJOR_REV,		"Revision High Word",				false },
	{ ABP_EPL_IA_MINOR_REV,		"Revision Low Word",				false },
	{ ABP_EPL_IA_SERIAL_NUMBER,	"Serial Number",					false },
	{ ABP_EPL_IA_MANF_DEV_NAME,	"Manufacturer Device Name",			false },
	{ ABP_EPL_IA_MANF_HW_VER,	"Manufacturer Hardware Verison",	false },
	{ ABP_EPL_IA_MANF_SW_VER,	"Manufacturer Software Version",	false },
	{ ABP_EPL_IA_DEVICE_TYPE,	"Device Type",						false },
	{ ABP_EPL_IA_MANF_NAME,		"Manufacturer Name",				false }
};

tValueName asPnioInstAttrNames[] =
{
	{ ABP_PNIO_IA_DEVICE_ID,				"Device ID",						false },
	{ ABP_PNIO_IA_VENDOR_ID,				"Vendor ID (I&M Manufacturer ID)",	false },
	{ ABP_PNIO_IA_STATION_TYPE,				"Station Type",						false },
	{ ABP_PNIO_IA_MAX_AR,					"MaxAr",							false },
	{ 0x05,									"Reserved",							true  },
	{ 0x06,									"Reserved",							true  },
	{ ABP_PNIO_IA_RTM,						"Record Data Mode",					false },
	{ ABP_PNIO_IA_IM_ORDER_ID,				"I&M Order ID",						false },
	{ ABP_PNIO_IA_IM_SERIAL_NBR,			"I&M Serial Number",				false },
	{ ABP_PNIO_IA_IM_HW_REV,				"I&M Hardware Revision",			false },
	{ ABP_PNIO_IA_IM_SW_REV,				"I&M Software Revision",			false },
	{ ABP_PNIO_IA_IM_REV_CNT,				"I&M Revision Counter",				false },
	{ ABP_PNIO_IA_IM_PROFILE_ID,			"I&M Profile ID",					false },
	{ ABP_PNIO_IA_IM_PROFILE_SPEC_TYPE,		"I&M Profile Specific Type",		false },
	{ ABP_PNIO_IA_IM_VER,					"I&M Version",						false },
	{ ABP_PNIO_IA_IM_SUPPORTED,				"I&M Supported",					false },
	{ ABP_PNIO_IA_PORT1_MAC_ADDRESS,		"Port 1 MAC Address",				false },
	{ ABP_PNIO_IA_PORT2_MAC_ADDRESS,		"Port 2 MAC Address",				false },
	{ ABP_PNIO_IA_SYSTEM_DESCRIPTION,		"System Description",				false },
	{ ABP_PNIO_IA_INTERFACE_DESCRIPTION,	"Interface Description",			false },
	{ ABP_PNIO_IA_MOD_ID_ASSIGN_MODE,		"Module Id Assignment Mode",		false },
	{ ABP_PNIO_IA_SYSTEM_CONTACT,			"System Contact",					false },
	{ ABP_PNIO_IA_PROFIENERGY_FUNC,			"PROFIenergy Functionality",		false },
	{ 0x18,									"Custom Station Name",				false } /* TODO: ABP macro does not exist yet */
};

tValueName asEipInstAttrNames[] =
{
	{ ABP_EIP_IA_VENDOR_ID,						"Vendor ID",										false },
	{ ABP_EIP_IA_DEVICE_TYPE,					"Device Type",										false },
	{ ABP_EIP_IA_PRODUCT_CODE,					"Product Code",										false },
	{ ABP_EIP_IA_REVISION,						"Revision",											false },
	{ ABP_EIP_IA_SERIAL_NUMBER,					"Serial Number",									false },
	{ ABP_EIP_IA_PRODUCT_NAME,					"Product Name",										false },
	{ ABP_EIP_IA_PROD_INSTANCE,					"Producing Instance Number",						false },
	{ ABP_EIP_IA_CONS_INSTANCE,					"Consuming Instance Number",						false },
	{ ABP_EIP_IA_COMM_SETTINGS_FROM_NET,		"Enable Communication Settings From Net",			false },
	{ ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS,		"Enable CIP Forwarding",							false },
	{ ABP_EIP_IA_ENABLE_PARAM_OBJECT,			"Enable Parameter Object",							false },
	{ ABP_EIP_IA_INPUT_INSTANCE_OBJECT,			"Input-Only Heartbeat Instance Number",				false },
	{ ABP_EIP_IA_LISTEN_INSTANCE_OBJECT,		"Listen-Only Heartbeat Instance Number",			false },
	{ ABP_EIP_IA_CONFIG_INSTANCE,				"Assembly Object Configuration Instance Number",	false },
	{ ABP_EIP_IA_DISABLE_STRICT_IO_MATCH,		"Disable Strict I/O Match",							false },
	{ ABP_EIP_IA_ENABLE_UNCONNECTED_SEND,		"Enable Unconnected Routing",						false },
	{ ABP_EIP_IA_INPUT_EXT_INSTANCE_OBJECT,		"Input-Only Extended Heartbeat Instance Number",	false },
	{ ABP_EIP_IA_LISTEN_EXT_INSTANCE_OBJECT,	"Listen-Only Extended Heartbeat Instance Number",	false },
	{ ABP_EIP_IA_IF_LABEL_PORT_1,				"Interface Label Port 1",							false },
	{ ABP_EIP_IA_IF_LABEL_PORT_2,				"Interface Label Port 2",							false },
	{ ABP_EIP_IA_IF_LABEL_PORT_INT,				"Interface Label Internal Port",					false },
	{ ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS_EXT,	"Enable Application CIP Object Extended",			false },
	{ ABP_EIP_IA_PREPEND_PRODUCING,				"Prepend Producing",								false },
	{ ABP_EIP_IA_PREPEND_CONSUMING,				"Prepend Consuming",								false },
	{ ABP_EIP_IA_ENABLE_EIP_QC,					"Enable EtherNet/IP QuickConnect",					false },
	{ ABP_EIP_IA_PROD_INSTANCE_MAP,				"Producing Instance Mapping",						false },
	{ ABP_EIP_IA_CONS_INSTANCE_MAP,				"Consuming Instance Mapping",						false },
	{ ABP_EIP_IA_IGNORE_SEQ_COUNT_CHECK,		"Ignore Sequence Count Check",						false },
	{ ABP_EIP_IA_ABCC_ADI_OBJECT,				"ABCC ADI Object Number",							false },
	{ 0x1F,										"Enable DLR",										false } /* TODO: ABP macro does not exist yet */
};

tValueName asEtcInstAttrNames[] =
{
	{ ABP_ECT_IA_VENDOR_ID,				"Vendor ID",								false },
	{ ABP_ECT_IA_PRODUCT_CODE,			"Product Code",								false },
	{ ABP_ECT_IA_MAJOR_REV,				"Major Revision",							false },
	{ ABP_ECT_IA_MINOR_REV,				"Minor Revision",							false },
	{ ABP_ECT_IA_SERIAL_NUMBER,			"Serial Number",							false },
	{ ABP_ECT_IA_MANF_DEV_NAME,			"Manufacturer Device Name",					false },
	{ ABP_ECT_IA_MANF_HW_VER,			"Manufacturer Hardware Version",			false },
	{ ABP_ECT_IA_MANF_SW_VER,			"Manufacturer Software Version",			false },
	{ ABP_ECT_IA_ENUM_ADIS,				"ENUM ADIs",								false },
	{ ABP_ECT_IA_DEVICE_TYPE,			"Device Type",								false },
	{ ABP_ECT_IA_WR_PD_ASSY_INST_TRANS,	"Write PD Assembly Instance Translation",	false },
	{ ABP_ECT_IA_RD_PD_ASSY_INST_TRANS,	"Read PD Assembly Instance Translation",	false },
	{ ABP_ECT_IA_ADI_TRANS,				"ADI Translation",							false },
	{ ABP_ECT_IA_OBJ_SUB_TRANS,			"Object SubIndex Translation",				false },
	{ ABP_ECT_IA_ENABLE_FOE,			"Enable FoE",								false },
	{ ABP_ECT_IA_ENABLE_EOE,			"Enable EoE",								false },
	{ ABP_ECT_IA_CHANGE_SR_SWITCH,		"Change Shift Register Switch",				false },
	{ ABP_ECT_IA_SET_DEV_ID_AS_CSA,		"Set Device ID as Configured Station Alias",false }
};

tValueName asAppdObjAttrNames[] =
{
	{ ABP_APPD_OA_NR_READ_PD_MAPPABLE_INSTANCES,	"No. of RD PD Mappable Instances",	false },
	{ ABP_APPD_OA_NR_WRITE_PD_MAPPABLE_INSTANCES,	"No. of WR PD Mappable Instances",	false }
};

tValueName asAppdInstAttrNames[] =
{
	{ ABP_APPD_IA_NAME,			"Name",						false },
	{ ABP_APPD_IA_DATA_TYPE,	"Data Type",				false },
	{ ABP_APPD_IA_NUM_ELEM,		"Number of Elements",		false },
	{ ABP_APPD_IA_DESCRIPTOR,	"Descriptor",				false },
	{ ABP_APPD_IA_VALUE,		"Value(s)",					false },
	{ ABP_APPD_IA_MAX_VALUE,	"Max Value",				false },
	{ ABP_APPD_IA_MIN_VALUE,	"Min Value",				false },
	{ ABP_APPD_IA_DFLT_VALUE,	"Default Value",			false },
	{ ABP_APPD_IA_NUM_SUB_ELEM,	"Number of SubElements",	false },
	{ ABP_APPD_IA_ELEM_NAME,	"Element Name",				false }
};

tValueName asAppInstAttrNames[] =
{
	{ ABP_APP_IA_CONFIGURED,	"Configured",						false },
	{ ABP_APP_IA_SUP_LANG,		"Supported Languages",				false },
	{ ABP_APP_IA_SER_NUM,		"Serial Number",					false },
	{ ABP_APP_IA_PAR_CRTL_SUM,	"Parameter Control Sum",			false },
	{ ABP_APP_IA_FW_AVAILABLE,	"Candidate Firmware Available",		false },
	{ ABP_APP_IA_HW_CONF_ADDR,	"Hardware Configurable Address",	false }
};

tValueName asFsiObjAttrNames[] =
{
	{ ABP_FSI_OA_MAX_INST,						"Max Number of Instances",		false },
	{ ABP_FSI_OA_DISABLE_VFS,					"Disable Virtual File System",	false },
	{ ABP_FSI_OA_TOTAL_DISC_SIZE,				"Total Disc Size",				false },
	{ ABP_FSI_OA_FREE_DISC_SIZE,				"Free Disc Size",				false },
	{ ABP_FSI_OA_DISC_CRC,						"Disc CRC",						false },
	{ ABP_FSI_OA_DISC_TYPE,						"Disc Type",					false },
	{ ABP_FSI_OA_DISC_FAULT_TOLERANCE_LEVEL,	"Disc Fault Tolerance Level",	false }
};

tValueName asFsiInstAttrNames[] =
{
	{ ABP_FSI_IA_TYPE,		"Instance Type",			false },
	{ ABP_FSI_IA_FILE_SIZE,	"File Size",				false },
	{ ABP_FSI_IA_PATH,		"Current Instance Path",	false }
};

tValueName asAsmObjAttrNames[] =
{
	{ ABP_ASM_OA_WRITE_PD_INST_LIST,	"Write PD Instance List",	false },
	{ ABP_ASM_OA_READ_PD_INST_LIST,		"Read PD Instance List",	false }
};

tValueName asAsmInstAttrNames[] =
{
	{ ABP_ASM_IA_DESCRIPTOR,		"Assembly Descriptor",	false },
	{ ABP_ASM_IA_ADI_MAP_XX + 0,	"ADI Map 0",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 1,	"ADI Map 1",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 2,	"ADI Map 2",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 3,	"ADI Map 3",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 4,	"ADI Map 4",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 5,	"ADI Map 5",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 6,	"ADI Map 6",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 7,	"ADI Map 7",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 8,	"ADI Map 8",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 9,	"ADI Map 9",			false },
	{ ABP_ASM_IA_ADI_MAP_XX + 10,	"ADI Map 10",			false }
};

tValueName asMddObjAttrNames[] =
{
	{ ABP_MDD_OA_NUM_SLOTS,			"Number of Slots",			false },
	{ ABP_MDD_OA_NUM_ADIS_PER_SLOT,	"Number of ADIs Per Slot",	false }
};

tValueName asSyncInstAttrNames[] =
{
	{ ABP_SYNC_IA_CYCLE_TIME,			"Cycle Time",				false },
	{ ABP_SYNC_IA_OUTPUT_VALID,			"Output Valid",				false },
	{ ABP_SYNC_IA_INPUT_CAPTURE,		"Input Capture",			false },
	{ ABP_SYNC_IA_OUTPUT_PROCESSING,	"Output Processing Time",	false },
	{ ABP_SYNC_IA_INPUT_PROCESSING,		"Input Processing Time",	false },
	{ ABP_SYNC_IA_MIN_CYCLE_TIME,		"Minimum Cycle Time",		false },
	{ ABP_SYNC_IA_SYNC_MODE,			"Sync Mode",				false },
	{ ABP_SYNC_IA_SUPPORTED_SYNC_MODES,	"Supported Sync Modes",		false }
};

tValueName asFusmInstAttrNames[] =
{
	{ ABP_FUSM_IA_STATE,		"State",					false },
	{ ABP_FUSM_IA_VENDOR_ID,	"Vendor ID",				false },
	{ ABP_FUSM_IA_MODULE_ID,	"I/O Channel ID",			false },
	{ ABP_FUSM_IA_FW_VERSION,	"Firmware Version",			false },
	{ ABP_FUSM_IA_SERIAL_NUM,	"Serial Number",			false },
	{ ABP_FUSM_IA_DATA_OUT,		"Output Data",				false },
	{ ABP_FUSM_IA_DATA_IN,		"Input Data",				false },
	{ ABP_FUSM_IA_ERROR_CNTRS,	"Error Counters",			false },
	{ ABP_FUSM_IA_FATAL_EVENT,	"Event Log",				false },
	{ ABP_FUSM_IA_EXCPT_INFO,	"Exception Information",	false },
	{ ABP_FUSM_IA_BL_VERSION,	"Bootloader Version",		false }
};

tValueName asSafeInstAttrNames[] =
{
	{ ABP_SAFE_IA_SAFETY_ENABLE,	"Safety Enabled",		false },
	{ ABP_SAFE_IA_BAUD_RATE,		"Baud Rate",			false },
	{ 0x03,							"I/O Configuration",	false } /* TODO: ABP macro does not exist yet */
};


tValueName asObjAttrNames[] =
{
	{ ABP_OA_NAME,			"Name",						false },
	{ ABP_OA_REV,			"Revision",					false },
	{ ABP_OA_NUM_INST,		"Number of Instances",		false },
	{ ABP_OA_HIGHEST_INST,	"Highest Instance Number",	false }
};

tValueName asObjectNames[] =
{
	/*------------------------------------------------------------------------------
	** Anybus module objects
	**------------------------------------------------------------------------------
	*/
	{ ABP_OBJ_NUM_ANB,		"Anybus Object",						false },
	{ ABP_OBJ_NUM_DI,		"Diagnositic Object",					false },
	{ ABP_OBJ_NUM_NW,		"Network Object",						false },
	{ ABP_OBJ_NUM_NC,		"Network Configuration Object",			false },
	{ ABP_OBJ_NUM_ADD,		"PROFIBUS DP-V1 Additional Diag",		false },
	{ ABP_OBJ_NUM_RSV1,		"Reserved",								true  },
	{ ABP_OBJ_NUM_SOC,		"Socket Interface Object",				false },
	{ ABP_OBJ_NUM_NWCCL,	"Network CC-Link Object",				false },
	{ ABP_OBJ_NUM_SMTP,		"SMTP Client Object",					false },
	{ ABP_OBJ_NUM_FSI,		"Anybus File System Interface Object",	false },
	{ ABP_OBJ_NUM_NWDPV1,	"Network PROFIBUS DP-V1 Object",		false },
	{ ABP_OBJ_NUM_NWETN,	"Network Ethernet Object",				false },
	{ ABP_OBJ_NUM_CPC,		"CIP Port Configuration Object",		false },
	{ ABP_OBJ_NUM_NWPNIO,	"Network PROFINET IO Object",			false },
	{ ABP_OBJ_NUM_PNIOADD,	"PROFINET IO Additional Diag Object",	false },
	{ ABP_OBJ_NUM_DPV0DI,	"PROFIBUS DP-V0 Diagnostic Object",		false },
	{ ABP_OBJ_NUM_FUSM,		"Functional Safety Module Object",		false },
	{ ABP_OBJ_NUM_NWCFN,	"Network CC-Link IE Field Network",		false },
	/*------------------------------------------------------------------------------
	** Host application objects
	**------------------------------------------------------------------------------
	*/
	{ ABP_OBJ_NUM_ER,		"Energy Reporting Object",						false },
	{ ABP_OBJ_NUM_SAFE,		"Functional Safety Object",						false },
	{ ABP_OBJ_NUM_EPL,		"POWERLINK Object",								false },
	{ ABP_OBJ_NUM_AFSI,		"Application File System Interface Object",		false },
	{ ABP_OBJ_NUM_ASM,		"Assembly Object",								false },
	{ ABP_OBJ_NUM_MDD,		"Modular Device Object",						false },
	{ ABP_OBJ_NUM_CIPID,	"CIP Identity Host Object",						false },
	{ ABP_OBJ_NUM_SYNC,		"Sync Object",									false },
	{ ABP_OBJ_NUM_BAC,		"BACnet Object",								false },
	{ ABP_OBJ_NUM_ECO,		"Energy Control Object",						false },
	{ ABP_OBJ_NUM_SRC3,		"SERCOS III Object",							false },
	{ ABP_OBJ_NUM_PRD,		"PROFIdrive Object",							false },
	{ ABP_OBJ_NUM_CNT,		"ControlNet Object",							false },
	{ ABP_OBJ_NUM_CPN,		"CompoNet Object",								false },
	{ ABP_OBJ_NUM_ECT,		"EtherCAT Object",								false },
	{ ABP_OBJ_NUM_PNIO,		"PROFINET IO Object",							false },
	{ ABP_OBJ_NUM_CCL,		"CC-Link Host Object",							false },
	{ ABP_OBJ_NUM_EIP,		"EtherNet/IP Host Object",						false },
	{ ABP_OBJ_NUM_ETN,		"Ethernet Host Object",							false },
	{ ABP_OBJ_NUM_MOD,		"Modbus Host Object",							false },
	{ ABP_OBJ_NUM_COP,		"CANopen Object",								false },
	{ ABP_OBJ_NUM_DEV,		"DeviceNet Host Object",						false },
	{ ABP_OBJ_NUM_DPV1,		"PROFIBUS DP-V1 Object",						false },
	{ ABP_OBJ_NUM_APPD,		"Application Data Object",						false },
	{ ABP_OBJ_NUM_APP,		"Application Object",							false }
};

tValueName asCmdNames[] =
{
	{ ABP_CMD_GET_ATTR,			"Get_Attribute",			false },
	{ ABP_CMD_SET_ATTR,			"Set_Attribute",			false },
	{ ABP_CMD_CREATE,			"Create",					false },
	{ ABP_CMD_DELETE,			"Delete",					false },
	{ ABP_CMD_RESET,			"Reset",					false },
	{ ABP_CMD_GET_ENUM_STR,		"Get_Enum_String",			false },
	{ ABP_CMD_GET_INDEXED_ATTR,	"Get_Indexed_Attribute",	false },
	{ ABP_CMD_SET_INDEXED_ATTR,	"Set_Indexed_Attribute",	false }
};


tValueName asMddCmdNames[] =
{
	{ ABP_MDD_CMD_GET_LIST, "Get_List",	false },
};

tValueName asAsmCmdNames[] =
{
	{ ABP_ASM_CMD_WRITE_ASSEMBLY_DATA,	"Write_Assembly_Data",	false },
	{ ABP_ASM_CMD_READ_ASSEMBLY_DATA,	"Read_Assembly_Data",	false }
};

tValueName asFusmCmdNames[] =
{
	{ ABP_FUSM_CMD_ERROR_CONFIRMATION,	"Error_Confirmation",	true }
};

tValueName asFsiCmdNames[] =
{
	{ ABP_FSI_CMD_FILE_OPEN,			"File_Open",		false },
	{ ABP_FSI_CMD_FILE_CLOSE,			"File_Close",		false },
	{ ABP_FSI_CMD_FILE_DELETE,			"File_Delete",		false },
	{ ABP_FSI_CMD_FILE_COPY,			"File_Copy",		false },
	{ ABP_FSI_CMD_FILE_RENAME,			"File_Rename",		false },
	{ ABP_FSI_CMD_FILE_READ,			"File_Read",		false },
	{ ABP_FSI_CMD_FILE_WRITE,			"File_Write",		false },
	{ ABP_FSI_CMD_DIRECTORY_OPEN,		"Directory_Open",	false },
	{ ABP_FSI_CMD_DIRECTORY_CLOSE,		"Directory_Close",	false },
	{ ABP_FSI_CMD_DIRECTORY_DELETE,		"Directory_Delete",	false },
	{ ABP_FSI_CMD_DIRECTORY_READ,		"Directory_Read",	false },
	{ ABP_FSI_CMD_DIRECTORY_CREATE,		"Directory_Create",	false },
	{ ABP_FSI_CMD_DIRECTORY_CHANGE,		"Directory_Change",	false },
	{ ABP_FSI_CMD_FORMAT_DISC,			"Format_Disc",		false }
};

tValueName asEipCmdNames[] =
{
	{ ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST,		"Process_CIP_Obj_Request",		false },
	{ ABP_EIP_CMD_SET_CONFIG_DATA,				"Set_Config_Data",				false },
	{ ABP_EIP_CMD_PROCESS_CIP_ROUTING_REQUEST,	"Process_CIP_Routing_Request",	false },
	{ ABP_EIP_CMD_GET_CONFIG_DATA,				"Get_Config_Data",				false },
	{ ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST_EXT,	"Process_CIP_Obj_Request_Ext",	false }
};

tValueName asPnioCmdNames[] =
{
	{ ABP_PNIO_CMD_GET_RECORD,			"Get_Record",			false },
	{ ABP_PNIO_CMD_SET_RECORD,			"Set_Record",			false },
	{ ABP_PNIO_CMD_GET_IM_RECORD,		"Get_IM_Record",		false },
	{ ABP_PNIO_CMD_SET_IM_RECORD,		"Set_IM_Record",		false },
	{ ABP_PNIO_CMD_AR_CHECK_IND,		"AR_Check_Ind",			false },
	{ ABP_PNIO_CMD_CFG_MISMATCH_IND,	"Cfg_Mismatch_Ind",		true  }, //TODO check if alert is appropriate here
	{ ABP_PNIO_CMD_AR_INFO_IND,			"AR_Info_Ind",			false },
	{ ABP_PNIO_CMD_END_OF_PRM_IND,		"End_Of_Prm_Ind",		false },
	{ ABP_PNIO_CMD_AR_ABORT_IND,		"AR_Abort_Ind",			true  }, //TODO check if alert is appropriate here
	{ ABP_PNIO_CMD_PLUG_SUB_FAILED,		"Plug_Sub_Failed",		true  }, //TODO check if alert is appropriate here
	{ ABP_PNIO_CMD_EXPECTED_IDENT_IND,	"Expected_Ident_Ind",	true  }, //TODO check if alert is appropriate here
	{ ABP_PNIO_CMD_SAVE_IP_SUITE,		"Save_IP_Suite",		false },
	{ ABP_PNIO_CMD_SAVE_STATION_NAME,	"Save_Station_Name",	false },
	{ 0x1E,								"Indicate_Device",		false } /* TODO: ABP macro does not exist yet */
};

tValueName asAppCmdNames[] =
{
	{ ABP_APP_CMD_RESET_REQUEST,		"Reset_Request",			false },
	{ ABP_APP_CMD_CHANGE_LANG_REQUEST,	"Change_Language_Request",	false },
	{ ABP_APP_CMD_RESET_DIAGNOSTIC,		"Reset_Diagnostic",			false }
};

tValueName asAppDataCmdNames[] =
{
	{ ABP_APPD_CMD_GET_INST_BY_ORDER,		"Get_Instance_Number_By_Order",	false },
	{ ABP_APPD_GET_PROFILE_INST_NUMBERS,	"Get_Profile_Inst_Numbers",		false },
	{ ABP_APPD_GET_ADI_INFO,				"Get_ADI_Info (Deprecated)",	true  }, /* ABCC40 deprecated shall not be used */
	{ ABP_APPD_REMAP_ADI_WRITE_AREA,		"Remap_ADI_Write_Area",			false },
	{ ABP_APPD_REMAP_ADI_READ_AREA,			"Remap_Adi_Read_Area",			false },
	{ ABP_APPD_GET_INSTANCE_NUMBERS,		"Get_Instance_Numbers",			false }
};

tValueName asNetCmdNames[] =
{
	{ ABP_NW_CMD_MAP_ADI_WRITE_AREA,		"Map_ADI_Write_Area",		false },
	{ ABP_NW_CMD_MAP_ADI_READ_AREA,			"Map_ADI_Read_Area",		false },
	{ ABP_NW_CMD_MAP_ADI_WRITE_EXT_AREA,	"Map_ADI_Write_Ext_Area",	false },
	{ ABP_NW_CMD_MAP_ADI_READ_EXT_AREA,		"Map_ADI_Read_Ext_Area",	false }
};

tValueName asIntMaskNames[] =
{
	{ ABP_INTMASK_RDPDIEN,		"RDPD",		false },
	{ ABP_INTMASK_RDMSGIEN,		"RDMSG",	false },
	{ ABP_INTMASK_WRMSGIEN,		"WRMSG",	false },
	{ ABP_INTMASK_ANBRIEN,		"ANBR",		false },
	{ ABP_INTMASK_STATUSIEN,	"STATUS",	false },
	{ 0x20,						"RESERVED",	true  }, /* No ABP mask exists */
	{ ABP_INTMASK_SYNCIEN,		"SYNC",		false },
	{ 0x80,						"RESERVED",	true  }  /* No ABP mask exists */
};

tValueName asLedStsNames[] =
{
	{ 0x0001, "LED1A",		false },
	{ 0x0002, "LED1B",		false },
	{ 0x0004, "LED2A",		false },
	{ 0x0008, "LED2B",		false },
	{ 0x0010, "LED3A",		false },
	{ 0x0020, "LED3B",		false },
	{ 0x0040, "LED4A",		false },
	{ 0x0080, "LED4B",		false },
	{ 0xFF00, "RESERVED",	true }
};

tValueName asErrorRspNames[] =
{
	{ ABP_ERR_INV_MSG_FORMAT,					"Invalid message format",							true },
	{ ABP_ERR_UNSUP_OBJ,						"Unsupported object",								true },
	{ ABP_ERR_UNSUP_INST,						"Unsupported instance",								true },
	{ ABP_ERR_UNSUP_CMD,						"Unsupported command",								true },
	{ ABP_ERR_INV_CMD_EXT_0,					"Invalid CmdExt0",									true },
	{ ABP_ERR_INV_CMD_EXT_1,					"Invalid CmdExt1",									true },
	{ ABP_ERR_ATTR_NOT_SETABLE,					"Attribute not settable",							true },
	{ ABP_ERR_ATTR_NOT_GETABLE,					"Attribute not gettable",							true },
	{ ABP_ERR_TOO_MUCH_DATA,					"Too much data",									true },
	{ ABP_ERR_NOT_ENOUGH_DATA,					"Not enough data",									true },
	{ ABP_ERR_OUT_OF_RANGE,						"Out of range",										true },
	{ ABP_ERR_INV_STATE,						"Invalid state",									true },
	{ ABP_ERR_NO_RESOURCES,						"Out of resources",									true },
	{ ABP_ERR_SEG_FAILURE,						"Segmentation failure",								true },
	{ ABP_ERR_SEG_BUF_OVERFLOW,					"Segmentation buffer overflow",						true },
	{ ABP_ERR_VAL_TOO_HIGH,						"Value too high",									true },
	{ ABP_ERR_VAL_TOO_LOW,						"Value too low",									true },
	{ ABP_ERR_CONTROLLED_FROM_OTHER_CHANNEL,	"NAK writes to \"read process data\" mapped attr.",	true },
	{ ABP_ERR_MSG_CHANNEL_TOO_SMALL,			"Response does not fit",							true },
	{ ABP_ERR_GENERAL_ERROR,					"General error",									true },
	{ 0xFF,										"Object specific error",							true }
};

tValueName asAnybusStsNames[] =
{
	{ ABP_ANB_STATE_SETUP,			"SETUP",			false },
	{ ABP_ANB_STATE_NW_INIT,		"NW_INIT",			false },
	{ ABP_ANB_STATE_WAIT_PROCESS,	"WAIT_PROCESS",		false },
	{ ABP_ANB_STATE_IDLE,			"IDLE",				false },
	{ ABP_ANB_STATE_PROCESS_ACTIVE, "PROCESS_ACTIVE",	false },
	{ ABP_ANB_STATE_ERROR,			"ERROR",			true  },
	{ ABP_ANB_STATE_EXCEPTION,		"EXCEPTION",		true  }
};

tValueName asApplStsNames[] =
{
	{ ABP_APPSTAT_NO_ERROR,			"No Error",									false },
	{ ABP_APPSTAT_NOT_SYNCED,		"Not yet synchronized",						false },
	{ ABP_APPSTAT_SYNC_CFG_ERR,		"Sync configuration error",					true  },
	{ ABP_APPSTAT_READ_PD_CFG_ERR,	"Read process data configuration error",	true  },
	{ ABP_APPSTAT_WRITE_PD_CFG_ERR,	"Write process data configuration error",	true  },
	{ ABP_APPSTAT_SYNC_LOSS,		"Synchronization loss",						true  },
	{ ABP_APPSTAT_PD_DATA_LOSS,		"Excessive data loss",						true  },
	{ ABP_APPSTAT_OUTPUT_ERR,		"Output error",								true  }
};

tValueName asSpiStsNames[] =
{
	{ 0xC0,							"RESERVED",		true  }, /* No ABP mask exists */
	{ ABP_SPI_STATUS_NEW_PD,		"NEW_PD",		false },
	{ ABP_SPI_STATUS_LAST_FRAG,		"LAST_FRAG",	false },
	{ ABP_SPI_STATUS_M,				"M",			false },
	{ ABP_SPI_STATUS_CMDCNT,		"CMDCNT",		false },
	{ ABP_SPI_STATUS_WRMSG_FULL,	"WRMSG_FULL",	true  }
};

tValueName asSpiCtrlNames[] =
{
	{ ABP_SPI_CTRL_T,			"TOGGLE",		false },
	{ 0x60,						"RESERVED",		true  }, /* No ABP mask exists */
	{ ABP_SPI_CTRL_LAST_FRAG,	"LAST_FRAG",	false },
	{ ABP_SPI_CTRL_M,			"M",			false },
	{ ABP_SPI_CTRL_CMDCNT,		"CMDCNT",		false },
	{ ABP_SPI_CTRL_WRPD_VALID,	"WRPD_VALID",	false }
};

SpiAnalyzerResults::SpiAnalyzerResults(SpiAnalyzer* analyzer, SpiAnalyzerSettings* settings)
	: AnalyzerResults(),
	mSettings(settings),
	mAnalyzer(analyzer)
{
}

SpiAnalyzerResults::~SpiAnalyzerResults()
{
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* verbose, bool alert)
{
	StringBuilder(tag, value, verbose, alert, false);
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* verbose, bool alert, bool prioritizeValue)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	U16 len2, len3;
	char pad[32] = "";
	const char alert_str[] = "!ALERT - ";
	bool apply_pad = false;

	if (verbose && value)
	{
		len2 = (U16)strlen(value);
		if(alert)
		{
			len2 += sizeof(alert_str);
		}
		len3 = (U16)strlen(verbose);
		if (len3 <= len2)
		{
			/* We must pad the level3 (bit states) text to maintain display priority */
			apply_pad = true;
			memset(pad, ' ', ((len2 - len3) >> 1) + 1);
		}
	}

	if (tag)
	{
		if (alert)
		{
			str[0] = '!';
		}
		else
		{
			str[0] = tag[0];
		}

		if (alert || !prioritizeValue)
		{
			str[1] = '\0';
			AddResultString(str);
		}

		if (prioritizeValue)
		{
			if (value)
			{
				if (alert)
				{
					AddResultString(alert_str, value);
				}
				else
				{
					AddResultString(value);
				}
			}

			if (verbose)
			{
				SNPRINTF(str, sizeof(str), "%s: %s", tag, value);
			}
			else
			{
				SNPRINTF(str, sizeof(str), "%s: [%s]", tag, value);
		}

			if (alert)
			{
				AddResultString(alert_str, str);
			}
			else
			{
		AddResultString(str);
			}
		}
		else
		{
		if (alert)
		{
				AddResultString(alert_str, tag);
		}
		else
		{
			AddResultString(tag);
		}

		if (value)
		{
			if (verbose)
			{
				SNPRINTF(str, sizeof(str), "%s: %s", tag, value);
			}
			else
			{
				SNPRINTF(str, sizeof(str), "%s: [%s]", tag, value);
			}
			if (alert)
			{
					AddResultString(alert_str, str);
			}
			else
			{
				AddResultString(str);
			}
		}
	}
	}
	else
	{
		if (value)
		{
			SNPRINTF(str, sizeof(str), "%s", value);
			if (alert)
			{
				AddResultString(alert_str, str);
			}
			else
			{
			AddResultString(str);
			}
		}
	}

	if (verbose)
	{
		if (tag)
		{
			if (apply_pad)
			{
				SNPRINTF(str, sizeof(str), "%s%s: (%s)%s", pad, tag, verbose, pad);
			}
			else
			{
				SNPRINTF(str, sizeof(str), "%s: (%s)", tag, verbose);
			}
		}
		else
		{
			if (apply_pad)
			{
				SNPRINTF(str, sizeof(str), "%s(%s)%s", pad, verbose, pad);
			}
			else
			{
				SNPRINTF(str, sizeof(str), "(%s)", verbose);
			}
		}
		AddResultString(str);
	}
}

void SpiAnalyzerResults::TableBuilder(bool fMosiChannel, char* text, bool alert)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char* prefix;
	char mosi_prefix[] = "MOSI-";
	char miso_prefix[] = "MISO-";
	if(fMosiChannel)
	{
		prefix = &mosi_prefix[0];
	}
	else
	{
		prefix = &miso_prefix[0];
	}
	if(alert)
	{
		SNPRINTF(str, sizeof(str), "%s!%s", prefix, text);
	}
	else
	{
		SNPRINTF(str, sizeof(str), "%s%s", prefix, text);
	}
	AddTabularText(str);
}

bool GetSpiCtrlString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;
	const char separatorStr[] = " | ";
	display_base = display_base; /* Resolve compiler warning */

	for (U8 i = 0; i < (sizeof(asSpiCtrlNames) / sizeof(tValueName)); i++)
	{
		if (((asSpiCtrlNames[i].value & val) != 0) || (asSpiCtrlNames[i].value == ABP_SPI_CTRL_CMDCNT))
		{
			if (!firstFlag)
			{
				SNPRINTF(str, maxLen, separatorStr);
				str += (U16)strlen(separatorStr);
				maxLen -= (U16)strlen(separatorStr);
			}
			firstFlag = false;
			alert = (alert || asSpiCtrlNames[i].alert);
			if (asSpiCtrlNames[i].value == ABP_SPI_CTRL_CMDCNT)
			{
				/* Special handling for command count */
				SNPRINTF(str, maxLen, "%s%d", asSpiCtrlNames[i].name, (val & ABP_SPI_CTRL_CMDCNT) >> 1);
				str += ((U16)strlen(asSpiCtrlNames[i].name) + 1);
				maxLen -= ((U16)strlen(asSpiCtrlNames[i].name) + 1);
			}
			else
			{
				SNPRINTF(str, maxLen, asSpiCtrlNames[i].name);
				str += (U16)strlen(asSpiCtrlNames[i].name);
				maxLen -= (U16)strlen(asSpiCtrlNames[i].name);
			}
		}
	}

	return alert;
}

bool GetSpiStsString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;
	const char separatorStr[] = " | ";

	display_base = display_base; /* Resolve compiler warning */

	for (U8 i = 0; i < (sizeof(asSpiStsNames) / sizeof(tValueName)); i++)
	{
		if (((asSpiStsNames[i].value & val) != 0) || (asSpiStsNames[i].value == ABP_SPI_STATUS_CMDCNT))
		{
			if (!firstFlag)
			{
				SNPRINTF(str, maxLen, separatorStr);
				str += (U16)strlen(separatorStr);
				maxLen -= (U16)strlen(separatorStr);
			}
			firstFlag = false;
			alert = (alert || asSpiStsNames[i].alert);
			if (asSpiStsNames[i].value == ABP_SPI_STATUS_CMDCNT)
			{
				/* Special handling for command count */
				SNPRINTF(str, maxLen, "%s%d", asSpiStsNames[i].name, (val & ABP_SPI_STATUS_CMDCNT) >> 1);
				str += ((U16)strlen(asSpiStsNames[i].name) + 1);
				maxLen -= ((U16)strlen(asSpiStsNames[i].name) + 1);
			}
			else
			{
				SNPRINTF(str, maxLen, asSpiStsNames[i].name);
				str += (U16)strlen(asSpiStsNames[i].name);
				maxLen -= (U16)strlen(asSpiStsNames[i].name);
			}
		}
	}

	return alert;
}

bool GetApplStsString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = false;
	bool found = false;

	for (U8 i = 0; i < (sizeof(asApplStsNames) / sizeof(tValueName)); i++)
	{
		if (asApplStsNames[i].value == val)
		{
			SNPRINTF(str, maxLen, asApplStsNames[i].name);
			alert = asApplStsNames[i].alert;
			found = true;
			break;
		}
	}

	if (!found)
	{
		AnalyzerHelpers::GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(e_ABCC_MOSI_APP_STAT), number_str, sizeof(number_str));
		SNPRINTF(str, maxLen, "Reserved: %s", number_str);
		alert = true;
	}

	return alert;
}

bool GetAbccStatusString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = false;
	bool found = false;
	char tmpstr[FORMATTED_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < (sizeof(asAnybusStsNames) / sizeof(tValueName)); i++)
	{
		if (asAnybusStsNames[i].value == (val & (ABCC_STATUS_CODE_MASK | ABCC_STATUS_RESERVED_MASK)))
		{
			SNPRINTF(tmpstr, sizeof(tmpstr), asAnybusStsNames[i].name);
			alert = asAnybusStsNames[i].alert;
			found = true;
			break;
		}
	}

	if (!found)
	{
		AnalyzerHelpers::GetNumberString(val, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MISO_ANB_STAT), number_str, sizeof(number_str));
		SNPRINTF(tmpstr, sizeof(tmpstr), "Reserved: %s", number_str);
		alert = true;
	}

	if ((val & ABCC_STATUS_SUP_MASK) == ABCC_STATUS_SUP_MASK)
	{
		SNPRINTF(str, maxLen, "%s | SUP", tmpstr);
	}
	else
	{
		SNPRINTF(str, maxLen, "%s", tmpstr);
	}

	return alert;
}

bool GetErrorRspString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	for (U8 i = 0; i < (sizeof(asErrorRspNames) / sizeof(tValueName)); i++)
	{
		if (asErrorRspNames[i].value == val)
		{
			SNPRINTF(str, maxLen, asErrorRspNames[i].name);
			return asErrorRspNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
	SNPRINTF(str, maxLen, "Reserved: %s", number_str);
	return true;
}

bool GetIntMaskString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;
	const char separatorStr[] = " | ";

	display_base = display_base; /* Resolve compiler warning */

	for (U8 i = 0; i < (sizeof(asIntMaskNames) / sizeof(tValueName)); i++)
	{
		if ((asIntMaskNames[i].value & val) != 0)
		{
			if (!firstFlag)
			{
				SNPRINTF(str, maxLen, separatorStr);
				str += (U16)strlen(separatorStr);
				maxLen -= (U16)strlen(separatorStr);
			}
			firstFlag = false;
			alert = (alert || asIntMaskNames[i].alert);
			SNPRINTF(str, maxLen, asIntMaskNames[i].name);
			str += (U16)strlen(asIntMaskNames[i].name);
			maxLen -= (U16)strlen(asIntMaskNames[i].name);
		}
	}

	if (firstFlag)
	{
		SNPRINTF(str, maxLen, "None");
	}
	return alert;
}

bool GetLedStatusString(U16 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;
	const char separatorStr[] = " | ";

	display_base = display_base; /* Resolve compiler warning */

	for (U8 i = 0; i < (sizeof(asLedStsNames) / sizeof(tValueName)); i++)
	{
		if ((asLedStsNames[i].value & val) != 0)
		{
			if (!firstFlag)
			{
				SNPRINTF(str, maxLen, separatorStr);
				str += (U16)strlen(separatorStr);
				maxLen -= (U16)strlen(separatorStr);
			}
			firstFlag = false;
			alert = (alert || asLedStsNames[i].alert);
			SNPRINTF(str, maxLen, asLedStsNames[i].name);
			str += (U16)strlen(asLedStsNames[i].name);
			maxLen -= (U16)strlen(asLedStsNames[i].name);
		}
	}

	if (firstFlag)
	{
		SNPRINTF(str, maxLen, "None");
	}
	return alert;
}

bool GetNamedAttrString(U16 inst, U8 val,
	char* str, U16 maxLen,
	DisplayBase display_base,
	tValueName* pasObjNames, U8 NoObjNames,
	tValueName* pasInstNames, U8 NoInstNames)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	tValueName* pasAttrNames = NULL;
	U8 bNoAttrs = 0;
	bool alert = false;
	bool found = false;

	if (inst == ABP_INST_OBJ)
	{
		if (val <= asObjAttrNames[(sizeof(asObjAttrNames) / sizeof(tValueName)) - 1].value)
		{
			pasAttrNames = &asObjAttrNames[0];
			bNoAttrs = (sizeof(asObjAttrNames) / sizeof(tValueName));
		}
		else
		{
			if (pasObjNames != NULL)
			{
				pasAttrNames = pasObjNames;
				bNoAttrs = NoObjNames;
			}
			else
			{
				alert = true;
			}
		}
	}
	else
	{
		if (pasInstNames != NULL)
		{
			pasAttrNames = pasInstNames;
			bNoAttrs = NoInstNames;
		}
		else
		{
			alert = true;
		}
	}

	if (!alert)
	{
		for (U8 i = 0; i < bNoAttrs; i++)
		{
			if (pasAttrNames[i].value == val)
			{
				SNPRINTF(str, maxLen, pasAttrNames[i].name);
				alert = pasAttrNames[i].alert;
				found = true;
			}
		}
	}

	if (!found)
	{
		AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
		SNPRINTF(str, maxLen, "Unknown: %s", number_str);
		alert = true;
	}
	return alert;
}

bool GetObjectString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	for (U8 i = 0; i < (sizeof(asObjectNames) / sizeof(tValueName)); i++)
	{
		if (asObjectNames[i].value == val)
		{
			SNPRINTF(str, maxLen, asObjectNames[i].name);
			return asObjectNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
	SNPRINTF(str, maxLen, "Unknown: %s", number_str);
	return true;
}

bool GetObjSpecificCmdString(U8 val, char* str, U16 maxLen, tValueName* pasCmdNames, U8 bNoCmds, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	for (U8 i = 0; i < bNoCmds; i++)
	{
		if (pasCmdNames[i].value == val)
		{
			SNPRINTF(str, maxLen, pasCmdNames[i].name);
			return pasCmdNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
	SNPRINTF(str, maxLen, "Unknown: %s", number_str);
	return true;
}

bool GetCmdString(U8 val, U8 obj, char* str, U16 maxLen, DisplayBase display_base)
{
	char strBuffer[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = false;
	U8 cmd = (val & ABP_MSG_HEADER_CMD_BITS);

	if (IS_CMD_STANDARD(cmd))
	{
		for (U8 i = 0; i < (sizeof(asCmdNames) / sizeof(tValueName)); i++)
		{
			if (asCmdNames[i].value == cmd)
			{
				SNPRINTF(str, maxLen, asCmdNames[i].name);
				return asCmdNames[i].alert;
			}
		}
	}
	else if (IS_CMD_OBJECT_SPECIFIC(cmd))
	{
		switch(obj)
		{
		case ABP_OBJ_NUM_NW:
			/* Network Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asNetCmdNames[0], (sizeof(asNetCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_FUSM:
			/* Functional Safety Module Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asFusmCmdNames[0], (sizeof(asFusmCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_FSI:
		case ABP_OBJ_NUM_AFSI:
			/* (Application/Anybus) File System Interface Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asFsiCmdNames[0], (sizeof(asFsiCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_ASM:
			/* Assembly Mapping Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asAsmCmdNames[0], (sizeof(asAsmCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_MDD:
			/* Modular Device Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asMddCmdNames[0], (sizeof(asMddCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_PNIO:
			/* PROFINET IO Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asPnioCmdNames[0], (sizeof(asPnioCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_EIP:
			/* EtherNet/IP Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asEipCmdNames[0], (sizeof(asEipCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_APPD:
			/* Application Data Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asAppDataCmdNames[0], (sizeof(asAppDataCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_APP:
			/* Application Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asAppCmdNames[0], (sizeof(asAppCmdNames) / sizeof(tValueName)), display_base);
			break;
		default:
			AnalyzerHelpers::GetNumberString(cmd, display_base, 8, strBuffer, sizeof(strBuffer));
			alert = true; //TODO: We only alert here because we have not implmented all object specific commands yet.
			break;
		}
		SNPRINTF(str, maxLen, "Object Specific: %s", strBuffer);
	}
	else
	{
		AnalyzerHelpers::GetNumberString(cmd, display_base, 8, strBuffer, sizeof(strBuffer));
		SNPRINTF(str, maxLen, "Reserved: %s", strBuffer);
		alert = true;
	}

	return alert;
}

bool SpiAnalyzerResults::BuildCmdString(U8 val, U8 obj, DisplayBase display_base)
{
	bool errorRspMsg;
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetCmdString(val, obj, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
	if ((val & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
	{
		errorRspMsg = true;
		StringBuilder("ERR_RSP", number_str, str, true);
	}
	else
	{
		errorRspMsg = false;
		if ((val & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
		{
			StringBuilder("CMD", number_str, str, alert);
		}
		else
		{
			StringBuilder("RSP", number_str, str, alert);
		}
	}

	return errorRspMsg;
}

bool GetAttrString(U8 obj, U16 inst, U16 val, char* str, U16 maxlen, bool indexed, bool* pAlert, DisplayBase display_base)
{
	bool objFound = true;
	U8 ofst = 0;
	if (indexed)
	{
		SNPRINTF(str, maxlen, "Index %d, ", (U8)(val >> 8));
		ofst = (U8)strlen(str);
	}
	switch (obj)
	{
	case ABP_OBJ_NUM_ANB:
		/* Anybus Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asAnbInstAttrNames[0], sizeof(asAnbInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_DI:
		/* Diagnostic Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asDiObjAttrNames[0], sizeof(asDiObjAttrNames) / sizeof(tValueName),
			&asDiInstAttrNames[0], sizeof(asDiInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_NW:
		/* Network Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asNwInstAttrNames[0], sizeof(asNwInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_NC:
		/* Network Configuration Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asNcInstAttrNames[0], sizeof(asNcInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_SOC:
		/* Socket Interface Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asSocObjAttrNames[0], sizeof(asSocObjAttrNames) / sizeof(tValueName),
			&asSocInstAttrNames[0], sizeof(asSocInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_SMTP:
		/* SMTP Client Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asSmtpObjAttrNames[0], sizeof(asSmtpObjAttrNames) / sizeof(tValueName),
			&asSmtpInstAttrNames[0], sizeof(asSmtpInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_FSI:
	case ABP_OBJ_NUM_AFSI:
		/* File system objects */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asFsiObjAttrNames[0], sizeof(asFsiObjAttrNames) / sizeof(tValueName),
			&asFsiInstAttrNames[0], sizeof(asFsiInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_FUSM:
		/* Functional Safety Module Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asFusmInstAttrNames[0], sizeof(asFusmInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_NWETN:
	case ABP_OBJ_NUM_ETN:
		/* Network Ethernet Object / Ethernet Host Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asNwEtnInstAttrNames[0], sizeof(asNwEtnInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_CPC:
		/* CIP Port Configuration Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asCpcObjAttrNames[0], sizeof(asCpcObjAttrNames) / sizeof(tValueName),
			&asCpcInstAttrNames[0], sizeof(asCpcInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_SAFE:
		/* Functional Safety Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asSafeInstAttrNames[0], sizeof(asSafeInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_EPL:
		/* Ethernet POWERLINK Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asEplInstAttrNames[0], sizeof(asEplInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_ASM:
		/* Assembly Mapping Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asAsmObjAttrNames[0], sizeof(asAsmObjAttrNames) / sizeof(tValueName),
			&asAsmInstAttrNames[0], sizeof(asAsmInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_MDD:
		/* Modular Device Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asMddObjAttrNames[0], sizeof(asMddObjAttrNames) / sizeof(tValueName),
			NULL, 0);
		break;
	case ABP_OBJ_NUM_CIPID:
		/* CIP Identity Host Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asCipIdInstAttrNames[0], sizeof(asCipIdInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_SYNC:
		/* Sync Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asSyncInstAttrNames[0], sizeof(asSyncInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_ECT:
		/* EtherCAT Host Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asEtcInstAttrNames[0], sizeof(asEtcInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_PNIO:
		/* PROFINET IO Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asPnioInstAttrNames[0], sizeof(asPnioInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_EIP:
		/* EtherNet/IP Host Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asEipInstAttrNames[0], sizeof(asEipInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_APPD:
		/* Application Data Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asAppdObjAttrNames[0], sizeof(asAppdObjAttrNames) / sizeof(tValueName),
			&asAppdInstAttrNames[0], sizeof(asAppdInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_APP:
		/* Application Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asAppInstAttrNames[0], sizeof(asAppInstAttrNames) / sizeof(tValueName));
		break;
	default:
		objFound = false;
		break;
	}

	return objFound;
}

void SpiAnalyzerResults::BuildAttrString(U8 obj, U16 inst, U16 val, bool indexed, DisplayBase display_base)
{
	char verbose_str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = false;
	bool objFound = true;

	objFound = GetAttrString(obj, inst, val, verbose_str, sizeof(verbose_str), indexed, &alert, display_base);

	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MSG_CMD_EXT), number_str, sizeof(number_str));

	if (objFound)
	{
		StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_CMD_EXT), number_str, verbose_str, alert);
	}
	else
	{
		StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_CMD_EXT), number_str, NULL, false);
	}
}

void SpiAnalyzerResults::BuildObjectString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetObjectString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MSG_OBJECT), number_str, sizeof(number_str));
	StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_OBJECT), number_str, str, alert);
}

void SpiAnalyzerResults::BuildSpiCtrlString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetSpiCtrlString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MOSI_SPI_CTRL), number_str, sizeof(number_str));
	StringBuilder(GET_MOSI_FRAME_TAG(e_ABCC_MOSI_SPI_CTRL), number_str, str, alert);
}

void SpiAnalyzerResults::BuildSpiStsString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetSpiStsString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MISO_SPI_STAT), number_str, sizeof(number_str));
	StringBuilder(GET_MISO_FRAME_TAG(e_ABCC_MISO_SPI_STAT), number_str, str, alert);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetErrorRspString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
	StringBuilder("ERR_CODE", number_str, str, alert);
}

void SpiAnalyzerResults::BuildIntMask(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetIntMaskString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MOSI_INT_MASK), number_str, sizeof(number_str));
	StringBuilder(GET_MOSI_FRAME_TAG(e_ABCC_MOSI_INT_MASK), number_str, str, alert);
}

void SpiAnalyzerResults::BuildAbccStatus(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetAbccStatusString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MISO_ANB_STAT), number_str, sizeof(number_str));
	StringBuilder(GET_MISO_FRAME_TAG(e_ABCC_MISO_ANB_STAT), number_str, str, alert);
}

void SpiAnalyzerResults::BuildApplStatus(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
	bool alert = GetApplStsString((U8)val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MOSI_APP_STAT), number_str, sizeof(number_str));
	StringBuilder(GET_MOSI_FRAME_TAG(e_ABCC_MOSI_APP_STAT), number_str, str, alert);
}

void SpiAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)
{
	ClearResultStrings();
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char str[FORMATTED_STRING_BUFFER_SIZE];
	bool alert = false;
	Frame frame = GetFrame(frame_index);
	uAbccSpiStates uState;
	uState.eMosi = (tAbccMosiStates)frame.mType;

	if ((frame.mFlags & (SPI_FRAG_ERROR_FLAG | SPI_ERROR_FLAG)) == 0)
	{
		if ((channel == mSettings->mMosiChannel) && IS_MOSI_FRAME(frame))
		{
			switch (uState.eMosi)
			{
			case e_ABCC_MOSI_IDLE:
				break;
			case e_ABCC_MOSI_SPI_CTRL:
				BuildSpiCtrlString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_RESERVED1:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, "Reserved", alert);
				break;
			case e_ABCC_MOSI_MSG_LEN:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, str, alert);
				break;
			case e_ABCC_MOSI_PD_LEN:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, str, alert);
				break;
			case e_ABCC_MOSI_APP_STAT:
				BuildApplStatus((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_INT_MASK:
				BuildIntMask((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_WR_MSG_FIELD:
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
				{
					BuildErrorRsp((U8)frame.mData1, display_base);
				}
				else
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, str, alert, (mSettings->mMsgDataPriority == e_MSG_DATA_PRIORITIZE_DATA));
				}
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MOSI_WR_MSG_SUBFIELD_data), number_str, sizeof(number_str));
				StringBuilder("--", number_str, NULL, alert, false);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					SNPRINTF(str, sizeof(str), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
					alert = true;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
				}
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, str, alert);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_res1:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, "Reserved", alert);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, NULL, alert);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
				BuildObjectString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, NULL, alert);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
				BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_res2:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, "Reserved", alert);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
				if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
				{
					//TODO joca consider making a type for the data stored here
					BuildAttrString((U8)(frame.mData2 >> 8), (U16)(frame.mData2 >> 16), (U16)frame.mData1, false, display_base);
				}
				else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
				{
					//TODO joca consider making a type for the data stored here
					BuildAttrString((U8)(frame.mData2 >> 8), (U16)(frame.mData2 >> 16), (U16)frame.mData1, true, display_base);
				}
				else
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, NULL, alert);
				}
				break;
			case e_ABCC_MOSI_WR_PD_FIELD:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, str, alert);
				break;
			case e_ABCC_MOSI_CRC32:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
				{
					SNPRINTF(str, sizeof(str), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
					alert = true;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
				}
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, str, alert);
				break;
			case e_ABCC_MOSI_PAD:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), number_str, NULL, alert);
				break;
			default:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, sizeof(number_str));
				alert = true;
				StringBuilder("UNKWN", number_str, "Internal Error: Unknown State", alert);
				break;
			}
		}
		else if ((channel == mSettings->mMisoChannel) && IS_MISO_FRAME(frame))
		{
			switch (uState.eMiso)
			{
			case e_ABCC_MISO_IDLE:
				break;
			case e_ABCC_MISO_Reserved1:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, "Reserved", alert);
				break;
			case e_ABCC_MISO_Reserved2:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));

				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, "Reserved", alert);
				break;
			case e_ABCC_MISO_LED_STAT:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				alert = GetLedStatusString((U16)frame.mData1, str, sizeof(str), display_base);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, str, alert);
			break;
			case e_ABCC_MISO_ANB_STAT:
				BuildAbccStatus((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MISO_SPI_STAT:
				BuildSpiStsString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MISO_NET_TIME:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, NULL, alert);
				break;
			case e_ABCC_MISO_RD_MSG_FIELD:
			case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
				{
					BuildErrorRsp((U8)frame.mData1, display_base);
				}
				else
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, str, alert, (mSettings->mMsgDataPriority == e_MSG_DATA_PRIORITIZE_DATA));
				}
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MISO_RD_MSG_SUBFIELD_data), number_str, sizeof(number_str));
				StringBuilder("--", number_str, NULL, alert, false);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					SNPRINTF(str, sizeof(str), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
					alert = true;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
				}
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, str, alert);
			break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_res1:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, "Reserved", alert);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, NULL, alert);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
				BuildObjectString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, NULL, alert);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
				BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_res2:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				alert = (frame.mData1 != 0);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, "Reserved", alert);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
				if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
				{
					//TODO joca consider making a type for the data stored here
					BuildAttrString((U8)(frame.mData2 >> 8), (U16)(frame.mData2 >> 16), (U16)frame.mData1, false, display_base);
				}
				else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
				{
					//TODO joca consider making a type for the data stored here
					BuildAttrString((U8)(frame.mData2 >> 8), (U16)(frame.mData2 >> 16), (U16)frame.mData1, true, display_base);
				}
				else
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, NULL, alert);
				}
				break;
			case e_ABCC_MISO_RD_PD_FIELD:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, str, alert);
			break;
			case e_ABCC_MISO_CRC32:
			{
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), number_str, sizeof(number_str));
				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
				{
					SNPRINTF(str, sizeof(str), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
					alert = true;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
				}
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), number_str, str, alert);
			}
			break;
			default:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, sizeof(number_str));
				alert = true;
				StringBuilder("UNKWN", number_str, "Internal Error: Unknown State", alert);
				break;
			}
		}
	}
	else
	{
		alert = true;
		if ((frame.mFlags & SPI_FRAG_ERROR_FLAG) == SPI_FRAG_ERROR_FLAG)
		{
			StringBuilder("FRAG", NULL, "Fragmented ABCC SPI Packet", alert);
		}
		else if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
		{
			StringBuilder("ERROR", NULL, "Settings mismatch, The initial (idle) state of the CLK line does not match the settings.", alert);
		}
	}
}

void SpiAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 /*export_type_user_id*/)
{
	//export_type_user_id is only important if we have more than one export type.

	std::stringstream ss;
	void* f = AnalyzerHelpers::StartFile(file);

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	ss << "Time [s],Packet ID,MOSI,MISO" << std::endl;

	bool mosi_used = true;
	bool miso_used = true;

	if (mSettings->mMosiChannel == UNDEFINED_CHANNEL)
		mosi_used = false;

	if (mSettings->mMisoChannel == UNDEFINED_CHANNEL)
		miso_used = false;

	U64 num_frames = GetNumFrames();
	for (U32 i = 0; i < num_frames; i++)
	{
		Frame frame = GetFrame(i);

		if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
			continue;

		char time_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
		AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);

		char mosi_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE] = "";
		if (mosi_used == true)
		{
			if ((frame.mFlags & SPI_MOSI_FLAG) == SPI_MOSI_FLAG)
			{
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, mosi_str, sizeof(mosi_str));
			}
		}

		char miso_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE] = "";
		if (miso_used == true)
		{
			if ((frame.mFlags & SPI_MOSI_FLAG) != SPI_MOSI_FLAG)
			{
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, miso_str, sizeof(miso_str));
			}
		}

		U64 packet_id = GetPacketContainingFrameSequential(i);
		if (packet_id != INVALID_RESULT_INDEX)
			ss << time_str << "," << packet_id << "," << mosi_str << "," << miso_str << std::endl;
		else
			ss << time_str << ",," << mosi_str << "," << miso_str << std::endl;  //it's ok for a frame not to be included in a packet.

		AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), (U32)ss.str().length(), f);
		ss.str(std::string());

		if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true)
		{
			AnalyzerHelpers::EndFile(f);
			return;
		}
	}

	UpdateExportProgressAndCheckForCancel(num_frames, num_frames);
	AnalyzerHelpers::EndFile(f);
}

U64 SpiAnalyzerResults::GetFrameIdOfAbccFieldContainedInPacket(U64 packet_index, bool fMosiChannel, U8 type)
{
	U64 frame_index = INVALID_RESULT_INDEX;
	U64 first_frame_index;
	U64 last_frame_index;

	if(packet_index != INVALID_RESULT_INDEX)
	{
		GetFramesContainedInPacket(packet_index, &first_frame_index, &last_frame_index);
		if((first_frame_index != INVALID_RESULT_INDEX) && (last_frame_index != INVALID_RESULT_INDEX))
		{
			for(frame_index = first_frame_index; frame_index <= last_frame_index; frame_index++)
			{
				Frame frame = GetFrame(frame_index);
				if( ((fMosiChannel) && IS_MOSI_FRAME(frame) && (frame.mType == type)) ||
					((!fMosiChannel) && IS_MISO_FRAME(frame) && (frame.mType == type)) )
				{
					break;
				}
			}
		}
	}
	return frame_index;
}

void SpiAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
	ClearTabularText();
	Frame frame = GetFrame(frame_index);
	bool mosi_used = true;
	bool miso_used = true;

	if (mSettings->mMosiChannel == UNDEFINED_CHANNEL)
		mosi_used = false;

	if (mSettings->mMisoChannel == UNDEFINED_CHANNEL)
		miso_used = false;

	if (miso_used || mosi_used)
	{
		if (mSettings->mErrorIndexing == true)
		{
			if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
			{
				if (frame.mType == e_ABCC_MISO_CRC32)
				{
					TableBuilder(IS_MOSI_FRAME(frame), "CRC32", true);
				}
				return;
			}
			else if ((frame.mFlags & (SPI_FRAG_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_FRAG_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG))
			{
				AddTabularText("!Fragmented ABCC SPI Packet");
				return;
			}

			if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
			{
				if ((frame.mType == e_ABCC_MOSI_APP_STAT) && IS_MOSI_FRAME(frame))
				{
					char str[FORMATTED_STRING_BUFFER_SIZE];
					/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
					if (GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
					{
						AddTabularText("!Application Status : (", str, ")");
						return;
					}
					else
					{
						if (mSettings->mApplStatusIndexing == true)
						{
							AddTabularText("Application Status : (", str, ")");
							return;
						}
					}
				}
				else if ((frame.mType == e_ABCC_MISO_ANB_STAT) && IS_MISO_FRAME(frame))
				{
					char str[FORMATTED_STRING_BUFFER_SIZE];
					if (GetAbccStatusString((U8)frame.mData1, &str[0], sizeof(str), display_base))
					{
						AddTabularText("!Anybus Status : (", str, ")");
						return;
					}
					else
					{
						if (mSettings->mAnybusStatusIndexing == true)
						{
							AddTabularText("Anybus Status : (", str, ")");
							return;
						}
					}
				}
				else if ((frame.mType == e_ABCC_MISO_RD_MSG_SUBFIELD_size) && IS_MISO_FRAME(frame))
				{
					TableBuilder(false, "Message Size : Exceeds Maximum", true);
					return;
				}
				else if ((frame.mType == e_ABCC_MOSI_WR_MSG_SUBFIELD_size) && IS_MOSI_FRAME(frame))
				{
					TableBuilder(true, "Message Size : Exceeds Maximum", true);
					return;
				}
			}
			}

			if ((frame.mType == e_ABCC_MISO_NET_TIME) && IS_MISO_FRAME(frame))
			{
				char tmp[35];
			Frame frame_lookup;
			U64 frame_index_lookup = INVALID_RESULT_INDEX;
			U64 packet_index_lookup = GetPacketContainingFrame(frame_index);
			switch(mSettings->mTimestampIndexing)
			{
			case e_TIMESTAMP_ALL_PACKETS:
				SNPRINTF(tmp, sizeof(tmp), "0x%08X (Delta : 0x%08X)", (U32)frame.mData1, (U32)frame.mData2);
				AddTabularText("Time : ", tmp);
				SNPRINTF(tmp, sizeof(tmp), "Packet : 0x%016llX", packet_index_lookup);
				AddTabularText(tmp);
				break;
			case e_TIMESTAMP_WRITE_PROCESS_DATA_VALID:
				if(packet_index_lookup != INVALID_RESULT_INDEX)
				{
					frame_index_lookup = GetFrameIdOfAbccFieldContainedInPacket(packet_index_lookup, true, (U8)e_ABCC_MOSI_SPI_CTRL);
					if(frame_index_lookup != INVALID_RESULT_INDEX)
					{
						frame_lookup = GetFrame(frame_index_lookup);
						if ((frame_lookup.mData1 & ABP_SPI_CTRL_WRPD_VALID) != 0)
						{
							//TODO joca: adapt delta, to measure delta between "New Write Process Data"
							SNPRINTF(tmp, sizeof(tmp), "0x%08X (Delta : 0x%08X)", (U32)frame.mData1, (U32)frame.mData2);
							AddTabularText("Time : ", tmp);
							SNPRINTF(tmp, sizeof(tmp), "Packet : 0x%016llX", packet_index_lookup);
							AddTabularText(tmp);
						}
					}
					else
					{
						/* Print out an Error indicator so that the user can check on this event */
						SNPRINTF(tmp, sizeof(tmp), "(0x%016llX)", frame_index_lookup);
						AddTabularText("Frame : !", tmp);
					}
				}
				else
				{
					/* Print out an Error indicator so that the user can check on this event */
					SNPRINTF(tmp, sizeof(tmp), "(0x%016llX)", packet_index_lookup);
					AddTabularText("Packet : !", tmp);
				}
				break;
			case e_TIMESTAMP_NEW_READ_PROCESS_DATA:
				if(packet_index_lookup != INVALID_RESULT_INDEX)
				{
					frame_index_lookup = GetFrameIdOfAbccFieldContainedInPacket(packet_index_lookup, false, (U8)e_ABCC_MOSI_SPI_CTRL);
					if(frame_index_lookup != INVALID_RESULT_INDEX)
					{
						frame_lookup = GetFrame(frame_index_lookup);
						if ((frame_lookup.mData1 & ABP_SPI_STATUS_NEW_PD) != 0)
						{
							//TODO joca: adapt delta, to measure delta between "New Write Process Data"
							SNPRINTF(tmp, sizeof(tmp), "0x%08X (Delta : 0x%08X)", (U32)frame.mData1, (U32)frame.mData2);
							AddTabularText("Time : ", tmp);
							SNPRINTF(tmp, sizeof(tmp), "Packet : 0x%016llX", packet_index_lookup);
							AddTabularText(tmp);
						}
					}
					else
					{
						/* Print out an Error indicator so that the user can check on this event */
						SNPRINTF(tmp, sizeof(tmp), "(0x%016llX)", frame_index_lookup);
						AddTabularText("Frame : !", tmp);
					}
				}
				else
				{
					/* Print out an Error indicator so that the user can check on this event */
					SNPRINTF(tmp, sizeof(tmp), "(0x%016llX)", packet_index_lookup);
					AddTabularText("Packet : !", tmp);
				}
				break;
			default:
			case e_TIMESTAMP_DISABLED:
				break;
			}
			return;
		}

		if (mSettings->mApplStatusIndexing == true)
		{
			if ((frame.mType == e_ABCC_MOSI_APP_STAT) && IS_MOSI_FRAME(frame))
			{
				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					char str[FORMATTED_STRING_BUFFER_SIZE];
					/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
					if (GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
					{
						AddTabularText("!Application Status : (", str, ")");
					}
					else
					{
						AddTabularText("Application Status : (", str, ")");
					}
					return;
				}
			}
		}

		if (mSettings->mAnybusStatusIndexing == true)
		{
			if ((frame.mType == e_ABCC_MISO_ANB_STAT) && IS_MISO_FRAME(frame))
			{
				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					char str[FORMATTED_STRING_BUFFER_SIZE];
					if (GetAbccStatusString((U8)frame.mData1, &str[0], sizeof(str), display_base))
					{
						AddTabularText("!Anybus Status : (", str, ")");
					}
					else
					{
						AddTabularText("Anybus Status : (", str, ")");
					}
					return;
				}
			}
		}

		/* Since tabular text is sequentially processed and indexed,
		** buffer the "Object", "Instance", "Cmd", and "Ext";
		** then add as a single text entry. */
		if (mSettings->mMessageIndexingVerbosityLevel != e_VERBOSITY_LEVEL_DISABLED)
		{
			static char size_str[2][FORMATTED_STRING_BUFFER_SIZE] = { "" };
			static char src_str[2][FORMATTED_STRING_BUFFER_SIZE] = { "" };
			static char obj_str[2][FORMATTED_STRING_BUFFER_SIZE] = { "" };
			static char inst_str[2][FORMATTED_STRING_BUFFER_SIZE] = { "" };
			static char cmd_str[2][FORMATTED_STRING_BUFFER_SIZE] = { "" };
			static char ext_str[2][FORMATTED_STRING_BUFFER_SIZE] = { "" };
			static bool fMsgValid[2];
			static bool fMsgErrorRsp[2];
			if (IS_MISO_FRAME(frame))
			{
				switch (frame.mType)
				{
				case e_ABCC_MISO_SPI_STAT:
					if ((frame.mData1 & ABP_SPI_STATUS_M) != 0)
					{
						fMsgValid[ABCC_MISO_CHANNEL] = true;
					}
					else
					{
						fMsgValid[ABCC_MISO_CHANNEL] = false;
					}

					if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG)) == SPI_PROTO_EVENT_FLAG)
					{
						TableBuilder(false, "{Write Message Buffer Full}", false);
						return;
					}
					else if ((frame.mFlags & (SPI_MSG_FRAG_FLAG | SPI_MSG_FIRST_FRAG_FLAG)) == SPI_MSG_FRAG_FLAG)
					{
						/* Fragmentation is in progress */
						if (frame.mData1 & ABP_SPI_STATUS_M)
						{
							if (frame.mData1 & ABP_SPI_STATUS_LAST_FRAG)
							{
								/* Last fragment */
								TableBuilder(false, "{Message Fragment}", false);
							}
							else
							{
								/* More fragments follow */
								TableBuilder(false, "{Message Fragment}++", false);
							}
						}
						return;
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
					if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
					{
						SNPRINTF(size_str[ABCC_MISO_CHANNEL], sizeof(size_str[ABCC_MISO_CHANNEL]), "!Size: %u Bytes", (U16)frame.mData1);
					}
					else
					{
						SNPRINTF(size_str[ABCC_MISO_CHANNEL], sizeof(size_str[ABCC_MISO_CHANNEL]), "Size: %u Bytes", (U16)frame.mData1);
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
					SNPRINTF(src_str[ABCC_MISO_CHANNEL], sizeof(src_str[ABCC_MISO_CHANNEL]), "Source ID: 0x%02X", (U8)frame.mData1);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						char str[FORMATTED_STRING_BUFFER_SIZE];
						bool alert = GetObjectString((U8)frame.mData1, &str[0], sizeof(str), display_base);
						if (alert)
						{
							SNPRINTF(obj_str[ABCC_MISO_CHANNEL], sizeof(obj_str[ABCC_MISO_CHANNEL]), "!Object: %s", str);
						}
						else
						{
							SNPRINTF(obj_str[ABCC_MISO_CHANNEL], sizeof(obj_str[ABCC_MISO_CHANNEL]), "Object: %s", str);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(obj_str[ABCC_MISO_CHANNEL], sizeof(obj_str[ABCC_MISO_CHANNEL]), "Obj {%02X:", (U8)frame.mData1);
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						SNPRINTF(inst_str[ABCC_MISO_CHANNEL], sizeof(inst_str[ABCC_MISO_CHANNEL]), "Instance: 0x%04X", (U16)frame.mData1);
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(inst_str[ABCC_MISO_CHANNEL], sizeof(inst_str[ABCC_MISO_CHANNEL]), "%04Xh}", (U16)frame.mData1);
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
					if ((frame.mData1 & ABP_MSG_HEADER_E_BIT) != 0)
					{
						fMsgErrorRsp[ABCC_MISO_CHANNEL] = true;
					}
					else
					{
						fMsgErrorRsp[ABCC_MISO_CHANNEL] = false;
					}
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						char str[FORMATTED_STRING_BUFFER_SIZE];
						bool alert = GetCmdString((U8)frame.mData1, (U8)frame.mData2, &str[0], sizeof(str), display_base);
						if ((fMsgErrorRsp[ABCC_MISO_CHANNEL] == true) || (alert == true))
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(cmd_str[ABCC_MISO_CHANNEL], sizeof(cmd_str[ABCC_MISO_CHANNEL]), "!Command: %s", str);
							}
							else
							{
								SNPRINTF(cmd_str[ABCC_MISO_CHANNEL], sizeof(cmd_str[ABCC_MISO_CHANNEL]), "!Response: %s", str);
							}
						}
						else
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(cmd_str[ABCC_MISO_CHANNEL], sizeof(cmd_str[ABCC_MISO_CHANNEL]), "Command: %s", str);
							}
							else
							{
								SNPRINTF(cmd_str[ABCC_MISO_CHANNEL], sizeof(cmd_str[ABCC_MISO_CHANNEL]), "Response: %s", str);
							}
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(cmd_str[ABCC_MISO_CHANNEL], sizeof(cmd_str[ABCC_MISO_CHANNEL]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
						}
						else
						{
							SNPRINTF(cmd_str[ABCC_MISO_CHANNEL], sizeof(cmd_str[ABCC_MISO_CHANNEL]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
						}
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
					if (fMsgValid[ABCC_MISO_CHANNEL])
					{
						if ((mSettings->mMessageSrcIdIndexing == true) || (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED))
						{
							AddTabularText("-----MISO MESSAGE-----");
						}
						if (mSettings->mMessageSrcIdIndexing == true)
						{
							TableBuilder(false, src_str[ABCC_MISO_CHANNEL], false);
						}
						if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
						{
							char str[FORMATTED_STRING_BUFFER_SIZE];
							bool alert = false;
							bool found = false;
							bool attrCmd = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
							bool attrIdx = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
							if (attrCmd)
							{
								//TODO joca consider making a type for the data stored here
								found = GetAttrString((U8)(frame.mData2 >> 8), (U16)(frame.mData2 >> 16), (U16)frame.mData1, str, sizeof(str), attrIdx, &alert, display_base);
							}
							if (!found)
							{
								AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, str, sizeof(str));
							}
							if (alert)
							{
								SNPRINTF(ext_str[ABCC_MISO_CHANNEL], sizeof(ext_str[ABCC_MISO_CHANNEL]), "!Extension: %s", str);
							}
							else
							{
								SNPRINTF(ext_str[ABCC_MISO_CHANNEL], sizeof(ext_str[ABCC_MISO_CHANNEL]), "Extension: %s", str);
							}
							TableBuilder(false, size_str[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, obj_str[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, inst_str[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, cmd_str[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, ext_str[ABCC_MISO_CHANNEL], false);
							if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								TableBuilder(false, "First Fragment, More Follow", false);
							}
						}
						else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
						{
							SNPRINTF(ext_str[ABCC_MISO_CHANNEL], sizeof(ext_str[ABCC_MISO_CHANNEL]), "%04Xh}", (U16)frame.mData1);
							if (fMsgErrorRsp[ABCC_MISO_CHANNEL])
							{
								AddTabularText("MISO-!", obj_str[ABCC_MISO_CHANNEL], inst_str[ABCC_MISO_CHANNEL], cmd_str[ABCC_MISO_CHANNEL], ext_str[ABCC_MISO_CHANNEL]);
							}
							else
							{
								if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
								{
									AddTabularText("MISO-", obj_str[ABCC_MISO_CHANNEL], inst_str[ABCC_MISO_CHANNEL], cmd_str[ABCC_MISO_CHANNEL], ext_str[ABCC_MISO_CHANNEL], "++");
								}
								else
								{
									AddTabularText("MISO-", obj_str[ABCC_MISO_CHANNEL], inst_str[ABCC_MISO_CHANNEL], cmd_str[ABCC_MISO_CHANNEL], ext_str[ABCC_MISO_CHANNEL]);
								}
							}
						}
						return;
					}
					break;
				default:
					break;
				}
			}

			if (IS_MOSI_FRAME(frame))
			{
				switch (frame.mType)
				{
				case e_ABCC_MOSI_SPI_CTRL:
					if ((frame.mData1 & ABP_SPI_CTRL_M) != 0)
					{
						fMsgValid[ABCC_MOSI_CHANNEL] = true;
					}
					else
					{
						fMsgValid[ABCC_MOSI_CHANNEL] = false;
					}

					if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG)) == SPI_PROTO_EVENT_FLAG)
					{
						if (frame_index != 0)
						{
							TableBuilder(true, "{Message Retransmit}", false);
							return;
						}
					}
					else if ((frame.mFlags & (SPI_MSG_FRAG_FLAG | SPI_MSG_FIRST_FRAG_FLAG)) == SPI_MSG_FRAG_FLAG)
					{
						/* Fragmentation is in progress */
						if (frame.mData1 & ABP_SPI_CTRL_M)
						{
							if (frame.mData1 & ABP_SPI_CTRL_LAST_FRAG)
							{
								/* Last fragment */
								TableBuilder(true, "{Message Fragment}", false);
							}
							else
							{
								/* More fragments follow */
								TableBuilder(true, "{Message Fragment}++", false);
							}
						}
						return;
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
					if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
					{
						SNPRINTF(size_str[ABCC_MOSI_CHANNEL], sizeof(size_str[ABCC_MOSI_CHANNEL]), "!Size: %u Bytes", (U16)frame.mData1);
					}
					else
					{
						SNPRINTF(size_str[ABCC_MOSI_CHANNEL], sizeof(size_str[ABCC_MOSI_CHANNEL]), "Size: %u Bytes", (U16)frame.mData1);
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
					SNPRINTF(src_str[ABCC_MOSI_CHANNEL], sizeof(src_str[ABCC_MOSI_CHANNEL]), "Source ID: 0x%02X", (U8)frame.mData1);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						char str[FORMATTED_STRING_BUFFER_SIZE];
						bool alert = GetObjectString((U8)frame.mData1, &str[0], sizeof(str), display_base);
						if (alert)
						{
							SNPRINTF(obj_str[ABCC_MOSI_CHANNEL], sizeof(obj_str[ABCC_MOSI_CHANNEL]), "!Object: %s", str);
						}
						else
						{
							SNPRINTF(obj_str[ABCC_MOSI_CHANNEL], sizeof(obj_str[ABCC_MOSI_CHANNEL]), "Object: %s", str);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(obj_str[ABCC_MOSI_CHANNEL], sizeof(obj_str[ABCC_MOSI_CHANNEL]), "Obj {%02X:", (U8)frame.mData1);
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						SNPRINTF(inst_str[ABCC_MOSI_CHANNEL], sizeof(inst_str[ABCC_MOSI_CHANNEL]), "Instance: 0x%04X", (U16)frame.mData1);
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(inst_str[ABCC_MOSI_CHANNEL], sizeof(inst_str[ABCC_MOSI_CHANNEL]), "%04Xh}", (U16)frame.mData1);
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
					if ((frame.mData1 & ABP_MSG_HEADER_E_BIT) != 0)
					{
						fMsgErrorRsp[ABCC_MOSI_CHANNEL] = true;
					}
					else
					{
						fMsgErrorRsp[ABCC_MOSI_CHANNEL] = false;
					}
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						char str[FORMATTED_STRING_BUFFER_SIZE];
						bool alert = GetCmdString((U8)frame.mData1, (U8)frame.mData2, &str[ABCC_MOSI_CHANNEL], sizeof(str), display_base);
						if ((fMsgErrorRsp[ABCC_MOSI_CHANNEL] == true) || (alert == true))
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(cmd_str[ABCC_MOSI_CHANNEL], sizeof(cmd_str[ABCC_MOSI_CHANNEL]), "!Command: %s", str);
							}
							else
							{
								SNPRINTF(cmd_str[ABCC_MOSI_CHANNEL], sizeof(cmd_str[ABCC_MOSI_CHANNEL]), "!Response: %s", str);
							}
						}
						else
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(cmd_str[ABCC_MOSI_CHANNEL], sizeof(cmd_str[ABCC_MOSI_CHANNEL]), "Command: %s", str);
							}
							else
							{
								SNPRINTF(cmd_str[ABCC_MOSI_CHANNEL], sizeof(cmd_str[ABCC_MOSI_CHANNEL]), "Response: %s", str);
							}
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(cmd_str[ABCC_MOSI_CHANNEL], sizeof(cmd_str[ABCC_MOSI_CHANNEL]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
						}
						else
						{
							SNPRINTF(cmd_str[ABCC_MOSI_CHANNEL], sizeof(cmd_str[ABCC_MOSI_CHANNEL]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
						}
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
					if (fMsgValid[ABCC_MOSI_CHANNEL])
					{
						if ((mSettings->mMessageSrcIdIndexing == true) || (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED))
						{
							AddTabularText("-----MOSI MESSAGE-----");
						}
						if (mSettings->mMessageSrcIdIndexing == true)
						{
							TableBuilder(true, src_str[ABCC_MOSI_CHANNEL], false);
						}
						if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
						{
							char str[FORMATTED_STRING_BUFFER_SIZE];
							bool alert = false;
							bool found = false;
							bool attrCmd = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
							bool attrIdx = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
							if (attrCmd)
							{
								//TODO joca consider making a type for the data stored here
								found = GetAttrString((U8)(frame.mData2 >> 8), (U16)(frame.mData2 >> 16), (U16)frame.mData1, str, sizeof(str), attrIdx, &alert, display_base);
							}
							if (!found)
							{
								AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, str, sizeof(str));
							}
							if (alert)
							{
								SNPRINTF(ext_str[ABCC_MOSI_CHANNEL], sizeof(ext_str[ABCC_MOSI_CHANNEL]), "!Extension: %s", str);
							}
							else
							{
								SNPRINTF(ext_str[ABCC_MOSI_CHANNEL], sizeof(ext_str[ABCC_MOSI_CHANNEL]), "Extension: %s", str);
							}
							TableBuilder(true, size_str[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, obj_str[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, inst_str[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, cmd_str[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, ext_str[ABCC_MOSI_CHANNEL], false);
							if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								TableBuilder(true, "First Fragment, More Follow", false);
							}
						}
						else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
						{
							SNPRINTF(ext_str[ABCC_MOSI_CHANNEL], sizeof(ext_str[ABCC_MOSI_CHANNEL]), "%04Xh}", (U16)frame.mData1);
							if (fMsgErrorRsp[ABCC_MOSI_CHANNEL])
							{
								AddTabularText("MOSI-!", obj_str[ABCC_MOSI_CHANNEL], inst_str[ABCC_MOSI_CHANNEL], cmd_str[ABCC_MOSI_CHANNEL], ext_str[ABCC_MOSI_CHANNEL]);
							}
							else
							{
								if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
								{
									AddTabularText("MOSI-", obj_str[ABCC_MOSI_CHANNEL], inst_str[ABCC_MOSI_CHANNEL], cmd_str[ABCC_MOSI_CHANNEL], ext_str[ABCC_MOSI_CHANNEL], "++");
								}
								else
								{
									AddTabularText("MOSI-", obj_str[ABCC_MOSI_CHANNEL], inst_str[ABCC_MOSI_CHANNEL], cmd_str[ABCC_MOSI_CHANNEL], ext_str[ABCC_MOSI_CHANNEL]);
								}
							}
						}
						return;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

void SpiAnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/)  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SpiAnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/)  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}
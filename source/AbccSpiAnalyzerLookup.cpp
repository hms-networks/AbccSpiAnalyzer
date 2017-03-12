/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerLookup.h
**    Summary: Lookup Table routines for DLL-Results
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include <iostream>
#include <sstream>
#include <AnalyzerHelpers.h>
#include "AbccSpiAnalyzer.h"
#include "AbccSpiAnalyzerResults.h"
#include "AbccSpiAnalyzerSettings.h"

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#include "abcc_abp/abp_add.h"
#include "abcc_abp/abp_asm.h"
#include "abcc_abp/abp_bac.h"
#include "abcc_abp/abp_ccl.h"
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
#include "abcc_abp/abp_epl.h"
#include "abcc_abp/abp_er.h"
#include "abcc_abp/abp_etn.h"
#include "abcc_abp/abp_fsi.h"
#include "abcc_abp/abp_fusm.h"
#include "abcc_abp/abp_mdd.h"
#include "abcc_abp/abp_mod.h"
//#include "abcc_abp/abp_nwccl.h" /* Currently not used */
//#include "abcc_abp/abp_nwdpv1.h" /* Currently not used */
#include "abcc_abp/abp_nwetn.h"
#include "abcc_abp/abp_nwpnio.h"
#include "abcc_abp/abp_pnio.h"
#include "abcc_abp/abp_safe.h"
#include "abcc_abp/abp_smtp.h"
#include "abcc_abp/abp_soc.h"
#include "abcc_abp/abp_sync.h"

#define IS_CMD_STANDARD(cmd)			(cmd > 0) && (cmd < 9)
#define IS_CMD_OBJECT_SPECIFIC(cmd) 	(((cmd >= 0x10) && (cmd <= 0x30)) || \
										(cmd == 0x3F))

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

const tAbccMosiInfo asMosiStates[] =
{
	{ e_ABCC_MOSI_IDLE,						"",			0 },
	{ e_ABCC_MOSI_SPI_CTRL,					"SPI_CTL",	1 },
	{ e_ABCC_MOSI_RESERVED1,				"RES",		1 },
	{ e_ABCC_MOSI_MSG_LEN,					"MSG_LEN",	2 },
	{ e_ABCC_MOSI_PD_LEN,					"PD_LEN",	2 },
	{ e_ABCC_MOSI_APP_STAT,					"APP_STS",	1 },
	{ e_ABCC_MOSI_INT_MASK,					"INT_MSK",	1 },
	{ e_ABCC_MOSI_WR_MSG_FIELD,				"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_size,		"MSG_SIZE",	ABCC_MSG_SIZE_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_res1,		"RES",		ABCC_MSG_RES1_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId,	"SRC_ID",	ABCC_MSG_SRC_ID_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_obj,		"OBJ",		ABCC_MSG_OBJ_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_inst,		"INST",		ABCC_MSG_INST_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd,		"CMD",		ABCC_MSG_CMD_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_res2,		"RES",		ABCC_MSG_RES2_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt,	"EXT",		ABCC_MSG_CMDEXT_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_data,		"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ e_ABCC_MOSI_WR_PD_FIELD,				"PD",		1 },
	{ e_ABCC_MOSI_CRC32,					"CRC32",	4 },
	{ e_ABCC_MOSI_PAD,						"PAD",		2 },
	{ e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid, "--", 1 }
};

const tAbccMisoInfo asMisoStates[] =
{
	{ e_ABCC_MISO_IDLE,						"",			0 },
	{ e_ABCC_MISO_Reserved1,				"RES",		1 },
	{ e_ABCC_MISO_Reserved2,				"RES",		1 },
	{ e_ABCC_MISO_LED_STAT,					"LED_STS",	2 },
	{ e_ABCC_MISO_ANB_STAT,					"ANB_STS",	1 },
	{ e_ABCC_MISO_SPI_STAT,					"SPI_STS",	1 },
	{ e_ABCC_MISO_NET_TIME,					"TIME",		4 },
	{ e_ABCC_MISO_RD_MSG_FIELD,				"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_size,		"MD_SIZE",	ABCC_MSG_SIZE_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_res1,		"RES",		ABCC_MSG_RES1_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_srcId,	"SRC_ID",	ABCC_MSG_SRC_ID_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_obj,		"OBJ",		ABCC_MSG_OBJ_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_inst,		"INST",		ABCC_MSG_INST_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_cmd,		"CMD",		ABCC_MSG_CMD_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_res2,		"RES",		ABCC_MSG_RES2_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt,	"EXT",		ABCC_MSG_CMDEXT_FIELD_SIZE },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_data,		"MD",		ABCC_MSG_DATA_FIELD_SIZE },
	{ e_ABCC_MISO_RD_PD_FIELD,				"PD",		1 },
	{ e_ABCC_MISO_CRC32,					"CRC32",	4 },
	{ e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid, "--", 1 }

};

const tAbccMsgInfo asMsgStates[] =
{
	{ e_ABCC_MSG_SIZE,		"MD_SIZE",	ABCC_MSG_SIZE_FIELD_SIZE },
	{ e_ABCC_MSG_RESERVED1,	"RES",		ABCC_MSG_RES1_FIELD_SIZE },
	{ e_ABCC_MSG_SOURCE_ID,	"SRC_ID",	ABCC_MSG_SRC_ID_FIELD_SIZE },
	{ e_ABCC_MSG_OBJECT,	"OBJ",		ABCC_MSG_OBJ_FIELD_SIZE },
	{ e_ABCC_MSG_INST,		"INST",		ABCC_MSG_INST_FIELD_SIZE },
	{ e_ABCC_MSG_CMD,		"CMD",		ABCC_MSG_CMD_FIELD_SIZE },
	{ e_ABCC_MSG_RESERVED2,	"RES",		ABCC_MSG_RES2_FIELD_SIZE },
	{ e_ABCC_MSG_CMD_EXT,	"EXT",		ABCC_MSG_CMDEXT_FIELD_SIZE },
	{ e_ABCC_MSG_DATA,		"MD",		ABCC_MSG_DATA_FIELD_SIZE }
};

static const tValueName asAddObjAttrNames[] =
{
	{ ABP_ADD_OA_MAX_INST,			"Max Instance",			false },
	{ ABP_ADD_OA_EXT_DIAG_OVERFLOW,	"Ext Diag Overflow",	false },
	{ ABP_ADD_OA_STATIC_DIAG,		"Static Diag",			false }
};

static const tValueName asAddInstAttrNames[] =
{
	{ ABP_ADD_IA_MODULE_NUMBER,		"Module Number",	false },
	{ ABP_ADD_IA_IO_TYPE,			"IO Type",			false },
	{ ABP_ADD_IA_CHANNEL_NUMBER,	"Channel Number",	false },
	{ ABP_ADD_IA_CHANNEL_TYPE,		"Channel Type",		false },
	{ ABP_ADD_IA_ERROR_TYPE,		"Error Type",		false }
};

static const tValueName asAnbInstAttrNames[] =
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
	{ ABP_ANB_IA_AUX_BIT_FUNC,		"Auxilary Bit Function",	false },
	{ ABP_ANB_IA_GPIO_CONFIG,		"GPIO Configuration",		false },
	{ ABP_ANB_IA_VIRTUAL_ATTRS,		"Virtual Attributes",		false },
	{ ABP_ANB_IA_BLACK_WHITE_LIST,	"Black/White List",			false },
	{ ABP_ANB_IA_NETWORK_TIME,		"Network Time",				false },
	{ ABP_ANB_IA_FW_CUST_VERSION,	"FW Custom Version",		false },
	{ ABP_ANB_IA_ABIP_LICENSE,		"Anybus IP License",		false }
};

static const tValueName asBacInstAttrNames[] =
{
	{ ABP_BAC_IA_OBJECT_NAME,			"Object Name",				false },
	{ ABP_BAC_IA_VENDOR_NAME,			"Vendor Name",				false },
	{ ABP_BAC_IA_VENDOR_IDENTIFIER,		"Vendor Identifier",		false },
	{ ABP_BAC_IA_MODEL_NAME,			"Model Name",				false },
	{ ABP_BAC_IA_FIRMWARE_REVISION,		"Firmware Revision",		false },
	{ ABP_BAC_IA_APP_SOFTWARE_VERSION,	"Software Revision",		false },
	{ ABP_BAC_IA_SUPPORT_ADV_MAPPING,	"Support Advanced Mapping",	false },
	{ ABP_BAC_IA_CURRENT_DATE_AND_TIME,	"Current Date and Time",	false },
	{ ABP_BAC_IA_PASSWORD,				"Password",					false }
};

static const tValueName asCclInstAttrNames[] =
{
	{ ABP_CCL_IA_VENDOR_CODE,			"Vendor Code",			false },
	{ ABP_CCL_IA_SOFTWARE_VERSION,		"Software Version",		false },
	{ ABP_CCL_IA_MODEL_CODE,			"Model Code",			false },
	{ ABP_CCL_IA_NETWORK_SETTINGS,		"Network Settings",		false },
	{ ABP_CCL_IA_SYS_AREA_HANDLER,		"System Area Handler",	false },
	{ ABP_CCL_IA_HOLD_CLEAR_SETTING,	"Hold Clear Setting",	false }
};

static const tValueName asCntInstAttrNames[] =
{
	{ ABP_CNT_IA_VENDOR_ID,					"Vendor ID",						false },
	{ ABP_CNT_IA_DEVICE_TYPE,				"Device Type",						false },
	{ ABP_CNT_IA_PRODUCT_CODE,				"Product Code",						false },
	{ ABP_CNT_IA_REVISION,					"Revision",							false },
	{ ABP_CNT_IA_SERIAL_NUMBER,				"Serial Number",					false },
	{ ABP_CNT_IA_PRODUCT_NAME,				"Product Name",						false },
	{ ABP_CNT_IA_PROD_INSTANCE,				"Producing Instance",				false },
	{ ABP_CNT_IA_CONS_INSTANCE,				"Consuming Instance",				false },
	{ ABP_CNT_IA_ENABLE_APP_CIP_OBJECTS,	"Enable Application CIP Objects",	false },
	{ ABP_CNT_IA_ENABLE_PARAM_OBJECT,		"Enable Parameter Object",			false },
	{ ABP_CNT_IA_CONFIG_INSTANCE,			"Configuration Instance",			false }
};

static const tValueName asCopInstAttrNames[] =
{
	{ ABP_COP_IA_VENDOR_ID,			"Vendor ID",				false },
	{ ABP_COP_IA_PRODUCT_CODE,		"Product Code",				false },
	{ ABP_COP_IA_MAJOR_REV,			"Major Revision",			false },
	{ ABP_COP_IA_MINOR_REV,			"Minor Revision",			false },
	{ ABP_COP_IA_SERIAL_NUMBER,		"Serial Number",			false },
	{ ABP_COP_IA_MANF_DEV_NAME,		"Manufacturer Device Name",	false },
	{ ABP_COP_IA_MANF_HW_VER,		"Manufacturer HW Version",	false },
	{ ABP_COP_IA_MANF_SW_VER,		"Manufacturer SW Version",	false }
};

static const tValueName asCpnInstAttrNames[] =
{
	{ ABP_CPN_IA_VENDOR_ID,					"Vendor ID",						false },
	{ ABP_CPN_IA_DEVICE_TYPE,				"Device Type",						false },
	{ ABP_CPN_IA_PRODUCT_CODE,				"Product Code",						false },
	{ ABP_CPN_IA_REVISION,					"Revision",							false },
	{ ABP_CPN_IA_SERIAL_NUMBER,				"Serial Number",					false },
	{ ABP_CPN_IA_PRODUCT_NAME,				"Product Name",						false },
	{ ABP_CPN_IA_PROD_INSTANCE,				"Producing Instance",				false },
	{ ABP_CPN_IA_CONS_INSTANCE,				"Consuming Instance",				false },
	{ ABP_CPN_IA_ENABLE_APP_CIP_OBJECTS,	"Enable Application CIP Objects",	false },
	{ ABP_CPN_IA_ENABLE_PARAM_OBJECT,		"Enable Parameter Object",			false },
	{ ABP_CPN_IA_BIT_SLAVE,					"Bit Slave",						false }
};

static const tValueName asDevInstAttrNames[] =
{
	{ ABP_DEV_IA_VENDOR_ID,					"Vendor ID",						false },
	{ ABP_DEV_IA_DEVICE_TYPE,				"Device Type",						false },
	{ ABP_DEV_IA_PRODUCT_CODE,				"Product Code",						false },
	{ ABP_DEV_IA_REVISION,					"Revision",							false },
	{ ABP_DEV_IA_SERIAL_NUMBER,				"Serial Number",					false },
	{ ABP_DEV_IA_PRODUCT_NAME,				"Product Name",						false },
	{ ABP_DEV_IA_PROD_INSTANCE,				"Producing Instance",				false },
	{ ABP_DEV_IA_CONS_INSTANCE,				"Consuming Instance",				false },
	{ ABP_DEV_IA_ADDRESS_FROM_NET,			"Address From Network",				false },
	{ ABP_DEV_IA_BAUD_RATE_FROM_NET,		"Baud Rate From Network",			false },
	{ ABP_DEV_IA_ENABLE_APP_CIP_OBJECTS,	"Enable Application CIP Objects",	false },
	{ ABP_DEV_IA_ENABLE_PARAM_OBJECT,		"Enable Parameter Object",			false },
	{ ABP_DEV_IA_ENABLE_QUICK_CONNECT,		"Enable QuickConnect",				false },
	{ ABP_DEV_IA_PREPEND_PRODUCING,			"Prepend Producing",				false },
	{ ABP_DEV_IA_PREPEND_CONSUMING,			"Prepend Consuming",				false },
	{ ABP_DEV_IA_ABCC_ADI_OBJECT,			"ABCC ADI Object",					false }
};

static const tValueName asDiObjAttrNames[] =
{
	{ ABP_DI_OA_MAX_INST,		"Maximum Number of Instances",	false },
	{ ABP_DI_OA_SUPPORT_FUNC,	"Supported Functionality",		false }
};

static const tValueName asDiInstAttrNames[] =
{
	{ ABP_DI_IA_SEVERITY,			"Severity",						false },
	{ ABP_DI_IA_EVENT_CODE,			"Event Code",					false },
	{ ABP_DI_IA_NW_SPEC_EVENT_INFO,	"Network Specific Event Info",	false },
	{ ABP_DI_IA_SLOT,				"Slot",							false },
	{ ABP_DI_IA_ADI,				"ADI",							false },
	{ ABP_DI_IA_ELEMENT,			"Element",						false },
	{ ABP_DI_IA_BIT,				"Bit",							false }
};

static const tValueName asDpv1InstAttrNames[] =
{
	{ ABP_DPV1_IA_IDENT_NUMBER,			"Identity Number",					false },
	{ ABP_DPV1_IA_PRM_DATA,				"Parameter Data",					false },
	{ ABP_DPV1_IA_EXPECTED_CFG_DATA,	"Expected Configuration Data",		false },
	{ ABP_DPV1_IA_SSA_ENABLED,			"SSA Enabled",						false },
	{ ABP_DPV1_IA_SIZEOF_ID_REL_DIAG,	"Size of ID Related Diagnostic",	false },
	{ ABP_DPV1_IA_BUFFER_MODE,			"Buffer Mode",						false },
	{ ABP_DPV1_IA_ALARM_SETTINGS,		"Alarm Setting",					false },
	{ ABP_DPV1_IA_MANUFACTURER_ID,		"Manufacturer ID",					false },
	{ ABP_DPV1_IA_ORDER_ID,				"Order ID",							false },
	{ ABP_DPV1_IA_SERIAL_NO,			"Serial Number",					false },
	{ ABP_DPV1_IA_HW_REV,				"Hardware Revision",				false },
	{ ABP_DPV1_IA_SW_REV,				"Software Revision",				false },
	{ ABP_DPV1_IA_REV_COUNTER,			"Revision Counter",					false },
	{ ABP_DPV1_IA_PROFILE_ID,			"Profile ID",						false },
	{ ABP_DPV1_IA_PROFILE_SPEC_TYPE,	"Profile Specific Type",			false },
	{ ABP_DPV1_IA_IM_VERSION,			"I&M Version",						false },
	{ ABP_DPV1_IA_IM_SUPPORTED,			"I&M Supported",					false },
	{ ABP_DPV1_IA_IM_HEADER,			"I&M Header",						false },
	{ ABP_DPV1_IA_CHK_CFG_BEHAVIOR,		"Check Configuration Behavior",		false },
	{ ABP_DPV1_IA_RESERVED,				"Reserved",							false }
};

static const tValueName asEcoObjAttrNames[] =
{
	{ ABP_ECO_OA_CURRENT_ENERGY_SAVING_MODE,	"Current Energy Saving Mode",			false },
	{ ABP_ECO_OA_REMAINING_TIME_TO_DEST,		"Remaining Time to Destination",		false },
	{ ABP_ECO_OA_ENERGY_CONSUMP_TO_DEST,		"Energy Consumption to Destination",	false }
};

static const tValueName asEcoInstAttrNames[] =
{
	{ ABP_ECO_IA_MODE_ATTRIBUTES,			"Mode Attributes",					false },
	{ ABP_ECO_IA_TIME_MIN_PAUSE,			"Time Min Pause",					false },
	{ ABP_ECO_IA_TIME_TO_PAUSE,				"Time To Pause",					false },
	{ ABP_ECO_IA_TIME_TO_OPERATE,			"Time To Operate",					false },
	{ ABP_ECO_IA_TIME_MIN_LENGTH_OF_STAY,	"Time Min Length Of Stay",			false },
	{ ABP_ECO_IA_TIME_MAX_LENGTH_OF_STAY,	"Time Max Length Of Stay",			false },
	{ ABP_ECO_IA_MODE_POWER_CONSUMP,		"Mode Power Consumption",			false },
	{ ABP_ECO_IA_ENERGY_CONSUMP_TO_PAUSE,	"Energy Consumption To Pause",		false },
	{ ABP_ECO_IA_ENERGY_CONSUMP_TO_OPERATE,	"Energy Consumption To Operate",	false },
	{ ABP_ECO_IA_AVAILABILITY,				"Availability",						false },
	{ ABP_ECO_IA_POWER_CONSUMPTION,			"Power Consumption",				false }
};

static const tValueName asErInstAttrNames[] =
{
	{ ABP_ER_IA_ENERGY_READING,				"Energy Reading",				false },
	{ ABP_ER_IA_DIRECTION,					"Direction",					false },
	{ ABP_ER_IA_ACCURACY,					"Accuracy",						false },
	{ ABP_ER_IA_CURRENT_POWER_CONSUMPTION,	"Current Power Consumption",	false },
	{ ABP_ER_IA_NOMINAL_POWER_CONSUMPTION,	"Nominal Power Consumption",	false },
};

static const tValueName asNwInstAttrNames[] =
{
	{ ABP_NW_IA_NW_TYPE,		"Network Type",				false },
	{ ABP_NW_IA_NW_TYPE_STR,	"Network Type String",		false },
	{ ABP_NW_IA_DATA_FORMAT,	"Data Format",		    	false },
	{ ABP_NW_IA_PARAM_SUPPORT,	"Parameter Support",		false },
	{ ABP_NW_IA_WRITE_PD_SIZE,	"Write Process Data Size",	false },
	{ ABP_NW_IA_READ_PD_SIZE,	"Read Process Data Size",	false },
	{ ABP_NW_IA_EXCEPTION_INFO,	"Exception Information",	false }
};

static const tValueName asSocObjAttrNames[] =
{
	{ ABP_SOC_OA_MAX_INST, "Maximum Number of Instances",	false }
};

static const tValueName asSocInstAttrNames[] =
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

static const tValueName asSmtpObjAttrNames[] =
{
	{ ABP_SMTP_OA_MAX_INST,		"Maximum Number of Instances",	false },
	{ ABP_SMTP_OA_EMAILS_SENT,	"Emails Sent",					false },
	{ ABP_SMTP_OA_EMAIL_FAILED,	"Emails Failed to Send",		false }
};

static const tValueName asSmtpInstAttrNames[] =
{
	{ ABP_SMTP_IA_FROM,		"From Address",		false },
	{ ABP_SMTP_IA_TO,		"To Address",		false },
	{ ABP_SMTP_IA_SUBJECT,	"Message Subject",  false },
	{ ABP_SMTP_IA_MESSAGE,	"Message Body",		false }
};

static const tValueName asNcInstAttrNames[] =
{
	{ ABP_NC_VAR_IA_NAME,			"Name",					false },
	{ ABP_NC_VAR_IA_DATA_TYPE,		"Data Type",			false },
	{ ABP_NC_VAR_IA_NUM_ELEM,		"Number of Elements",	false },
	{ ABP_NC_VAR_IA_DESCRIPTOR,		"Descriptor",			false },
	{ ABP_NC_VAR_IA_VALUE,			"Value",				false },
	{ ABP_NC_VAR_IA_CONFIG_VALUE,	"Configured Value",		false }
};

static const tValueName asNwEtnInstAttrNames[] =
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

static const tValueName asCpcObjAttrNames[] =
{
	{ ABP_CPC_OA_MAX_INST, "Maximum Number of Instances",	false }
};

static const tValueName asCpcInstAttrNames[] =
{
	{ ABP_CPC_IA_PORT_TYPE,			"Port Type",		false },
	{ ABP_CPC_IA_PORT_NUMBER,		"Port Number",		false },
	{ ABP_CPC_IA_LINK_PATH,			"Link Path",		false },
	{ ABP_CPC_IA_PORT_NAME,			"Port Name",		false },
	{ ABP_CPC_IA_NODE_ADDRESS,		"Node Address",		false },
	{ ABP_CPC_IA_PORT_NODE_RANGE,	"Port Node Range",	false }
};

static const tValueName asCipIdInstAttrNames[] =
{
	{ ABP_CIPID_IA_VENDOR_ID,		"Vendor ID",		false },
	{ ABP_CIPID_IA_DEVICE_TYPE,		"Device Type",		false },
	{ ABP_CIPID_IA_PRODUCT_CODE,	"Product Code",		false },
	{ ABP_CIPID_IA_REVISION,		"Revision",			false },
	{ ABP_CIPID_IA_STATUS,			"Status",			false },
	{ ABP_CIPID_IA_SERIAL_NUMBER,	"Serial Number",	false },
	{ ABP_CIPID_IA_PRODUCT_NAME,	"Product Name",		false }
};

static const tValueName asEplInstAttrNames[] =
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

static const tValueName asPnioInstAttrNames[] =
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

static const tValueName asEipInstAttrNames[] =
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

static const tValueName asEtcInstAttrNames[] =
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
	{ ABP_ECT_IA_SET_DEV_ID_AS_CSA,		"Set Device ID as Configured Station Alias",false },
	{ ABP_ECT_IA_ETHERCAT_STATE,		"EtherCAT State",							false },
	{ ABP_ECT_IA_STATE_TIMEOUTS,		"State Timeouts",							false },
};

static const tValueName asAppdObjAttrNames[] =
{
	{ ABP_APPD_OA_NR_READ_PD_MAPPABLE_INSTANCES,	"No. of RD PD Mappable Instances",	false },
	{ ABP_APPD_OA_NR_WRITE_PD_MAPPABLE_INSTANCES,	"No. of WR PD Mappable Instances",	false },
	{ ABP_APPD_OA_NR_NV_INSTANCES,					"No. of Non-Volatile Instances",	false }
};

static const tValueName asAppdInstAttrNames[] =
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

static const tValueName asAppInstAttrNames[] =
{
	{ ABP_APP_IA_CONFIGURED,	"Configured",						false },
	{ ABP_APP_IA_SUP_LANG,		"Supported Languages",				false },
	{ ABP_APP_IA_SER_NUM,		"Serial Number",					false },
	{ ABP_APP_IA_PAR_CRTL_SUM,	"Parameter Control Sum",			false },
	{ ABP_APP_IA_FW_AVAILABLE,	"Candidate Firmware Available",		false },
	{ ABP_APP_IA_HW_CONF_ADDR,	"Hardware Configurable Address",	false }
};

static const tValueName asFsiObjAttrNames[] =
{
	{ ABP_FSI_OA_MAX_INST,						"Max Number of Instances",		false },
	{ ABP_FSI_OA_DISABLE_VFS,					"Disable Virtual File System",	false },
	{ ABP_FSI_OA_TOTAL_DISC_SIZE,				"Total Disc Size",				false },
	{ ABP_FSI_OA_FREE_DISC_SIZE,				"Free Disc Size",				false },
	{ ABP_FSI_OA_DISC_CRC,						"Disc CRC",						false },
	{ ABP_FSI_OA_DISC_TYPE,						"Disc Type",					false },
	{ ABP_FSI_OA_DISC_FAULT_TOLERANCE_LEVEL,	"Disc Fault Tolerance Level",	false }
};

static const tValueName asFsiInstAttrNames[] =
{
	{ ABP_FSI_IA_TYPE,		"Instance Type",			false },
	{ ABP_FSI_IA_FILE_SIZE,	"File Size",				false },
	{ ABP_FSI_IA_PATH,		"Current Instance Path",	false }
};

static const tValueName asAsmObjAttrNames[] =
{
	{ ABP_ASM_OA_WRITE_PD_INST_LIST,	"Write PD Instance List",	false },
	{ ABP_ASM_OA_READ_PD_INST_LIST,		"Read PD Instance List",	false }
};

static const tValueName asAsmInstAttrNames[] =
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

static const tValueName asMddObjAttrNames[] =
{
	{ ABP_MDD_OA_NUM_SLOTS,			"Number of Slots",			false },
	{ ABP_MDD_OA_NUM_ADIS_PER_SLOT,	"Number of ADIs Per Slot",	false }
};

static const tValueName asSyncInstAttrNames[] =
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

static const tValueName asFusmInstAttrNames[] =
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

static const tValueName asSafeInstAttrNames[] =
{
	{ ABP_SAFE_IA_SAFETY_ENABLE,	"Safety Enabled",		false },
	{ ABP_SAFE_IA_BAUD_RATE,		"Baud Rate",			false },
	{ 0x03,							"I/O Configuration",	false } /* TODO: ABP macro does not exist yet */
};


static const tValueName asObjAttrNames[] =
{
	{ ABP_OA_NAME,			"Name",						false },
	{ ABP_OA_REV,			"Revision",					false },
	{ ABP_OA_NUM_INST,		"Number of Instances",		false },
	{ ABP_OA_HIGHEST_INST,	"Highest Instance Number",	false }
};

static const tValueName asObjectNames[] =
{
	/*------------------------------------------------------------------------------
	** Anybus module objects
	**------------------------------------------------------------------------------
	*/
	{ ABP_OBJ_NUM_ANB,		"Anybus Object",						false },
	{ ABP_OBJ_NUM_DI,		"Diagnostic Object",					false },
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

static const tValueName asCmdNames[] =
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

static const tValueName asAddCmdNames[] =
{
	{ ABP_ADD_CMD_ALARM_NOTIFICATION,	"Alarm_Notification",	true }
};

static const tValueName asBacCmdNames[] =
{
	{ ABP_BAC_CMD_GET_ADI_BY_BACNET_OBJ_INST,		"Get_Adi_By_BacNet_Obj_Inst",		false },
	{ ABP_BAC_CMD_GET_ADI_BY_BACNET_OBJ_INST_NAME,	"Get_Adi_By_BacNet_Obj_Inst_Name",	false },
	{ ABP_BAC_CMD_GET_ALL_BACNET_OBJ_INSTANCES,		"Get_All_BacNet_Obj_Instances",		false },
	{ ABP_BAC_CMD_GET_BACNET_OBJ_INST_BY_ADI,		"Get_BacNet_Obj_Inst_By_Adi",		false }
};

static const tValueName asCclCmdNames[] =
{
	{ ABP_CCL_CMD_INITIAL_DATA_SETTING_NOTIFICATION,				"Initial_Data_Setting_Notfication",					false },
	{ ABP_CCL_CMD_INITIAL_DATA_PROCESSING_COMPLETED_NOTIFICATION,	"Initial_Data_Processing_Completed_Notfication",	false }
};

static const tValueName asCntCmdNames[] =
{
	{ ABP_CNT_CMD_PROCESS_CIP_OBJ_REQUEST,	"Process_CIP_Obj_Request",	false },
	{ ABP_CNT_CMD_SET_CONFIG_DATA,			"Set_Config_Data",			false },
	{ ABP_CNT_CMD_GET_CONFIG_DATA,			"Get_Config_Data",			false },
};

static const tValueName asCpnCmdNames[] =
{
	{ ABP_CPN_CMD_PROCESS_CIP_OBJ_REQUEST,	"Process_CIP_Obj_Request",	false }
};

static const tValueName asDevCmdNames[] =
{
	{ ABP_DEV_CMD_PROCESS_CIP_OBJ_REQUEST,	"Process_CIP_Obj_Request",	false }
};

static const tValueName asDpv1CmdNames[] =
{
	{ ABP_DPV1_CMD_GET_IM_RECORD,	"Get_IM_Record",	false },
	{ ABP_DPV1_CMD_SET_IM_RECORD,	"Set_IM_Record",	false },
	{ ABP_DPV1_CMD_ALARM_ACK,		"Alarm_Ack",		false },
	{ ABP_DPV1_CMD_GET_RECORD,		"Get_Record",		false },
	{ ABP_DPV1_CMD_SET_RECORD,		"Set_Record",		false }
};

static const tValueName asEcoCmdNames[] =
{
	{ ABP_ECO_CMD_START_PAUSE,			"Start_Pause",			false },
	{ ABP_ECO_CMD_END_PAUSE,			"End_Pause",			false },
	{ ABP_ECO_CMD_PREVIEW_PAUSE_TIME,	"Preview_Pause_Time",	false }
};

static const tValueName asMddCmdNames[] =
{
	{ ABP_MDD_CMD_GET_LIST, "Get_List",	false }
};

static const tValueName asAsmCmdNames[] =
{
	{ ABP_ASM_CMD_WRITE_ASSEMBLY_DATA,	"Write_Assembly_Data",	false },
	{ ABP_ASM_CMD_READ_ASSEMBLY_DATA,	"Read_Assembly_Data",	false }
};

static const tValueName asFusmCmdNames[] =
{
	{ ABP_FUSM_CMD_ERROR_CONFIRMATION,	"Error_Confirmation",	true }
};

static const tValueName asFsiCmdNames[] =
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

static const tValueName asEipCmdNames[] =
{
	{ ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST,		"Process_CIP_Obj_Request",		false },
	{ ABP_EIP_CMD_SET_CONFIG_DATA,				"Set_Config_Data",				false },
	{ ABP_EIP_CMD_PROCESS_CIP_ROUTING_REQUEST,	"Process_CIP_Routing_Request",	false },
	{ ABP_EIP_CMD_GET_CONFIG_DATA,				"Get_Config_Data",				false },
	{ ABP_EIP_CMD_PROCESS_CIP_OBJ_REQUEST_EXT,	"Process_CIP_Obj_Request_Ext",	false }
};

static const tValueName asEctCmdNames[] =
{
	{ ABP_ECT_CMD_GET_OBJECT_DESC,	"Get_Object_Description",	false }
};

static const tValueName asPnioCmdNames[] =
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

static const tValueName asAppCmdNames[] =
{
	{ ABP_APP_CMD_RESET_REQUEST,		"Reset_Request",			false },
	{ ABP_APP_CMD_CHANGE_LANG_REQUEST,	"Change_Language_Request",	false },
	{ ABP_APP_CMD_RESET_DIAGNOSTIC,		"Reset_Diagnostic",			false }
};

static const tValueName asAppDataCmdNames[] =
{
	{ ABP_APPD_CMD_GET_INST_BY_ORDER,		"Get_Instance_Number_By_Order",	false },
	{ ABP_APPD_GET_PROFILE_INST_NUMBERS,	"Get_Profile_Inst_Numbers",		false },
	{ ABP_APPD_GET_ADI_INFO,				"Get_ADI_Info (Deprecated)",	true  }, /* ABCC40 deprecated shall not be used */
	{ ABP_APPD_REMAP_ADI_WRITE_AREA,		"Remap_ADI_Write_Area",			false },
	{ ABP_APPD_REMAP_ADI_READ_AREA,			"Remap_Adi_Read_Area",			false },
	{ ABP_APPD_GET_INSTANCE_NUMBERS,		"Get_Instance_Numbers",			false }
};

static const tValueName asNetCmdNames[] =
{
	{ ABP_NW_CMD_MAP_ADI_WRITE_AREA,		"Map_ADI_Write_Area",		false },
	{ ABP_NW_CMD_MAP_ADI_READ_AREA,			"Map_ADI_Read_Area",		false },
	{ ABP_NW_CMD_MAP_ADI_WRITE_EXT_AREA,	"Map_ADI_Write_Ext_Area",	false },
	{ ABP_NW_CMD_MAP_ADI_READ_EXT_AREA,		"Map_ADI_Read_Ext_Area",	false }
};

static const tValueName asAnbErrNames[] =
{
	{ ABP_ANB_ERR_INV_PRD_CFG,		"Invalid process data config",		true },
	{ ABP_ANB_ERR_INV_DEV_ADDR,		"Invalid device address",			true },
	{ ABP_ANB_ERR_INV_COM_SETTINGS,	"Invalid communication settings",	true }
};

static const tValueName asBacErrNames[] =
{
	{ ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_AV,		"Could not read object instance AV",		true },
	{ ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_BV,		"Could not read object instance BV",		true },
	{ ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_MSV,		"Could not read object instance MSV",		true },
	{ ABP_BAC_EXCPT_INFO_COULD_NOT_READ_OBJ_INST_BY_ADI,	"Could not read object instance by ADI",	true }
};

static const tValueName asCntErrNames[] =
{
	{ ABP_CNT_NW_EXCPT_INFO_INVALID_SY_INST,	"Invalid SY Instance",	true }
};

static const tValueName asCpnErrNames[] =
{
	{ ABP_CPN_NW_EXCPT_INFO_INVALID_SY_INST,	"Invalid SY Instance",	true }
};

static const tValueName asDevErrNames[] =
{
	{ ABP_DEV_NW_EXCPT_INFO_INVALID_SY_INST,	"Invalid SY Instance",	true }
};

static const tValueName asDiErrNames[] =
{
	{ ABP_DI_ERR_NOT_REMOVED,		"Event could not be removed",		true },
	{ ABP_DI_LATCH_NOT_SUPPORTED,	"Latching events not supported",	true },
	{ ABP_DI_ERR_NW_SPECIFIC,		"Network specific error",			true }
};

static const tValueName asNwErrNames[] =
{
	{ ABP_NW_ERR_INVALID_ADI_DATA_TYPE,	"Invalid ADI data type",		true },
	{ ABP_NW_ERR_INVALID_NUM_ELEMENTS,	"Invalid number of elements",	true },
	{ ABP_NW_ERR_INVALID_TOTAL_SIZE,	"Invalid total size",			true },
	{ ABP_NW_ERR_MULTIPLE_MAPPING,		"Multiple mapping",				true },
	{ ABP_NW_ERR_INVALID_ORDER_NUM,		"Invalid ADI order number",		true },
	{ ABP_NW_ERR_INVALID_MAP_CMD_SEQ,	"Invalid map cmd sequence",		true },
	{ ABP_NW_ERR_INVALID_MAP_CMD,		"Command impossible to parse",	true },
	{ ABP_NW_ERR_BAD_ALIGNMENT,			"Invalid data alignment",		true },
	{ ABP_NW_ERR_INVALID_ADI_0,			"Invalid use of ADI 0",			true },
	{ ABP_NW_ERR_NW_SPEC_RESTRICTION,	"Network specific restriction",	true }
};

static const tValueName asAppdErrNames[] =
{
	{ ABP_APPD_ERR_MAPPING_ITEM_NAK,				"Mapping item NAK",				true },
	{ ABP_APPD_ERR_INVALID_TOTAL_SIZE,				"Invalid total size",			true },
	{ ABP_APPD_ERR_ATTR_CTRL_FROM_OTHER_CHANNEL,	"Attr ctrl from other channel",	true }
};

static const tValueName asSmtpErrNames[] =
{
	{ ABP_SMTP_NO_EMAIL_SERVER,			"No e-mail server",			true },
	{ ABP_SMTP_SERVER_NOT_READY,		"Server not ready",			true },
	{ ABP_SMTP_AUTHENTICATION_ERROR,	"Authentication error",		true },
	{ ABP_SMTP_SOCKET_ERROR,			"Socket error",				true },
	{ ABP_SMTP_SSI_SCAN_ERROR,			"SSI scan error",			true },
	{ ABP_SMTP_FILE_ERROR,				"File error",				true },
	{ ABP_SMTP_OTHER,					"Other",					true }
};

static const tValueName asEipErrNames[] =
{
	{ ABP_EIP_ERR_OWNERSHIP_CONFLICT,	"Ownership conflict",		true },
	{ ABP_EIP_ERR_INVALID_CONFIG,		"Invalid configuration",	true }
};

static const tValueName asPnioErrNames[] =
{
	{ ABP_NWPNIO_ERR_ADI_WRITE_NOT_MAPPED,		"ADI write not mapped",				true },
	{ ABP_NWPNIO_ERR_ADI_READ_NOT_MAPPED,		"ADI read not mapped",				true },
	{ ABP_NWPNIO_ERR_ADI_ELEM_NOT_PRESENT,		"Element is not present",			true },
	{ ABP_NWPNIO_ERR_ADI_ALREADY_MAPPED,		"ADI is already mapped",			true },
	{ ABP_NWPNIO_ERR_API_0_NOT_ADDED,			"API 0 not added",					true },
	{ ABP_NWPNIO_ERR_API_NOT_PRESENT,			"API not present",					true },
	{ ABP_NWPNIO_ERR_API_ALREADY_PRESENT,		"API already present",				true },
	{ ABP_NWPNIO_ERR_API_CANNOT_BE_ADDED,		"API cannot be added",				true },
	{ ABP_NWPNIO_ERR_NO_IO_IN_SLOT_0,			"No I/O in slot 0",					true },
	{ ABP_NWPNIO_ERR_SLOT_0_NOT_PROP_PLUGGED,	"Slot 0 not properly plugged",		true },
	{ ABP_NWPNIO_ERR_SLOT_OCCUPIED,				"Slot occupied",					true },
	{ ABP_NWPNIO_ERR_SUBSLOT_OCCUPIED,			"Subslot occupied",					true },
	{ ABP_NWPNIO_ERR_NO_MODULE_SPECIFIED_SLOT,	"No module specified slot",			true },
	{ ABP_NWPNIO_ERR_NO_SUBMOD_SPECIFIED_SLOT,	"No submodule specified slot",		true },
	{ ABP_NWPNIO_ERR_SLOT_OUT_OF_RANGE,			"Slot out of range",				true },
	{ ABP_NWPNIO_ERR_SUBSLOT_OUT_OF_RANGE,		"Subslot out of range",				true },
	{ ABP_NWPNIO_ERR_AR_NOT_VALID,				"AR not valid",						true },
	{ ABP_NWPNIO_ERR_NO_PEND_APPL_READY,		"No pend appl ready",				true },
	{ ABP_NWPNIO_ERR_UNKNOWN_STACK_ERROR,		"Unknown stack error",				true },
	{ ABP_NWPNIO_ERR_MAX_NBR_OF_PLUGGED_SUBMOD,	"Max number of plugged submodules",	true },
	{ ABP_NWPNIO_ERR_SAFETY_NOT_ENABLED,		"Safety not enabled",				true },
	{ ABP_NWPNIO_ERR_ADI_DATATYPE_CONSTRAINT,	"ADI datatype constraint",			true }
};

static const tValueName asSocErrNames[] =
{
	{ SOC_ERR_ENOBUFS,			"ENOBUFS",			true },
	{ SOC_ERR_ETIMEDOUT,		"ETIMEDOUT",		true },
	{ SOC_ERR_EISCONN,			"EISCONN",			true },
	{ SOC_ERR_EOPNOTSUPP,		"EOPNOTSUPP",		true },
	{ SOC_ERR_ECONNABORTED,		"ECONNABORTED",		true },
	{ SOC_ERR_EWOULDBLOCK,		"EWOULDBLOCK",		true },
	{ SOC_ERR_ECONNREFUSED,		"ECONNREFUSED",		true },
	{ SOC_ERR_ECONNRESET,		"ECONNRESET",		true },
	{ SOC_ERR_ENOTCONN,			"ENOTCONN",			true },
	{ SOC_ERR_EALREADY,			"EALREADY",			true },
	{ SOC_ERR_EINVAL,			"EINVAL",			true },
	{ SOC_ERR_EMSGSIZE,			"EMSGSIZE",			true },
	{ SOC_ERR_EPIPE,			"EPIPE",			true },
	{ SOC_ERR_EDESTADDRREQ,		"EDESTADDRREQ",		true },
	{ SOC_ERR_ESHUTDOWN,		"ESHUTDOWN",		true },
	{ SOC_ERR_EHAVEOOB,			"EHAVEOOB",			true },
	{ SOC_ERR_ENOMEM,			"ENOMEM",			true },
	{ SOC_ERR_EADDRNOTAVAIL,	"EADDRNOTAVAIL",	true },
	{ SOC_ERR_EADDRINUSE,		"EADDRINUSE",		true },
	{ SOC_ERR_EINPROGRESS,		"EINPROGRESS",		true },
	{ SOC_ERR_ETOOMANYREFS,		"ETOOMANYREFS",		true },
	{ SOC_ERR_CMD_ABORTED,		"CMD_ABORTED",		true },
	{ SOC_ERR_DNS_NAME,			"DNS_NAME",			true },
	{ SOC_ERR_DNS_TIMEOUT,		"DNS_TIMEOUT",		true },
	{ SOC_ERR_DNS_CMD_FAILED,	"DNS_CMD_FAILED",	true }
};

static const tValueName asFusmErrNames[] =
{
	{ ABP_FUSM_ERR_REJECT_BY_MODULE,	"Rejected by module",			true },
	{ ABP_FUSM_ERR_MODULE_RSP_FAULTY,	"Module response is faulty",	true }
};


static const tValueName asFsiErrNames[] =
{
	{ ABP_FSI_ERR_FILE_OPEN_FAILED,				"File_Open Failed",			true },
	{ ABP_FSI_ERR_FILE_CLOSE_FAILED,			"File_Close Failed",		true },
	{ ABP_FSI_ERR_FILE_DELETE_FAILED,			"File_Delete Failed",		true },
	{ ABP_FSI_ERR_DIRECTORY_OPEN_FAILED,		"Directory_Open Failed",	true },
	{ ABP_FSI_ERR_DIRECTORY_CLOSE_FAILED,		"Directory_Close Failed",	true },
	{ ABP_FSI_ERR_DIRECTORY_CREATE_FAILED,		"Directory_Create Failed",	true },
	{ ABP_FSI_ERR_DIRECTORY_DELETE_FAILED,		"Directory_Delete Failed",	true },
	{ ABP_FSI_ERR_DIRECTORY_CHANGE_FAILED,		"Directory_Change Failed",	true },
	{ ABP_FSI_ERR_FILE_COPY_OPEN_READ_FAILED,	"Copy Open Read Failed",	true },
	{ ABP_FSI_ERR_FILE_COPY_OPEN_WRITE_FAILED,	"Copy Open Write Failed",	true },
	{ ABP_FSI_ERR_FILE_COPY_WRITE_FAILED,		"Copy Write Failed",		true },
	{ ABP_FSI_ERR_FILE_RENAME_FAILED,			"File_Rename Failed",		true }
};

static const tValueName asIntMaskNames[] =
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

static const tValueName asLedStsNames[] =
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

static const tValueName asErrorRspNames[] =
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
	{ ABP_ERR_PROTECTED_ACCESS,					"Protected access",									true },
	{ 0xFF,										"Object specific error",							true }
};

static const tValueName asAnybusStsNames[] =
{
	{ ABP_ANB_STATE_SETUP,			"SETUP",			false },
	{ ABP_ANB_STATE_NW_INIT,		"NW_INIT",			false },
	{ ABP_ANB_STATE_WAIT_PROCESS,	"WAIT_PROCESS",		false },
	{ ABP_ANB_STATE_IDLE,			"IDLE",				false },
	{ ABP_ANB_STATE_PROCESS_ACTIVE, "PROCESS_ACTIVE",	false },
	{ ABP_ANB_STATE_ERROR,			"ERROR",			true  },
	{ ABP_ANB_STATE_EXCEPTION,		"EXCEPTION",		true  }
};

static const tValueName asApplStsNames[] =
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

static const tValueName asSpiStsNames[] =
{
	{ 0xC0,							"RESERVED",		true  }, /* No ABP mask exists */
	{ ABP_SPI_STATUS_NEW_PD,		"NEW_PD",		false },
	{ ABP_SPI_STATUS_LAST_FRAG,		"LAST_FRAG",	false },
	{ ABP_SPI_STATUS_M,				"M",			false },
	{ ABP_SPI_STATUS_CMDCNT,		"CMDCNT",		false },
	{ ABP_SPI_STATUS_WRMSG_FULL,	"WRMSG_FULL",	true  }
};

static const tValueName asSpiCtrlNames[] =
{
	{ ABP_SPI_CTRL_T,			"TOGGLE",		false },
	{ 0x60,						"RESERVED",		true  }, /* No ABP mask exists */
	{ ABP_SPI_CTRL_LAST_FRAG,	"LAST_FRAG",	false },
	{ ABP_SPI_CTRL_M,			"M",			false },
	{ ABP_SPI_CTRL_CMDCNT,		"CMDCNT",		false },
	{ ABP_SPI_CTRL_WRPD_VALID,	"WRPD_VALID",	false }
};

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

bool GetObjSpecificErrString(U8 val, char* str, U16 maxLen, const tValueName* pasErrNames, U8 bNoErrors, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	for (U8 i = 0; i < bNoErrors; i++)
	{
		if (pasErrNames[i].value == val)
		{
			SNPRINTF(str, maxLen, pasErrNames[i].name);
			return pasErrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, sizeof(number_str));
	SNPRINTF(str, maxLen, "Unknown: %s", number_str);
	return true;
}

bool GetErrorRspString(U8 obj, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	switch (obj)
	{
	case ABP_OBJ_NUM_FSI:
	case ABP_OBJ_NUM_AFSI:
		/* (Application/Anybus) File System Interface Object */
		GetObjSpecificErrString(val, str, maxLen, &asFsiErrNames[0],
			(sizeof(asFsiErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_FUSM:
		/* Functional Safety Module Object */
		GetObjSpecificErrString(val, str, maxLen, &asFusmErrNames[0],
			(sizeof(asFusmErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_ANB:
		/* Anybus Object */
		GetObjSpecificErrString(val, str, maxLen, &asAnbErrNames[0],
			(sizeof(asAnbErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_BAC:
		/* BacNet Object */
		GetObjSpecificErrString(val, str, maxLen, &asBacErrNames[0],
			(sizeof(asBacErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_CNT:
		/* ControlNet Object */
		GetObjSpecificErrString(val, str, maxLen, &asCntErrNames[0],
			(sizeof(asCntErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_CPN:
		/* CompoNet Object */
		GetObjSpecificErrString(val, str, maxLen, &asCpnErrNames[0],
			(sizeof(asCpnErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_DEV:
		/* DeviceNet Object */
		GetObjSpecificErrString(val, str, maxLen, &asDevErrNames[0],
			(sizeof(asDevErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_DI:
		/* Diagnostic Object */
		GetObjSpecificErrString(val, str, maxLen, &asDiErrNames[0],
			(sizeof(asDiErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_NW:
		/* Network Object */
		GetObjSpecificErrString(val, str, maxLen, &asNwErrNames[0],
			(sizeof(asNwErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_APPD:
		/* Application Data Object */
		GetObjSpecificErrString(val, str, maxLen, &asAppdErrNames[0],
			(sizeof(asAppdErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_SMTP:
		/* SMTP Object */
		GetObjSpecificErrString(val, str, maxLen, &asSmtpErrNames[0],
			(sizeof(asSmtpErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_SOC:
		/* Socket Object */
		GetObjSpecificErrString(val, str, maxLen, &asSocErrNames[0],
			(sizeof(asSocErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_EIP:
		/* EtherNet/IP Object */
		GetObjSpecificErrString(val, str, maxLen, &asEipErrNames[0],
			(sizeof(asEipErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_PNIO:
		/* PROFINET IO Object */
		GetObjSpecificErrString(val, str, maxLen, &asPnioErrNames[0],
			(sizeof(asPnioErrNames) / sizeof(tValueName)), display_base);
		break;
	case ABP_OBJ_NUM_CCL:
		/* CC-Link Object */
	case ABP_OBJ_NUM_DPV1:
		/* DPV1 Object */
	default:
		AnalyzerHelpers::GetNumberString(val, display_base, 8, number_str, maxLen);
		SNPRINTF(str, maxLen, "Unknown: 0x%02X, %s", obj, number_str);
		break;
	}
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
	const tValueName* pasObjNames, U8 NoObjNames,
	const tValueName* pasInstNames, U8 NoInstNames)
{
	char number_str[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	const tValueName* pasAttrNames = NULL;
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

bool GetObjSpecificCmdString(U8 val, char* str, U16 maxLen, const tValueName* pasCmdNames, U8 bNoCmds, DisplayBase display_base)
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
		case ABP_OBJ_NUM_ADD:
			/* Additional Diagnostic Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asAddCmdNames[0], (sizeof(asAddCmdNames) / sizeof(tValueName)), display_base);
			break;
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
		case ABP_OBJ_NUM_BAC:
			/* BacNet Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asBacCmdNames[0], (sizeof(asBacCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_CCL:
			/* CC-Link Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asCclCmdNames[0], (sizeof(asCclCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_CNT:
			/* ControlNet Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asCntCmdNames[0], (sizeof(asCntCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_CPN:
			/* CompoNet Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asCpnCmdNames[0], (sizeof(asCpnCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_DEV:
			/* DeviceNet Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asDevCmdNames[0], (sizeof(asDevCmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_DPV1:
			/* DPV1 Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asDpv1CmdNames[0], (sizeof(asDpv1CmdNames) / sizeof(tValueName)), display_base);
			break;
		case ABP_OBJ_NUM_ECO:
			/* Energy Control Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asEcoCmdNames[0], (sizeof(asEcoCmdNames) / sizeof(tValueName)), display_base);
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
		case ABP_OBJ_NUM_ECT:
			/* EtherNet/IP Object */
			alert = GetObjSpecificCmdString(cmd, strBuffer, sizeof(strBuffer),
				&asEctCmdNames[0], (sizeof(asEctCmdNames) / sizeof(tValueName)), display_base);
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
			alert = true; //TODO: We only alert here because we have not implemented all object specific commands yet.
			break;
		}
		SNPRINTF(str, maxLen, "%s", strBuffer);
	}
	else
	{
		AnalyzerHelpers::GetNumberString(cmd, display_base, 8, strBuffer, sizeof(strBuffer));
		SNPRINTF(str, maxLen, "Reserved: %s", strBuffer);
		alert = true;
	}

	return alert;
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
	case ABP_OBJ_NUM_ADD:
		/* Additional Diagnostic Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asAddObjAttrNames[0], sizeof(asAddObjAttrNames) / sizeof(tValueName),
			&asAddInstAttrNames[0], sizeof(asAddInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_ANB:
		/* Anybus Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asAnbInstAttrNames[0], sizeof(asAnbInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_BAC:
		/* BacNet Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asBacInstAttrNames[0], sizeof(asBacInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_CCL:
		/* CC-Link Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asCclInstAttrNames[0], sizeof(asCclInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_CNT:
		/* ControlNet Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asCntInstAttrNames[0], sizeof(asCntInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_COP:
		/* CANopen Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asCopInstAttrNames[0], sizeof(asCopInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_CPN:
		/* CompoNet Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asCpnInstAttrNames[0], sizeof(asCpnInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_DEV:
		/* DeviceNet Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asDevInstAttrNames[0], sizeof(asDevInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_DI:
		/* Diagnostic Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asDiObjAttrNames[0], sizeof(asDiObjAttrNames) / sizeof(tValueName),
			&asDiInstAttrNames[0], sizeof(asDiInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_DPV1:
		/* DPV1 Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asDpv1InstAttrNames[0], sizeof(asDpv1InstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_ECO:
		/* Energy Control Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			&asEcoObjAttrNames[0], sizeof(asEcoObjAttrNames) / sizeof(tValueName),
			&asEcoInstAttrNames[0], sizeof(asEcoInstAttrNames) / sizeof(tValueName));
		break;
	case ABP_OBJ_NUM_ER:
		/* Energy Reporting Object */
		*pAlert = GetNamedAttrString(inst, (U8)val, &str[ofst], maxlen, display_base,
			NULL, 0, &asErInstAttrNames[0], sizeof(asErInstAttrNames) / sizeof(tValueName));
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

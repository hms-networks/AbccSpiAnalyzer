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

#define IS_MISO_FRAME ((frame.mFlags & SPI_MOSI_FLAG)!=SPI_MOSI_FLAG)
#define IS_MOSI_FRAME ((frame.mFlags & SPI_MOSI_FLAG)==SPI_MOSI_FLAG)

typedef struct
{
	U8 value;
	char* name;
	bool alert;
}t_ValueName;

t_ValueName sAnbInstAttrNames[] =
{
	{ 0x01, "Module Type",				false },
	{ 0x02, "Firmware Version",			false },
	{ 0x03, "Serial Number",		    false },
	{ 0x04, "Watchdog Timeout",			false },
	{ 0x05, "Setup Complete",			false },
	{ 0x06, "Exception",				false },
	{ 0x07, "Fatal Event",				false },
	{ 0x08, "Error Counters",			false },
	{ 0x09, "Language",					false },
	{ 0x0A, "Provider ID",				false },
	{ 0x0B, "Provider Info",			false },
	{ 0x0C, "LED Colors",				false },
	{ 0x0D, "LED Status",				false },
	{ 0x0E, "Switch Status",			false },
	{ 0x0F, "Auxilery Bit Function",	false },
	{ 0x10, "GPIO Configuration",		false },
	{ 0x11, "Virtual Attributes",		false },
	{ 0x12, "Black/White List",			false },
	{ 0x13, "Network Time",				false },
	{ 0x14, "FW Custom Version",		false }
};

t_ValueName sDiObjAttrNames[] =
{
	{ 0x0B, "Maximum Number of Instances",	false },
	{ 0x0C, "Supported Functionality",		false }
};

t_ValueName sDiInstAttrNames[] =
{
	{ 0x01, "Severity",						false },
	{ 0x02, "Event Code",					false },
	{ 0x03, "Network Specific Event Info",	false },
	{ 0x04, "Slot",							false },
	{ 0x05, "ADI",							false },
	{ 0x06, "Element",						false },
	{ 0x07, "Bit",							false }
};

t_ValueName sNwInstAttrNames[] =
{
	{ 0x01, "Network Type",				false },
	{ 0x02, "Network Type String",		false },
	{ 0x03, "Data Format",		    	false },
	{ 0x04, "Parameter Support",		false },
	{ 0x05, "Write Process Data Size",	false },
	{ 0x06, "Read Process Data Size",	false },
	{ 0x07, "Exception Information",	false }
};

t_ValueName sSocObjAttrNames[] =
{
	{ 0x0B, "Maximum Number of Instances",	false }
};

t_ValueName sSocInstAttrNames[] =
{
	{ 0x01, "Socket Type",			false },
	{ 0x02, "Port",					false },
	{ 0x03, "Host IP",		    	false },
	{ 0x04, "Host Port",			false },
	{ 0x05, "TCP State",			false },
	{ 0x06, "TCP RX Bytes",			false },
	{ 0x07, "TCP TX Bytes",			false },
	{ 0x08, "Reuse Address",		false },
	{ 0x09, "Keep Alive",			false },
	{ 0x0A, "IP Multicast TTL",		false },
	{ 0x0B, "IP Multicast Loop",	false },
	{ 0x0C, "Ack Delay Time",		false },
	{ 0x0D, "TCP No Delay",			false },
	{ 0x0E, "TCP Connect Timeout",	false }
};

t_ValueName sSmtpObjAttrNames[] =
{
	{ 0x0B, "Maximum Number of Instances",	false },
	{ 0x0C, "Success count",				false },
	{ 0x0D, "Error",						false }
};

t_ValueName sSmtpInstAttrNames[] =
{
	{ 0x01, "From",		false },
	{ 0x02, "To",		false },
	{ 0x03, "Subject",  false },
	{ 0x04, "Message",	false }
};

t_ValueName sNcInstAttrNames[] =
{
	{ 0x01, "Name",					false },
	{ 0x02, "Data Type",			false },
	{ 0x03, "Number of Elements",	false },
	{ 0x04, "Descriptor",			false },
	{ 0x05, "Value",				false },
	{ 0x06, "Configured Value",		false }
};

t_ValueName sEtnInstAttrNames[] =
{
	{ 0x01, "MAC Address",	false }
};

t_ValueName sCipObjAttrNames[] =
{
	{ 0x0B, "Maximum Number of Instances",	false }
};

t_ValueName sCipInstAttrNames[] =
{
	{ 0x01, "Port Type",		false },
	{ 0x02, "Port Number",		false },
	{ 0x03, "Link Path",		false },
	{ 0x04, "Port Name",		false },
	{ 0x07, "Node Address",		false },
	{ 0x08, "Port Node Range",	false }
};

t_ValueName sIdInstAttrNames[] =
{
	{ 0x01, "Vendor ID",		false },
	{ 0x02, "Device Type",		false },
	{ 0x03, "Product Code",		false },
	{ 0x04, "Revision",			false },
	{ 0x05, "Status",			false },
	{ 0x06, "Serial Number",	false },
	{ 0x07, "Product Name",		false }
};

t_ValueName sEipInstAttrNames[] =
{
	{ 0x01, "Vendor ID",										false },
	{ 0x02, "Device Type",										false },
	{ 0x03, "Product Code",		    							false },
	{ 0x04, "Revision",											false },
	{ 0x05, "Serial Number",									false },
	{ 0x06, "Product Name",										false },
	{ 0x07, "Producing Instance Number",						false },
	{ 0x08, "Consuming Instance Number",						false },
	{ 0x09, "Enable Communication Settings From Net",			false },
	{ 0x0A, "Enable CIP Forwarding",							false },
	{ 0x0C, "Enable Parameter Object",							false },
	{ 0x0D, "Input-Only Heartbeat Instance Number",				false },
	{ 0x0E, "Listen-Only Heartbeat Instance Number",			false },
	{ 0x0F, "Assembly Object Configuration Instance Number",	false },
	{ 0x10, "Disable Strict I/O Match",							false },
	{ 0x11, "Enable Unconnected Routing",						false },
	{ 0x12, "Input-Only Extended Heartbeat Instance Number",	false },
	{ 0x13, "Listen-Only Extended Heartbeat Instance Number",	false },
	{ 0x14, "Interface Label Port 1",							false },
	{ 0x15, "Interface Label Port 2",							false },
	{ 0x16, "Interface Label Internal Port",					false },
	{ 0x1A, "Enable EtherNet/IP QuickConnect",					false },
	{ 0x1D, "Ignore Sequence Count Check",						false },
	{ 0x1E, "ABCC ADI Object Number",							false }
};

t_ValueName sEtcInstAttrNames[] =
{
	{ 0x01, "Vendor ID",								false },
	{ 0x02, "Product Code",								false },
	{ 0x03, "Major Revision",	    					false },
	{ 0x04, "Minor Revision",							false },
	{ 0x05, "Serial Number",							false },
	{ 0x06, "Manufacturer Device Name",					false },
	{ 0x07, "Manufacturer Hardware Version",			false },
	{ 0x08, "Manufacturer Software Version",			false },
	{ 0x09, "ENUM ADIs",								false },
	{ 0x0A, "Device Type",								false },
	{ 0x0B, "Write PD Assembly Instance Translation",	false },
	{ 0x0C, "Read PD Assembly Instance Translation",	false },
	{ 0x0D, "ADI Translation",							false },
	{ 0x0F, "Object SubIndex Translation",				false },
	{ 0x10, "Enable FoE",								false }
};

t_ValueName sAppdObjAttrNames[] =
{
	{ 0x0B, "No. of RD PD Mappable Instances",	false },
	{ 0x0C, "No. of WR PD Mappable Instances",	false }
};

t_ValueName sAppdInstAttrNames[] =
{
	{ 0x01, "Name",						false },
	{ 0x02, "Data Type",				false },
	{ 0x03, "Number of Elements",		false },
	{ 0x04, "Descriptor",				false },
	{ 0x05, "Value(s)",					false },
	{ 0x06, "Max Value",				false },
	{ 0x07, "Min Value",				false },
	{ 0x08, "Default Value",			false },
	{ 0x09, "Number of SubElements",	false },
	{ 0x0A, "Element Name",				false }
};

t_ValueName sAppInstAttrNames[] =
{
	{ 0x01, "Configured",						false },
	{ 0x02, "Supported Languages",				false },
	{ 0x03, "Serial Number",					false },
	{ 0x04, "Parameter Control Sum",			false },
	{ 0x05, "Candidate Firmware Available",		false },
	{ 0x06, "Hardware Configurable Address",	false }
};

t_ValueName sFsiObjAttrNames[] =
{
	{ 0x0B, "Max Number of Instances",		false },
	{ 0x0C, "Disable Virtual File System",	false },
	{ 0x0D, "Total Disc Size",				false },
	{ 0x0E, "Free Disc Size",				false },
	{ 0x0F, "Disc CRC",						false }
};

t_ValueName sFsiInstAttrNames[] =
{
	{ 0x01, "Instance Type",	false },
	{ 0x02, "File Size",		false },
	{ 0x03, "Path",				false }
};

t_ValueName sAsmObjAttrNames[] =
{
	{ 0x0B, "Write PD Instance List",	false },
	{ 0x0C, "Read PD Instance List",	false }
};

t_ValueName sAsmInstAttrNames[] =
{
	{ 0x01, "Assembly Descriptor",	false },
	{ 0x02, "ADI Map 0",			false },
	{ 0x03, "ADI Map 1",			false },
	{ 0x04, "ADI Map 2",			false },
	{ 0x05, "ADI Map 3",			false },
	{ 0x06, "ADI Map 4",			false },
	{ 0x07, "ADI Map 5",			false },
	{ 0x08, "ADI Map 6",			false },
	{ 0x09, "ADI Map 7",			false },
	{ 0x0A, "ADI Map 8",			false },
	{ 0x0B, "ADI Map 9",			false },
	{ 0x0C, "ADI Map 10",			false }
};

t_ValueName sMddObjAttrNames[] =
{
	{ 0x0B, "Number of Slots",			false },
	{ 0x0C, "Number of ADIs Per Slot",	false }
};

t_ValueName sSyncInstAttrNames[] =
{
	{ 0x01, "Cycle Time",				false },
	{ 0x02, "Output Valid",				false },
	{ 0x03, "Input Capture",			false },
	{ 0x04, "Output Processing Time",	false },
	{ 0x05, "Input Processing Time",	false },
	{ 0x06, "Minimum Cycle Time",		false },
	{ 0x07, "Sync Mode",				false },
	{ 0x08, "Supported Sync Modes",		false }
};


t_ValueName sFsmInstAttrNames[] =
{
	{ 0x01, "State",					false },
	{ 0x02, "Vendor ID",				false },
	{ 0x03, "I/O Channel ID",			false },
	{ 0x04, "Firmware Version",			false },
	{ 0x05, "Serial Number",			false },
	{ 0x06, "Output Data",				false },
	{ 0x07, "Input Data",				false },
	{ 0x08, "Error Counters",			false },
	{ 0x09, "Event Log",				false },
	{ 0x0A, "Exception Information",	false },
	{ 0x0B, "Bootloader Version",		false }
};

t_ValueName sFsInstAttrNames[] =
{
	{ 0x01, "Safety Enabled",		false },
	{ 0x02, "Baud Rate",			false },
	{ 0x03, "I/O Configuration",	false }
};


t_ValueName sObjAttrNames[] =
{
	{ 0x01, "Name",						false },
	{ 0x02, "Revision",					false },
	{ 0x03, "Number of Instances",		false },
	{ 0x04, "Highest Instance Number",	false }
};

t_ValueName sObjectNames[] =
{
	{ 0x01, "Anybus Object",								false },
	{ 0x02, "Diagnositic Object",							false },
	{ 0x03, "Network Object",								false },
	{ 0x04, "Network Configuration Object",					false },
	{ 0x0A, "Anybus File System Interface Object",			false },
	{ 0x11, "Functional Safety Module Object",				false },
	{ 0xE7, "Energy Reporting Object",						false },
	{ 0xE8, "Functional Safety Object",						false },
	{ 0xE9, "POWERLINK Object",								false },
	{ 0xEA, "Application File System Interface Object",		false },
	{ 0xEB, "Assembly Object",								false },
	{ 0xEC, "Modular Device Object",						false },
	{ 0xED, "CIP Identity Host Object",						false },
	{ 0xEE, "Sync Object",									false },
	{ 0xF0, "Energy Control Object",						false },
	{ 0xF5, "EtherCAT Object",								false },
	{ 0xF6, "PROFINET IO Object",							false },
	{ 0xF7, "CC-Link Host Object",							false },
	{ 0xF8, "EtherNet/IP Host Object",						false },
	{ 0xF9, "Ethernet Host Object",							false },
	{ 0xFA, "Modbus Host Object",							false },
	{ 0xFC, "DeviceNet Host Object",						false },
	{ 0xFD, "PROFIBUS DP-V1 Object",						false },
	{ 0xFE, "Application Data Object",						false },
	{ 0xFF, "Application Object",							false }
};

t_ValueName sCmdNames[] = 
{
	{ 0x00, "Reserved",					true },
	{ 0x01, "Get_Attribute",			false },
	{ 0x02, "Set_Attribute",			false },
	{ 0x03, "Create",					false },
	{ 0x04, "Delete",					false },
	{ 0x05, "Reset",					false },
	{ 0x06, "Get_Enum_String",			false },
	{ 0x07, "Get_Indexed_Attribute",	false },
	{ 0x08, "Set_Indexed_Attribute",	false }
};


t_ValueName sMddCmdNames[] =
{
	{ 0x15, "Get_List",	false },
};

t_ValueName sAsmCmdNames[] =
{
	{ 0x10, "Write_Assembly_Data",	false },
	{ 0x11, "Read_Assembly_Data",	false }
};

t_ValueName sFsiCmdNames[] =
{
	{ 0x10, "File_Open",		false },
	{ 0x11, "File_Close",		false },
	{ 0x12, "File_Delete",		false },
	{ 0x13, "File_Copy",		false },
	{ 0x14, "File_Rename",		false },
	{ 0x15, "File_Read",		false },
	{ 0x16, "File_Write",		false },
	{ 0x20, "Directory_Open",	false },
	{ 0x21, "Directory_Close",	false },
	{ 0x22, "Directory_Delete",	false },
	{ 0x23, "Directory_Read",	false },
	{ 0x24, "Directory_Create",	false },
	{ 0x25, "Directory_Change",	false },
	{ 0x30, "Format_Disc",		false }
};

t_ValueName sEipCmdNames[] =
{
	{ 0x10, "Process_CIP_Obj_Request",		false },
	{ 0x11, "Set_Config_Data",				false },
	{ 0x12, "Process_CIP_Routing_Request",	false },
	{ 0x13, "Get_Config_Data",				false },
	{ 0x14, "Process_CIP_Obj_Request_Ext",	false }
};

t_ValueName sAppCmdNames[] =
{
	{ 0x10, "Reset_Request",			false },
	{ 0x11, "Change_Language_Request",	false },
	{ 0x12, "Reset_Diagnostic",			false }
};

t_ValueName sAppDataCmdNames[] =
{
	{ 0x10, "Get_Instance_Number_By_Order",	false },
	{ 0x11, "Get_Profile_Inst_Numbers",		false },
	{ 0x12, "Get_ADI_Info",					false },
	{ 0x13, "Remap_ADI_Write_Area",			false },
	{ 0x14, "Remap_Adi_Read_Area",			false },
	{ 0x15, "Get_Instance_Numbers",			false }
};

t_ValueName sNetCmdNames[] =
{
	{ 0x10, "Map_ADI_Write_Area",		false },
	{ 0x11, "Map_ADI_Read_Area",		false },
	{ 0x12, "Map_ADI_Write_Ext_Area",	false },
	{ 0x13, "Map_ADI_Read_Ext_Area",	false }
};

t_ValueName sIntMaskNames[] = 
{
	{ 0x01, "RDPD",		false },
	{ 0x02, "RDMSG",	false },
	{ 0x04, "WRMSG",	false },
	{ 0x08, "ANBR",		false },
	{ 0x10, "STATUS",	false },
	{ 0x20, "RESERVED",	true },
	{ 0x40, "SYNC",		false },
	{ 0x80, "RESERVED",	true },
};

t_ValueName sLedStsNames[] = 
{
	{ 0x01, "LED1A",	false },
	{ 0x02, "LED1B",	false },
	{ 0x04, "LED2A",	false },
	{ 0x08, "LED2B",	false },
	{ 0x10, "LED3A",	false },
	{ 0x20, "LED3B",	false },
	{ 0x40, "LED4A",	false },
	{ 0x80, "LED4B",	false },
};

t_ValueName sErrorRspNames[] =
{
	{ 0x02, "Invalid message format",		true },
	{ 0x03, "Unsupported object",			true },
	{ 0x04, "Unsupported instance",			true },
	{ 0x05, "Unsupported command",			true },
	{ 0x06, "Invalid CmdExt0",				true },
	{ 0x07, "Invalid CmdExt1",				true },
	{ 0x08, "Attribute not settable",		true },
	{ 0x09, "Attribute not gettable",		true },
	{ 0x0A, "Too much data",				true },
	{ 0x0B, "Not enough data",				true },
	{ 0x0C, "Out of range",					true },
	{ 0x0D, "Invalid state",				true },
	{ 0x0E, "Out of resources",				true },
	{ 0x0F, "Segmentation failure",			true },
	{ 0x10, "Segmentation buffer overflow",	true },
	{ 0x11, "Value too high",				true },
	{ 0x12, "Value too low",				true },
	{ 0xFF, "Object specific error",		true }
};

t_ValueName sAnybusStsNames[] =
{
	{ 0x00, "SETUP",			false },
	{ 0x01, "NW_INIT",			false },
	{ 0x02, "WAIT_PROCESS",		false },
	{ 0x03, "IDLE",				false },
	{ 0x04, "PROCESS_ACTIVE",	false },
	{ 0x05, "ERROR",			true },
	{ 0x07, "EXCEPTION",		true }
};

t_ValueName sApplStsNames[] =
{
	{ 0x00, "No Error",									false },
	{ 0x01, "Not yet synchronized",						false },
	{ 0x02, "Sync configuration error",					true },
	{ 0x03, "Read process data configuration error",	true },
	{ 0x04, "Write process data configuration error",	true },
	{ 0x05, "Synchronization loss",						true },
	{ 0x06, "Excessive data loss",						true },
	{ 0x07, "Output error",								true }
};

t_ValueName sSpiStsNames[] = 
{
	{ 0xC0, "RESERVED",		true },
	{ 0x20, "NEW_PD",		false },
	{ 0x10, "LAST_FRAG",	false },
	{ 0x08, "M",			false },
	{ 0x06, "CMDCNT%d",		false },
	{ 0x01, "WRMSG_FULL",	true }
};

t_ValueName sSpiCtrlNames[] = 
{
	{ 0x80, "TOGGLE",		false },
	{ 0x60, "RESERVED",		true },
	{ 0x10, "LAST_FRAG",	false },
	{ 0x08, "M",			false },
	{ 0x06, "CMDCNT%d",		false },
	{ 0x01, "WRPD_VALID",	false }
};

SpiAnalyzerResults::SpiAnalyzerResults( SpiAnalyzer* analyzer, SpiAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

SpiAnalyzerResults::~SpiAnalyzerResults()
{
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* bit_states, bool alert)
{
	char str[256];
	U16 len2, len3;
	char pad[32] = "";
	bool fPad = false;
	if(bit_states && value)
	{
		len2 = (U16)strlen(value);
		len3 = (U16)strlen(bit_states);
		if(len3<=len2)
		{
			/* We must pad the level3 (bit states) text to maintain display priority */
			fPad = true;
			memset(pad,' ',((len2-len3)>>1)+1);
		}
	}

	if(tag)
	{
		if(alert)
		{
			str[0] = '!';
		}
		else
		{
			str[0] = tag[0];
		}
		
		str[1] = NULL;
		AddResultString( str );
		AddResultString( tag );
		if(value)
		{
			if(bit_states)
			{
				SNPRINTF(str,sizeof(str),"%s: %s",tag, value);
			}
			else
			{
				SNPRINTF(str,sizeof(str),"%s: [%s]",tag, value);
			}
			AddResultString( str );
		}
	}
	else
	{
		if(value)
		{
			SNPRINTF(str,sizeof(str),"%s", value);
			AddResultString( str );
		}
	}

	if(bit_states)
	{
		if(tag)
		{
			if(fPad)
			{
				SNPRINTF(str,sizeof(str),"%s%s: (%s)%s", pad, tag, bit_states, pad);
			}
			else
			{
				SNPRINTF(str,sizeof(str),"%s: (%s)",tag, bit_states);
			}
		}
		else
		{
			if(fPad)
			{
				SNPRINTF(str,sizeof(str),"%s(%s)%s", pad, bit_states, pad);
			}
			else
			{
				SNPRINTF(str,sizeof(str),"(%s)", bit_states);
			}
		}
		AddResultString( str );
	}
}

bool GetSpiCtrlString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;
	
	display_base = display_base; /* Resolve compiler warning */

	for(U8 i=0; i<(sizeof(sSpiCtrlNames)/sizeof(t_ValueName)); i++)
	{
		if(((sSpiCtrlNames[i].value & val) != 0) || (sSpiCtrlNames[i].value == 0x06))
		{
			if(!firstFlag)
			{
				SNPRINTF(str, maxLen, " | ");
				str += 3;
				maxLen -= 3;
			}
			firstFlag = false;
			alert = (alert || sSpiCtrlNames[i].alert);
			if(sSpiCtrlNames[i].value == 0x06)
			{
				/* Special handling for command count */
				SNPRINTF(str, maxLen, sSpiCtrlNames[i].name, (val&0x06)>>1);
				str += 7;
				maxLen -= 7;
			}
			else
			{
				SNPRINTF(str, maxLen, sSpiCtrlNames[i].name);
				str += (U16)strlen(sSpiCtrlNames[i].name);
				maxLen -= (U16)strlen(sSpiCtrlNames[i].name);
			}
		}
	}

	return alert;
}

bool GetSpiStsString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;
	
	display_base = display_base; /* Resolve compiler warning */

	for(U8 i=0; i<(sizeof(sSpiStsNames)/sizeof(t_ValueName)); i++)
	{
		if(((sSpiStsNames[i].value & val) != 0) || (sSpiStsNames[i].value == 0x06))
		{
			if(!firstFlag)
			{
				SNPRINTF(str, maxLen, " | ");
				str += 3;
				maxLen -= 3;
			}
			firstFlag = false;
			alert = (alert || sSpiStsNames[i].alert);
			if(sSpiStsNames[i].value == 0x06)
			{
				/* Special handling for command count */
				SNPRINTF(str, maxLen, sSpiStsNames[i].name, (val&0x06)>>1);
				str += 7;
				maxLen -= 7;
			}
			else
			{
				SNPRINTF(str, maxLen, sSpiStsNames[i].name);
				str += (U16)strlen(sSpiStsNames[i].name);
				maxLen -= (U16)strlen(sSpiStsNames[i].name);
			}
		}
	}

	return alert;
}

bool GetApplStsString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	bool alert = false;
	bool found = false;
	
	for(U8 i=0; i<(sizeof(sApplStsNames)/sizeof(t_ValueName)); i++)
	{
		if(sApplStsNames[i].value == val)
		{
			SNPRINTF(str, maxLen, sApplStsNames[i].name);
			alert = sApplStsNames[i].alert;
			found = true;
			break;
		}
	}

	if(!found)
	{
		AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
		SNPRINTF(str, maxLen, "Reserved: %s", number_str);
		alert = true;
	}

	return alert;
}

bool GetAbccStatusString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	bool alert = false;
	bool found = false;
	char tmpstr[256];
			
	for(U8 i=0; i<(sizeof(sAnybusStsNames)/sizeof(t_ValueName)); i++)
	{
		if(sAnybusStsNames[i].value == (val&0xF7))
		{
			SNPRINTF(tmpstr, sizeof(tmpstr), sAnybusStsNames[i].name);
			alert = sAnybusStsNames[i].alert;
			found = true;
			break;
		}
	}

	if(!found)
	{
		AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
		SNPRINTF(tmpstr, sizeof(tmpstr), "Reserved: %s", number_str);
		alert = true;
	}

	if(((val>>3)&0x1) == 1)
	{
		SNPRINTF(str,maxLen,"%s | SUP",tmpstr);
	}
	else
	{
		SNPRINTF(str,maxLen,"%s",tmpstr);
	}
		
	return alert;
}

bool GetErrorRspString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	for(U8 i=0; i<(sizeof(sErrorRspNames)/sizeof(t_ValueName)); i++)
	{
		if(sErrorRspNames[i].value == val)
		{
			SNPRINTF(str, maxLen, sErrorRspNames[i].name);
			return sErrorRspNames[i].alert;
		}
	}
	
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str, maxLen, "Reserved: %s", number_str);
	return true;
}

bool GetIntMaskString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;

	display_base = display_base; /* Resolve compiler warning */

	for(U8 i=0; i<(sizeof(sIntMaskNames)/sizeof(t_ValueName)); i++)
	{
		if((sIntMaskNames[i].value & val) != 0)
		{
			if(!firstFlag)
			{
				SNPRINTF(str, maxLen, " | ");
				str += 3;
				maxLen -= 3;
			}
			firstFlag = false;
			alert = (alert || sIntMaskNames[i].alert);
			SNPRINTF(str, maxLen, sIntMaskNames[i].name);
			str += (U16)strlen(sIntMaskNames[i].name);
			maxLen -= (U16)strlen(sIntMaskNames[i].name);
		}
	}
		
	if(firstFlag)
	{
		SNPRINTF(str, maxLen, "None");
	}
	return alert;
}

bool GetLedStatusString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	bool firstFlag = true;
	bool alert = false;

	display_base = display_base; /* Resolve compiler warning */

	for(U8 i=0; i<(sizeof(sLedStsNames)/sizeof(t_ValueName)); i++)
	{
		if((sLedStsNames[i].value & val) != 0)
		{
			if(!firstFlag)
			{
				SNPRINTF(str, maxLen, " | ");
				str += 3;
				maxLen -= 3;
			}
			firstFlag = false;
			alert = (alert || sLedStsNames[i].alert);
			SNPRINTF(str, maxLen, sLedStsNames[i].name);
			str += (U16)strlen(sLedStsNames[i].name);
			maxLen -= (U16)strlen(sLedStsNames[i].name);
		}
	}
		
	if(firstFlag)
	{
		SNPRINTF(str, maxLen, "None");
	}
	return alert;
}



bool GetAnbAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sAnbInstAttrNames[0];
		bNoAttrs = (sizeof(sAnbInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetNwAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sNwInstAttrNames[0];
		bNoAttrs = (sizeof(sNwInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetNcAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sNcInstAttrNames[0];
		bNoAttrs = (sizeof(sNcInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetAppAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sAppInstAttrNames[0];
		bNoAttrs = (sizeof(sAppInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetSyncAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sSyncInstAttrNames[0];
		bNoAttrs = (sizeof(sSyncInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetEtnAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sEtnInstAttrNames[0];
		bNoAttrs = (sizeof(sEtnInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetIdAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sIdInstAttrNames[0];
		bNoAttrs = (sizeof(sIdInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetEipAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sEipInstAttrNames[0];
		bNoAttrs = (sizeof(sEipInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetEtcAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sEtcInstAttrNames[0];
		bNoAttrs = (sizeof(sEtcInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetFsmAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sFsmInstAttrNames[0];
		bNoAttrs = (sizeof(sFsmInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetFsAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		psAttrNames = &sObjAttrNames[0];
		bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
	}
	else
	{
		psAttrNames = &sFsInstAttrNames[0];
		bNoAttrs = (sizeof(sFsInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetMddAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sMddObjAttrNames[0];
			bNoAttrs = (sizeof(sMddObjAttrNames)/sizeof(t_ValueName));
		}
	
		for(U8 i=0; i<bNoAttrs; i++)
		{
			if(psAttrNames[i].value == val)
			{
				SNPRINTF(str,maxLen,psAttrNames[i].name);
				return psAttrNames[i].alert;
			}
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetDiAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sDiObjAttrNames[0];
			bNoAttrs = (sizeof(sDiObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sDiInstAttrNames[0];
		bNoAttrs = (sizeof(sDiInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetFsiAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sFsiObjAttrNames[0];
			bNoAttrs = (sizeof(sFsiObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sFsiInstAttrNames[0];
		bNoAttrs = (sizeof(sFsiInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetAppdAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sAppdObjAttrNames[0];
			bNoAttrs = (sizeof(sAppdObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sAppdInstAttrNames[0];
		bNoAttrs = (sizeof(sAppdInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetAsmAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sAsmObjAttrNames[0];
			bNoAttrs = (sizeof(sAsmObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sAsmInstAttrNames[0];
		bNoAttrs = (sizeof(sAsmInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetSocAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sSocObjAttrNames[0];
			bNoAttrs = (sizeof(sSocObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sSocInstAttrNames[0];
		bNoAttrs = (sizeof(sSocInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetSmtpAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sSmtpObjAttrNames[0];
			bNoAttrs = (sizeof(sSmtpObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sSmtpInstAttrNames[0];
		bNoAttrs = (sizeof(sSmtpInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetCipAttrString(U16 inst, U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	t_ValueName* psAttrNames;
	U8 bNoAttrs;
	if(inst==0)
	{
		if(val<=4)
		{
			psAttrNames = &sObjAttrNames[0];
			bNoAttrs = (sizeof(sObjAttrNames)/sizeof(t_ValueName));
		}
		else
		{
			psAttrNames = &sCipObjAttrNames[0];
			bNoAttrs = (sizeof(sCipObjAttrNames)/sizeof(t_ValueName));
		}
	}
	else
	{
		psAttrNames = &sCipInstAttrNames[0];
		bNoAttrs = (sizeof(sCipInstAttrNames)/sizeof(t_ValueName));
	}

	for(U8 i=0; i<bNoAttrs; i++)
	{
		if(psAttrNames[i].value == val)
		{
			SNPRINTF(str,maxLen,psAttrNames[i].name);
			return psAttrNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetObjectString(U8 val, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];

	for(U8 i=0; i<(sizeof(sObjectNames)/sizeof(t_ValueName)); i++)
	{
		if(sObjectNames[i].value == val)
		{
			SNPRINTF(str,maxLen,sObjectNames[i].name);
			return sObjectNames[i].alert;
		}
	}

	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str,maxLen,"Unknown: %s", number_str);
	return true;
}

bool GetObjSpecificCmdString(U8 val, char* str, U16 maxLen,t_ValueName* pasCmdNames, U8 bNoCmds, DisplayBase display_base)
{
	char number_str[128];
	for(U8 i=0; i<bNoCmds; i++)
	{
		if(pasCmdNames[i].value == val)
		{
			SNPRINTF(str,maxLen,pasCmdNames[i].name);
			return pasCmdNames[i].alert;
		}
	}
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	SNPRINTF(str, maxLen, "Unknown: %s", number_str);
	return true;
}

bool GetCmdString(U8 val, U8 obj, char* str, U16 maxLen, DisplayBase display_base)
{
	char number_str[128];
	bool alert = false;
	U8 cmd = (val & 0x3F);

	for(U8 i=0; i<(sizeof(sCmdNames)/sizeof(t_ValueName)); i++)
	{
		if(sCmdNames[i].value == cmd)
		{
			SNPRINTF(str,maxLen,sCmdNames[i].name);
			return sCmdNames[i].alert;
		}
	}

	if (((cmd >= 0x09) && (cmd <= 0x0F)) ||
		((cmd >= 0x31) && (cmd <= 0x3E)))
	{
		AnalyzerHelpers::GetNumberString( cmd, display_base, 8, number_str, 128 );
		SNPRINTF(str, maxLen, "Reserved: %s", number_str);
		alert = true;
	}
	else if (((cmd >= 0x10) && (cmd <= 0x30)) ||
			 (cmd == 0x3F))
	{
		
		if(obj == 0x03)
		{
			char objCmdStr[256];
			/* Network Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sNetCmdNames[0], (sizeof(sNetCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else if((obj == 0x0A) || (obj == 0xEA))
		{
			char objCmdStr[256];
			/* (Application/Anybus) File System Interface Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sFsiCmdNames[0], (sizeof(sFsiCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else if(obj == 0xEB)
		{
			char objCmdStr[256];
			/* Assembly Mapping Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sAsmCmdNames[0], (sizeof(sAsmCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else if(obj == 0xEC)
		{
			char objCmdStr[256];
			/* Modular Device Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sMddCmdNames[0], (sizeof(sMddCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else if(obj == 0xF8)
		{
			char objCmdStr[256];
			/* EtherNet/IP Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sEipCmdNames[0], (sizeof(sEipCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else if(obj == 0xFE)
		{
			char objCmdStr[256];
			/* Application Data Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sAppDataCmdNames[0], (sizeof(sAppDataCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else if(obj == 0xFF)
		{
			char objCmdStr[256];
			/* Application Object */
			alert = GetObjSpecificCmdString(cmd, objCmdStr, sizeof(objCmdStr), &sAppCmdNames[0], (sizeof(sAppCmdNames)/sizeof(t_ValueName)), display_base);
			SNPRINTF(str, maxLen, "Object Specific: %s", objCmdStr);
		}
		else
		{
			AnalyzerHelpers::GetNumberString( cmd, display_base, 8, number_str, 128 );
			SNPRINTF(str, maxLen, "Object Specific: %s", number_str);
			alert = true; //TODO: We only alert here because we have not implmented all object specific commands yet.
		}
	}

	return alert;
}

bool SpiAnalyzerResults::BuildCmdString(U8 val, U8 obj, DisplayBase display_base)
{
	bool errorRspMsg;
	char str[256];
	char number_str[128];
	bool alert = GetCmdString(val, obj, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	if ((val & 0x80) == 0x80)
	{
		errorRspMsg = true;
		StringBuilder("ERR", number_str, str, true);
	}
	else
	{
		errorRspMsg = false;
		if ((val & 0x40) == 0x40)
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

void SpiAnalyzerResults::BuildAttrString(U8 obj, U16 inst, U16 val, bool indexed, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = false;
	U8 ofst = 0;
	char tag[] = "EXT";
	if(indexed)
	{
		SNPRINTF(str,sizeof(str),"Index %d, ",(U8)(val>>8));
		ofst = (U8)strlen(str);
	}
	if(obj == 0x01)
	{
		/* Anybus Object */
		alert = GetAnbAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x02)
	{
		/* Diagnostic Object */
		alert = GetDiAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x03)
	{
		/* Network Object */
		alert = GetNwAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x04)
	{
		/* Network Configuration Object */
		alert = GetNcAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x07)
	{
		/* Socket Interface Object */
		alert = GetSocAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x09)
	{
		/* SMTP Client Object */
		alert = GetSmtpAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if((obj == 0x0A)||(obj == 0xEA))
	{
		/* File system objects */
		alert = GetFsiAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x0B)
	{
		/* Functional Safety Module Object */
		alert = GetFsmAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if((obj == 0x0C)||(obj == 0xF9))
	{
		/* Network Ethernet Object / Ethernet Host Object */
		alert = GetEtnAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0x0D)
	{
		/* CIP Port Configuration Object */
		alert = GetCipAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xE8)
	{
		/* Functional Safety Object */
		alert = GetFsAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xEB)
	{
		/* Assembly Mapping Object */
		alert = GetAsmAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xEC)
	{
		/* Modular Device Object */
		alert = GetMddAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xED)
	{
		/* CIP Identity Host Object */
		alert = GetIdAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xEE)
	{
		/* Sync Object */
		alert = GetSyncAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xF5)
	{
		/* EtherCAT Host Object */
		alert = GetEtcAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xF8)
	{
		/* EtherNet/IP Host Object */
		alert = GetEipAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xFE)
	{
		/* Application Data Object */
		alert = GetAppdAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else if(obj == 0xFF)
	{
		/* Application Object */
		alert = GetAppAttrString( inst, (U8)val, &str[ofst], sizeof(str), display_base );
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, str, alert);
	}
	else
	{
		AnalyzerHelpers::GetNumberString( val, display_base, 16, number_str, 128 );
		StringBuilder(tag, number_str, NULL, false);
	}
}

void SpiAnalyzerResults::BuildObjectString(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = GetObjectString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("OBJ", number_str, str, alert);
}

void SpiAnalyzerResults::BuildSpiCtrlString(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = GetSpiCtrlString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("SPI_CTL", number_str, str, alert);
}

void SpiAnalyzerResults::BuildSpiStsString(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = GetSpiStsString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("SPI_STS", number_str, str, alert);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = GetErrorRspString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("ERR_RSP", number_str, str, alert);
}

void SpiAnalyzerResults::BuildIntMask(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = GetIntMaskString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("INT_MSK", number_str, str, alert);
}

void SpiAnalyzerResults::BuildAbccStatus(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	bool alert = GetAbccStatusString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("ANB_STS", number_str, str, alert);
}

void SpiAnalyzerResults::BuildApplStatus(U8 val, DisplayBase display_base)
{
	char str[256];
	char number_str[128];
	/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
	bool alert = GetApplStsString((U8)val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString( val, display_base, 8, number_str, 128 );
	StringBuilder("APP_STS", number_str, str, alert);
}

void SpiAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	char number_str[128];
	Frame frame = GetFrame( frame_index );
	
	if( ( frame.mFlags & (SPI_FRAG_ERROR_FLAG|SPI_ERROR_FLAG) ) == 0 )
	{
		if( (channel == mSettings->mMosiChannel ) && IS_MOSI_FRAME )
		{
			switch(frame.mType)
			{
			case e_ABCC_MOSI_IDLE:
				break;
			case e_ABCC_MOSI_SPI_CTRL:
				BuildSpiCtrlString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_RESERVED1:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MOSI_MSG_LEN:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );	
					SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
					StringBuilder("MSG_LEN", number_str, str, false);
				}
				break;
			case e_ABCC_MOSI_PD_LEN:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );	
					SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
					StringBuilder("PD_LEN", number_str, str, false);
				}
				break;
			case e_ABCC_MOSI_APP_STAT:
				BuildApplStatus((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_INT_MASK:
				BuildIntMask((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_WR_MSG_FIELD:
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG))
				{
					BuildErrorRsp((U8)frame.mData1, display_base);
				}
				else
				{
					char str[256];
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
					StringBuilder("MD", number_str, str, false);
				}
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );	
					SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
					StringBuilder("MD_SIZE", number_str, str, false);
				}
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_res1:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
				StringBuilder("SRC_ID", number_str, NULL, false);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
				BuildObjectString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, number_str, 128);
				StringBuilder("INST", number_str, NULL, false);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
				BuildCmdString((U8)frame.mData1,(U8)frame.mData2, display_base);
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_res2:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
				if(((frame.mData2&0x3F) == 1) || ((frame.mData2&0x3F) == 2))
				{
					BuildAttrString((U8)(frame.mData2>>8), (U16)(frame.mData2>>16), (U16)frame.mData1, false, display_base);
				}
				else if(((frame.mData2&0x3F) == 7) || ((frame.mData2&0x3F) == 8))
				{
					BuildAttrString((U8)(frame.mData2>>8), (U16)(frame.mData2>>16), (U16)frame.mData1, true, display_base);
				}
				else
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, number_str, 128);
					StringBuilder("EXT", number_str, NULL, false);
				}
				break;			
			case e_ABCC_MOSI_WR_PD_FIELD:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
					StringBuilder("PD", number_str, str, false);
				}
				break;
			case e_ABCC_MOSI_CRC32:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, number_str, 128);
					if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG))
					{	
						SNPRINTF(str, sizeof(str), "Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
						StringBuilder("CRC32_ERROR", number_str, str, true);
					}
					else
					{
						
						SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
						StringBuilder("CRC32", number_str, str, false);
					}
				}
				break;
			case e_ABCC_MOSI_PAD:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, number_str, 128);
				StringBuilder("PAD", number_str, NULL, (frame.mData1 != 0));
				break;
			default:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				StringBuilder("UNKWN", number_str, "Internal Error: Unknown State", true);
				break;
			}
		}
		else if ((channel == mSettings->mMisoChannel) && IS_MISO_FRAME)
		{
			switch (frame.mType)
			{
			case e_ABCC_MISO_IDLE:
				break;
			case e_ABCC_MISO_Reserved1:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MISO_Reserved2:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MISO_LED_STAT:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );	
					GetLedStatusString((U8)frame.mData1, str, sizeof(str), display_base);
					StringBuilder("LED_STS", number_str, str, false);
				}
				break;
			case e_ABCC_MISO_ANB_STAT:
				BuildAbccStatus((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MISO_SPI_STAT:
				BuildSpiStsString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MISO_NET_TIME:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, number_str, 128);
				StringBuilder("TIME", number_str, NULL, false);
				break;
			case e_ABCC_MISO_RD_MSG_FIELD:
			case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG))
				{
					BuildErrorRsp((U8)frame.mData1, display_base);
				}
				else
				{
					char str[256];
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
					StringBuilder("MD", number_str, str, false);
				}
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );	
					SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
					StringBuilder("MD_SIZE", number_str, str, false);
				}
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_res1:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
				StringBuilder("SRC_ID", number_str, NULL, false);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
				BuildObjectString((U8)frame.mData1, display_base);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, number_str, 128);
				StringBuilder("INST", number_str, NULL, false);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
				BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_res2:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				StringBuilder("RES", number_str, "Reserved", (frame.mData1 != 0));
				break;
			case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
				if(((frame.mData2&0x3F) == 1) || ((frame.mData2&0x3F) == 2))
				{
					BuildAttrString((U8)(frame.mData2>>8), (U16)(frame.mData2>>16), (U16)frame.mData1, false, display_base);
				}
				else if(((frame.mData2&0x3F) == 7) || ((frame.mData2&0x3F) == 8))
				{
					BuildAttrString((U8)(frame.mData2>>8), (U16)(frame.mData2>>16), (U16)frame.mData1, true, display_base);
				}
				else
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 16, number_str, 128);
					StringBuilder("EXT", number_str, NULL, false);
				}
				break;			
			case e_ABCC_MISO_RD_PD_FIELD:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", number_str, frame.mData2);
					StringBuilder("PD", number_str, str, false);
				}
				break;
			case e_ABCC_MISO_CRC32:
				{
					char str[256];
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, number_str, 128);
					if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG))
					{	
						SNPRINTF(str, sizeof(str), "Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
						StringBuilder("CRC32_ERROR", number_str, str, true);
					}
					else
					{
						
						SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
						StringBuilder("CRC32", number_str, str, false);
					}
				}
				break;
			default:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
				StringBuilder("UNKWN", number_str, "Internal Error: Unknown State", true);
				break;
			}
		}
	}
	else
	{
		if ((frame.mFlags & SPI_FRAG_ERROR_FLAG) == SPI_FRAG_ERROR_FLAG)
		{
			StringBuilder("FRAG", NULL, "Fragmented ABCC Message",true);
		}
		else if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
		{
			StringBuilder("ERROR", NULL, "Settings mismatch, The initial (idle) state of the CLK line does not match the settings.",true);
		}
	}
}

void SpiAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
	//export_type_user_id is only important if we have more than one export type.		

	std::stringstream ss;
	void* f = AnalyzerHelpers::StartFile( file );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	ss << "Time [s],Packet ID,MOSI,MISO" << std::endl;

	bool mosi_used = true;
	bool miso_used = true;

	if( mSettings->mMosiChannel == UNDEFINED_CHANNEL )
		mosi_used = false;

	if( mSettings->mMisoChannel == UNDEFINED_CHANNEL )
		miso_used = false;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		if( ( frame.mFlags & SPI_ERROR_FLAG ) == SPI_ERROR_FLAG )
			continue;
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char mosi_str[128] = "";
		if( mosi_used == true )
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, mSettings->mBitsPerTransfer, mosi_str, 128 );

		char miso_str[128] = "";
		if( miso_used == true )
			AnalyzerHelpers::GetNumberString( frame.mData2, display_base, mSettings->mBitsPerTransfer, miso_str, 128 );

		U64 packet_id = GetPacketContainingFrameSequential( i ); 
		if( packet_id != INVALID_RESULT_INDEX )
			ss << time_str << "," << packet_id << "," << mosi_str << "," << miso_str << std::endl;
		else
			ss << time_str << ",," << mosi_str << "," << miso_str << std::endl;  //it's ok for a frame not to be included in a packet.

		AnalyzerHelpers::AppendToFile( (U8*)ss.str().c_str(), (U32)ss.str().length(), f );
		ss.str( std::string() );

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			AnalyzerHelpers::EndFile( f );
			return;
		}
	}

	UpdateExportProgressAndCheckForCancel( num_frames, num_frames );
	AnalyzerHelpers::EndFile( f );
}

void SpiAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    ClearTabularText();
	Frame frame = GetFrame( frame_index );
	bool mosi_used = true;
	bool miso_used = true;

	if( mSettings->mMosiChannel == UNDEFINED_CHANNEL )
		mosi_used = false;

	if( mSettings->mMisoChannel == UNDEFINED_CHANNEL )
		miso_used = false;

	if (miso_used || mosi_used)
	{
		if (mSettings->mErrorIndexing == true)
		{
			if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG|DISPLAY_AS_ERROR_FLAG))
			{
				if((frame.mType == e_ABCC_MISO_CRC32) && IS_MISO_FRAME)
				{
					AddTabularText("!CRC32 MISO");
				}
				else if((frame.mType == e_ABCC_MOSI_CRC32) && IS_MOSI_FRAME)
				{
					AddTabularText("!CRC32 MOSI");
				}
				return;
			}
			else if ((frame.mFlags & (SPI_FRAG_ERROR_FLAG|DISPLAY_AS_ERROR_FLAG)) == (SPI_FRAG_ERROR_FLAG|DISPLAY_AS_ERROR_FLAG))
			{
				AddTabularText("!Fragmented ABCC Message");
				return;
			}

			if( (frame.mType == e_ABCC_MOSI_APP_STAT) && IS_MOSI_FRAME )
			{	
				if(frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					char str[256];
					/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
					if(GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
					{
						AddTabularText("!Application Status : (",str,")");
						return;
					}
					else
					{
						if (mSettings->mApplStatusIndexing == true)
						{
							AddTabularText("Application Status : (",str,")");
							return;
						}
					}
				}
			}

			if( (frame.mType == e_ABCC_MISO_ANB_STAT) && IS_MISO_FRAME )
			{	
				if(frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					char str[256];
					if(GetAbccStatusString((U8)frame.mData1,&str[0], sizeof(str), display_base))
					{
						AddTabularText("!Anybus Status : (",str,")");
						return;
					}
					else
					{
						if(mSettings->mAnybusStatusIndexing == true)
						{
							AddTabularText("Anybus Status : (",str,")");
							return;
						}
					}
				}
			}
		}

		if(mSettings->mTimestampIndexing == true)
		{
			if((frame.mType == e_ABCC_MISO_NET_TIME) && IS_MISO_FRAME)
			{
				char tmp[35];
				SNPRINTF(tmp,sizeof(tmp),"0x%08X (Delta : 0x%08X)", (U32)frame.mData1, (U32)frame.mData2);
				AddTabularText("Time : ",tmp);
				//SNPRINTF(tmp,sizeof(tmp),"0x%08X", (U32)frame.mData1);
				//AddTabularText("Network Time : ",tmp);
				return;
			}
		}

		if(mSettings->mApplStatusIndexing == true)
		{
			if( (frame.mType == e_ABCC_MOSI_APP_STAT) && IS_MOSI_FRAME )
			{	
				if(frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					char str[256];
					/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
					if(GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
					{
						AddTabularText("!Application Status : (",str,")");
					}
					else
					{
						AddTabularText("Application Status : (",str,")");
					}
					return;
				}
			}
		}

		if(mSettings->mAnybusStatusIndexing == true)
		{
			if( (frame.mType == e_ABCC_MISO_ANB_STAT) && IS_MISO_FRAME )
			{	
				if(frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					char str[256];
					if(GetAbccStatusString((U8)frame.mData1,&str[0], sizeof(str), display_base))
					{
						AddTabularText("!Anybus Status : (",str,")");
					}
					else
					{
						AddTabularText("Anybus Status : (",str,")");
					}
					return;
				}
			}
		}

		/* Since tabular text is sequential processed and indexed,
		** buffer the "Object", "Instance", "Cmd", and "Ext";
		** then add as a single text entry. */
		if (mSettings->mMessageIndexing == true)
		{
			static char src_str[2][256] = {""};
			static char obj_str[2][256] = {""};
			static char inst_str[2][256] = {""};
			static char cmd_str[2][256] = {""};
			static char ext_str[2][256] = {""};
			static bool fMsgValid[2];
			static bool fMsgErrorRsp[2];
			
			if(IS_MISO_FRAME)
			{				
				switch(frame.mType)
				{
				case e_ABCC_MISO_SPI_STAT:
					if((frame.mData1&0x08) != 0)
					{
						fMsgValid[1] =  true;
					}
					else
					{
						fMsgValid[1] =  false;
					}

					if((frame.mFlags & (SPI_PROTO_EVENT_FLAG)) == SPI_PROTO_EVENT_FLAG)
					{
						AddTabularText("{Write Message Buffer Full} MISO");
						return;
					}
					else if((frame.mFlags & (SPI_MSG_FRAG_FLAG|SPI_MSG_FIRST_FRAG_FLAG)) == SPI_MSG_FRAG_FLAG)
					{
						/* Fragmentation is in progress */
						if(frame.mData1&0x08)
						{
							if(frame.mData1&0x10)
							{
								/* Last fragment */
								AddTabularText("{Message Fragment} MISO");
							}
							else
							{
								/* More fragments follow */
								AddTabularText("{Message Fragment} MISO++");
							}
						}
						return;
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
					SNPRINTF(src_str[1],sizeof(src_str[1]),"Source ID: 0x%02X MISO", (U8)frame.mData1);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
					SNPRINTF(obj_str[1],sizeof(obj_str[1]),"Obj {%02X:", (U8)frame.mData1);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
					SNPRINTF(inst_str[1],sizeof(inst_str[1]),"%04Xh}", (U16)frame.mData1);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
					SNPRINTF(cmd_str[1],sizeof(cmd_str[1]),", Cmd {%02X:", (U8)frame.mData1);
					if((frame.mData1&0x80) != 0)
					{
						fMsgErrorRsp[1] = true;
					}
					else
					{
						fMsgErrorRsp[1] = false;
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
					if(fMsgValid[1])
					{
						SNPRINTF(ext_str[1],sizeof(ext_str[1]),"%04Xh} MISO", (U16)frame.mData1);
						if (mSettings->mMessageSrcIdIndexing == true)
						{
							AddTabularText("-----MESSAGE-----");
							AddTabularText(src_str[1]);
						}
						if(fMsgErrorRsp[1])
						{
							AddTabularText("!",obj_str[1],inst_str[1],cmd_str[1],ext_str[1]);
						}
						else
						{
							if(frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								AddTabularText(obj_str[1],inst_str[1],cmd_str[1],ext_str[1],"++");
							}
							else
							{
								AddTabularText(obj_str[1],inst_str[1],cmd_str[1],ext_str[1]);
							}
						}
						return;
					}
					break;
				default:
					break;
				}
			}
			
			if(IS_MOSI_FRAME)
			{
				switch(frame.mType)
				{
				case e_ABCC_MOSI_SPI_CTRL:
					if((frame.mData1&0x08) != 0)
					{
						fMsgValid[0] =  true;
					}
					else
					{
						fMsgValid[0] =  false;
					}
					if((frame.mFlags & (SPI_PROTO_EVENT_FLAG)) == SPI_PROTO_EVENT_FLAG)
					{
						if(frame_index != 0)
						{
							AddTabularText("{Message Retransmit} MOSI");
							return;
						}
					}
					else if((frame.mFlags & (SPI_MSG_FRAG_FLAG|SPI_MSG_FIRST_FRAG_FLAG)) == SPI_MSG_FRAG_FLAG)
					{
						/* Fragmentation is in progress */
						if(frame.mData1&0x08)
						{
							if(frame.mData1&0x10)
							{
								/* Last fragment */
								AddTabularText("{Message Fragment} MOSI");
							}
							else
							{
								/* More fragments follow */
								AddTabularText("{Message Fragment} MOSI++");
							}
						}
						return;
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
					SNPRINTF(src_str[0],sizeof(src_str[0]),"Source ID: 0x%02X MOSI", (U8)frame.mData1);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
					SNPRINTF(obj_str[0],sizeof(obj_str[0]),"Obj {%02X:", (U8)frame.mData1);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
					SNPRINTF(inst_str[0],sizeof(inst_str[0]),"%04Xh}", (U16)frame.mData1);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
					SNPRINTF(cmd_str[0],sizeof(cmd_str[0]),", Cmd {%02X:", (U8)frame.mData1);
					if((frame.mData1&0x80) != 0)
					{
						fMsgErrorRsp[0] = true;
					}
					else
					{
						fMsgErrorRsp[0] = false;
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
					if(fMsgValid[0])
					{
						SNPRINTF(ext_str[0],sizeof(ext_str[0]),"%04Xh} MOSI", (U16)frame.mData1);
						if (mSettings->mMessageSrcIdIndexing == true)
						{
							AddTabularText("-----MESSAGE-----");
							AddTabularText(src_str[0]);
						}
						if(fMsgErrorRsp[0])
						{
							AddTabularText("!",obj_str[0],inst_str[0],cmd_str[0],ext_str[0]);
						}
						else
						{
							if(frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								AddTabularText(obj_str[0],inst_str[0],cmd_str[0],ext_str[0],"++");
							}
							else
							{
								AddTabularText(obj_str[0],inst_str[0],cmd_str[0],ext_str[0]);
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

void SpiAnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/, DisplayBase /*display_base*/ )  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void SpiAnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/, DisplayBase /*display_base*/ )  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString( "not supported" );
}
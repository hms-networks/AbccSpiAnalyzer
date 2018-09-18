/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerResults.cpp
**    Summary: DLL-Results source
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "AbccSpiAnalyzerResults.h"
#include "AnalyzerHelpers.h"
#include "AbccSpiAnalyzer.h"
#include "AbccSpiAnalyzerSettings.h"
#include "AbccSpiAnalyzerLookup.h"

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#define IS_MISO_FRAME(frame)	((frame.mFlags & SPI_MOSI_FLAG) != SPI_MOSI_FLAG)
#define IS_MOSI_FRAME(frame)	((frame.mFlags & SPI_MOSI_FLAG) == SPI_MOSI_FLAG)

#define MOSI_TAG_STR			"MOSI-"
#define MISO_TAG_STR			"MISO-"

#define MOSI_STR				"MOSI"
#define MISO_STR				"MISO"

#define FIRST_FRAG_STR			"FIRST_FRAGMENT"
#define FRAGMENT_STR			"FRAGMENT"
#define LAST_FRAG_STR			"LAST_FRAGMENT"

#define ERROR_RESPONSE_STR		" (ERR_RSP)"
#define RESPONSE_STR			" (RSP)"
#define COMMAND_STR				" (CMD)"

#define CSV_DELIMITER			mSettings->mExportDelimiter

#ifdef _DEBUG
/* Dummy macros, the old SDK does not support these */
#define AddTabularText(...)
#define ClearTabularText(...)
#endif

SpiAnalyzerResults::SpiAnalyzerResults(SpiAnalyzer* analyzer, SpiAnalyzerSettings* settings)
	: AnalyzerResults(),
	  mSettings(settings),
	  mAnalyzer(analyzer)
{
	memset(mMsgSizeStr, 0, sizeof(mMsgSizeStr));
	memset(mMsgSrcStr, 0, sizeof(mMsgSrcStr));
	memset(mMsgObjStr, 0, sizeof(mMsgObjStr));
	memset(mMsgInstStr, 0, sizeof(mMsgInstStr));
	memset(mMsgCmdStr, 0, sizeof(mMsgCmdStr));
	memset(mMsgExtStr, 0, sizeof(mMsgExtStr));
	mMsgValidFlag[SpiChannel::MOSI] = false;
	mMsgValidFlag[SpiChannel::MISO] = false;
	mMsgErrorRspFlag[SpiChannel::MOSI] = false;
	mMsgErrorRspFlag[SpiChannel::MISO] = false;
}

SpiAnalyzerResults::~SpiAnalyzerResults()
{
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* verbose, NotifEvent_t notification)
{
	StringBuilder(tag, value, verbose, notification, DisplayPriority::Tag);
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* verbose, NotifEvent_t notification, DisplayPriority disp_priority)
{
	const char alertStr[] = "!ALERT - ";
	U16 strLenValue, strLenVerbose;
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char pad[32] = "";
	bool applyPad = false;

	if (verbose && value)
	{
		strLenValue = (U16)strlen(value);

		if (notification == NotifEvent::Alert)
		{
			strLenValue += sizeof(alertStr);
		}

		strLenVerbose = (U16)strlen(verbose);

		if (strLenVerbose <= strLenValue)
		{
			/* We must pad the level3 (bit states) text to maintain display priority */
			applyPad = true;
			memset(pad, ' ', ((strLenValue - strLenVerbose) >> 1) + 1);
		}
	}

	if (tag)
	{
		if (notification == NotifEvent::Alert)
		{
			str[0] = '!';
		}
		else
		{
			str[0] = tag[0];
		}

		if ((notification == NotifEvent::Alert) || disp_priority == DisplayPriority::Tag)
		{
			str[1] = '\0';
			AddResultString(str);
		}

		if (disp_priority == DisplayPriority::Value)
		{
			if (value)
			{
				if (notification == NotifEvent::Alert)
				{
					AddResultString(alertStr, value);
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

			if (notification == NotifEvent::Alert)
			{
				AddResultString(alertStr, str);
			}
			else
			{
				AddResultString(str);
			}
		}
		else
		{
			if (notification == NotifEvent::Alert)
			{
				AddResultString(alertStr, tag);
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

				if (notification == NotifEvent::Alert)
				{
					AddResultString(alertStr, str);
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

			if (notification == NotifEvent::Alert)
			{
				AddResultString(alertStr, str);
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
			if (applyPad)
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
			if (applyPad)
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

void SpiAnalyzerResults::TableBuilder(SpiChannel_t e_channel, char* text, NotifEvent_t notification)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char *prefix;
	char mosiPrefix[] = MOSI_TAG_STR;
	char misoPrefix[] = MISO_TAG_STR;

	if (e_channel == SpiChannel::MOSI)
	{
		prefix = &mosiPrefix[0];
	}
	else
	{
		prefix = &misoPrefix[0];
	}
	if (notification)
	{
		SNPRINTF(str, sizeof(str), "%s!%s", prefix, text);
	}
	else
	{
		SNPRINTF(str, sizeof(str), "%s%s", prefix, text);
	}

	AddTabularText(str);
}

bool SpiAnalyzerResults::BuildCmdString(U8 val, U8 obj, DisplayBase display_base)
{
	bool errorRspMsg;
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetCmdString(val, obj, &str[0], sizeof(str), display_base);

	GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::Command), numberStr, sizeof(numberStr), BaseType::Numeric);

	if ((val & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
	{
		errorRspMsg = true;
		StringBuilder("ERR_RSP", numberStr, str, NotifEvent::Alert);
	}
	else
	{
		errorRspMsg = false;
		if ((val & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
		{
			StringBuilder("CMD", numberStr, str, notification);
		}
		else
		{
			StringBuilder("RSP", numberStr, str, notification);
		}
	}

	return errorRspMsg;
}

void SpiAnalyzerResults::BuildInstString(U8 nw_type_idx, U8 obj, U16 val, DisplayBase display_base)
{
	char verbose_str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	bool objFound = true;

	objFound = GetInstString(nw_type_idx, obj, val, verbose_str, sizeof(verbose_str), &notification, display_base);

	GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::CommandExtension), numberStr, sizeof(numberStr), BaseType::Numeric);

	if (objFound)
	{
		StringBuilder(GET_MSG_FRAME_TAG(AbccMsgField::Instance), numberStr, verbose_str, notification);
	}
	else
	{
		StringBuilder(GET_MSG_FRAME_TAG(AbccMsgField::Instance), numberStr, nullptr, NotifEvent::None);
	}
}

void SpiAnalyzerResults::BuildAttrString(U8 obj, U16 inst, U16 val, AttributeAccessMode_t access_mode, DisplayBase display_base)
{
	char verbose_str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	bool objFound = true;

	objFound = GetAttrString(obj, inst, val, verbose_str, sizeof(verbose_str), access_mode, &notification, display_base);

	GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::CommandExtension), numberStr, sizeof(numberStr), BaseType::Numeric);

	if (objFound)
	{
		StringBuilder(GET_MSG_FRAME_TAG(AbccMsgField::CommandExtension), numberStr, verbose_str, notification);
	}
	else
	{
		StringBuilder(GET_MSG_FRAME_TAG(AbccMsgField::CommandExtension), numberStr, nullptr, NotifEvent::None);
	}
}

void SpiAnalyzerResults::BuildObjectString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetObjectString(val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::Object), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder(GET_MSG_FRAME_TAG(AbccMsgField::Object), numberStr, str, notification);
}

void SpiAnalyzerResults::BuildSpiCtrlString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetSpiCtrlString(val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::SpiControl), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder(GET_MOSI_FRAME_TAG(AbccMosiStates::SpiControl), numberStr, str, notification);
}

void SpiAnalyzerResults::BuildSpiStsString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetSpiStsString(val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::SpiStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder(GET_MISO_FRAME_TAG(AbccMisoStates::SpiStatus), numberStr, str, notification);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetErrorRspString(val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder("ERR_CODE", numberStr, str, notification);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 nw_type_idx, U8 obj, U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetErrorRspString(nw_type_idx, obj, val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr), BaseType::Numeric);

	if (nw_type_idx == 0)
	{
		StringBuilder("OBJ_ERR", numberStr, str, notification);
	}
	else
	{
		StringBuilder("NW_ERR", numberStr, str, notification);
	}
}

void SpiAnalyzerResults::BuildIntMask(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetIntMaskString(val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::InterruptMask), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder(GET_MOSI_FRAME_TAG(AbccMosiStates::InterruptMask), numberStr, str, notification);
}

void SpiAnalyzerResults::BuildAbccStatus(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetAbccStatusString(val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::AnybusStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder(GET_MISO_FRAME_TAG(AbccMisoStates::AnybusStatus), numberStr, str, notification);
}

void SpiAnalyzerResults::BuildApplStatus(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	/* Note ABCC documentation shows U16 datatype for status code, but SPI telegram is U8 */
	NotifEvent_t notification = GetApplStsString((U8)val, &str[0], sizeof(str), display_base);
	GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::ApplicationStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
	StringBuilder(GET_MOSI_FRAME_TAG(AbccMosiStates::ApplicationStatus), numberStr, str, notification);
}

void SpiAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel &channel, DisplayBase display_base)
{
	ClearResultStrings();
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char str[FORMATTED_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	Frame frame = GetFrame(frame_index);
	AbccSpiStatesUnion_t uState;
	uState.eMosi = (AbccMosiStates::Enum)frame.mType;

	if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
	{
		notification = NotifEvent::Alert;
		switch (frame.mType)
		{
		case AbccSpiError::Fragmentation:
			if ((IS_MISO_FRAME(frame) && (channel == mSettings->mMisoChannel)) ||
				(IS_MOSI_FRAME(frame) && (channel == mSettings->mMosiChannel)))
			{
				StringBuilder("FRAGMENT", nullptr, "Fragmented ABCC SPI Packet.", notification);
			}

			break;
		case AbccSpiError::EndOfTransfer:
			StringBuilder("CLOCKING", nullptr, "ABCC SPI Clocking. The analyzer expects one transaction per 'Active Enable' phase.", notification);
			break;
		case AbccSpiError::Generic:
		default:
			StringBuilder("ERROR", nullptr, "ABCC SPI Error.", notification);
			break;
		}
	}
	else
	{
		if ((channel == mSettings->mMosiChannel) && IS_MOSI_FRAME(frame))
		{
			switch (uState.eMosi)
			{
			case AbccMosiStates::Idle:
				break;
			case AbccMosiStates::SpiControl:
				BuildSpiCtrlString((U8)frame.mData1, display_base);
				break;
			case AbccMosiStates::Reserved1:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", notification);
				break;
			case AbccMosiStates::MessageLength:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
				SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, notification);
				break;
			case AbccMosiStates::ProcessDataLength:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
				SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, notification);
				break;
			case AbccMosiStates::ApplicationStatus:
				BuildApplStatus((U8)frame.mData1, display_base);
				break;
			case AbccMosiStates::InterruptMask:
				BuildIntMask((U8)frame.mData1, display_base);
				break;
			case AbccMosiStates::MessageField:
			case AbccMosiStates::MessageField_Data:
			{
				MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

				if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
				{
					if (info->msgDataCnt == 0)
					{
						BuildErrorRsp((U8)frame.mData1, display_base);
					}
					else
					{
						U8 nw_type_idx = 0;

						/* Bytes 1 and onward are always understood as object-specific
						** or network-specific error codes. The current implementation
						** supports only one extra byte in the error response message
						** data past the object specific or network-specific error */
						if (info->msgDataCnt == 2)
						{
							nw_type_idx = (U8)mSettings->mNetworkType;
						}

						BuildErrorRsp(nw_type_idx, info->msgHeader.obj, (U8)frame.mData1, display_base);
					}
				}
				else
				{
					BaseType type;

					if(((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
					   ((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
					   ((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
					   ((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
					{
						type = GetAttrBaseType(info->msgHeader.obj, info->msgHeader.inst, (U8)info->msgHeader.cmdExt);
					}
					else
					{
						type = GetCmdBaseType(info->msgHeader.obj, info->msgHeader.cmd);
					}

					GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), type);
					SNPRINTF(str, sizeof(str), " [%s] Byte #%d ", numberStr, info->msgDataCnt);

					if ((mSettings->mMsgDataPriority == DisplayPriority::Value))
					{
						/* Conditionally trim the leading 0x specifier */
						U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
						StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), &numberStr[offset], str, notification, DisplayPriority::Value);
					}
					else
					{
						StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, notification);
					}
				}

				break;
			}
			case AbccMosiStates::MessageField_DataNotValid:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(AbccMosiStates::MessageField_Data), numberStr, sizeof(numberStr));
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, nullptr, notification, DisplayPriority::Tag);
				break;
			case AbccMosiStates::MessageField_Size:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					SNPRINTF(str, sizeof(str), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
					notification = NotifEvent::Alert;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
				}

				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, notification);
				break;
			case AbccMosiStates::MessageField_Reserved1:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", notification);
				break;
			case AbccMosiStates::MessageField_SourceId:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, nullptr, notification);
				break;
			case AbccMosiStates::MessageField_Object:
				BuildObjectString((U8)frame.mData1, display_base);
				break;
			case AbccMosiStates::MessageField_Instance:
				BuildInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, display_base);
				break;
			case AbccMosiStates::MessageField_Command:
				BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
				break;
			case AbccMosiStates::MessageField_Reserved2:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", notification);
				break;
			case AbccMosiStates::MessageField_CommandExtension:
				if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
				{
					MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, AttributeAccessMode::Normal, display_base);
				}
				else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
						 ((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
				{
					MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, AttributeAccessMode::Indexed, display_base);
				}
				else
				{
					GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, nullptr, notification);
				}

				break;
			case AbccMosiStates::WriteProcessData:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
				SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", numberStr, frame.mData2);

				if ((mSettings->mProcessDataPriority == DisplayPriority::Value))
				{
					/* Conditionally trim the leading 0x specifier */
					U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), &numberStr[offset], str, notification, DisplayPriority::Value);
				}
				else
				{
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, notification);
				}

				break;
			case AbccMosiStates::Crc32:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
				{
					SNPRINTF(str, sizeof(str), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
					notification = NotifEvent::Alert;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
				}

				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, notification);
				break;
			case AbccMosiStates::Pad:
				GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, nullptr, notification);
				break;
			default:
				GetNumberString(frame.mData1, display_base, 8, numberStr, sizeof(numberStr), BaseType::Numeric);
				notification = NotifEvent::Alert;
				StringBuilder("UNKOWN", numberStr, "Internal Error: Unknown State", notification);
				break;
			}
		}
		else if ((channel == mSettings->mMisoChannel) && IS_MISO_FRAME(frame))
		{
			switch (uState.eMiso)
			{
			case AbccMisoStates::Idle:
				break;
			case AbccMisoStates::Reserved1:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);
				break;
			case AbccMisoStates::Reserved2:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);
				break;
			case AbccMisoStates::LedStatus:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
				notification = GetLedStatusString((U16)frame.mData1, str, sizeof(str), display_base);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, notification);
				break;
			case AbccMisoStates::AnybusStatus:
				BuildAbccStatus((U8)frame.mData1, display_base);
				break;
			case AbccMisoStates::SpiStatus:
				BuildSpiStsString((U8)frame.mData1, display_base);
				break;
			case AbccMisoStates::NetworkTime:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification);
				break;
			case AbccMisoStates::MessageField:
			case AbccMisoStates::MessageField_Data:
			{
				MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

				if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
				{
					if (info->msgDataCnt == 0)
					{
						BuildErrorRsp((U8)frame.mData1, display_base);
					}
					else
					{
						U8 nw_type_idx = 0;

						/* Bytes 1 and onward are always understood as object-specific
						** or network-specific error codes. The current implementation
						** supports only one extra byte in the error response message
						** data past the object specific or network-specific error */
						if (info->msgDataCnt == 2)
						{
							nw_type_idx = (U8)mSettings->mNetworkType;
						}

						BuildErrorRsp(nw_type_idx, info->msgHeader.obj, (U8)frame.mData1, display_base);
					}
				}
				else
				{
					BaseType type;

					if(((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
					   ((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
					   ((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
					   ((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
					{
						type = GetAttrBaseType(info->msgHeader.obj, info->msgHeader.inst, (U8)info->msgHeader.cmdExt);
					}
					else
					{
						type = GetCmdBaseType(info->msgHeader.obj, info->msgHeader.cmd);
					}

					GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), type);
					SNPRINTF(str, sizeof(str), " [%s] Byte #%d ", numberStr, info->msgDataCnt);

					if ((mSettings->mMsgDataPriority == DisplayPriority::Value))
					{
						/* Conditionally trim the leading 0x specifier */
						U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
						StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), &numberStr[offset], str, notification, DisplayPriority::Value);
					}
					else
					{
						StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, notification);
					}
				}

				break;
			}
			case AbccMisoStates::MessageField_DataNotValid:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::MessageField_Data), numberStr, sizeof(numberStr));
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification, DisplayPriority::Tag);
				break;
			case AbccMisoStates::MessageField_Size:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					SNPRINTF(str, sizeof(str), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
					notification = NotifEvent::Alert;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
				}

				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, notification);
				break;
			case AbccMisoStates::MessageField_Reserved1:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);
				break;
			case AbccMisoStates::MessageField_SourceId:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification);
				break;
			case AbccMisoStates::MessageField_Object:
				BuildObjectString((U8)frame.mData1, display_base);
				break;
			case AbccMisoStates::MessageField_Instance:
				BuildInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, display_base);
				break;
			case AbccMisoStates::MessageField_Command:
				BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
				break;
			case AbccMisoStates::MessageField_Reserved2:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

				if (frame.mData1 != 0)
				{
					notification = NotifEvent::Alert;
				}

				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);
				break;
			case AbccMisoStates::MessageField_CommandExtension:
				if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
				{
					MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, AttributeAccessMode::Normal, display_base);
				}
				else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
						 ((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
				{
					MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, AttributeAccessMode::Indexed, display_base);
				}
				else
				{
					GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification);
				}

				break;
			case AbccMisoStates::ReadProcessData:
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
				SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", numberStr, frame.mData2);

				if ((mSettings->mProcessDataPriority == DisplayPriority::Value))
				{
					/* Conditionally trim the leading 0x specifier */
					U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), &numberStr[offset], str, notification, DisplayPriority::Value);
				}
				else
				{
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, notification);
				}

				break;
			case AbccMisoStates::Crc32:
			{
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
				{
					SNPRINTF(str, sizeof(str), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1), (U32)(frame.mData2));
					notification = NotifEvent::Alert;
				}
				else
				{
					SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1), (U32)(frame.mData2));
				}

				StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, notification);
				break;
			}
			default:
				GetNumberString(frame.mData1, display_base, 8, numberStr, sizeof(numberStr), BaseType::Numeric);
				notification = NotifEvent::Alert;
				StringBuilder("UNKOWN", numberStr, "Internal Error: Unknown State", notification);
				break;
			}
		}
	}
}

void SpiAnalyzerResults::ExportAllFramesToFile(const char* file, DisplayBase display_base)
{
	std::stringstream ss;
	void *f = AnalyzerHelpers::StartFile(file);

	U64 triggerSample = mAnalyzer->GetTriggerSample();
	U32 sampleRate = mAnalyzer->GetSampleRate();
	U64 numFrames = GetNumFrames();

	ss << "Channel" + CSV_DELIMITER +
		  "Time [s]" + CSV_DELIMITER +
		  "Packet ID" + CSV_DELIMITER +
		  "Frame Type" + CSV_DELIMITER +
		  "Frame Data"
	   << std::endl;

	for (U32 i = 0; i < numFrames; i++)
	{
		Frame frame = GetFrame(i);
		U64 packetId = GetPacketContainingFrameSequential(i);
		char timestampStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
		char frameDataStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE] = "";

		AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timestampStr, sizeof(timestampStr));

		if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
		{
			ss << "ERROR";
		}
		else
		{
			if (IS_MOSI_FRAME(frame))
			{
				ss << MOSI_STR;
			}
			else
			{
				ss << MISO_STR;
			}

			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(frame.mType), frameDataStr, sizeof(frameDataStr));
		}

		ss << CSV_DELIMITER << timestampStr;

		if (packetId != INVALID_RESULT_INDEX)
		{
			ss << CSV_DELIMITER << packetId << CSV_DELIMITER;
		}
		else
		{
			ss << CSV_DELIMITER + CSV_DELIMITER;
		}

		if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
		{
			switch (frame.mType)
			{
			case AbccSpiError::Fragmentation:
				ss << "FRAGMENT";
				break;
			case AbccSpiError::EndOfTransfer:
				ss << "CLOCKING";
				break;
			case AbccSpiError::Generic:
			default:
				ss << "GENERIC";
				break;
			}
		}
		else
		{
			if (IS_MOSI_FRAME(frame))
			{
				ss << GET_MOSI_FRAME_TAG(frame.mType);
			}
			else
			{
				ss << GET_MISO_FRAME_TAG(frame.mType);
			}
		}

		ss << CSV_DELIMITER << frameDataStr << std::endl;

		AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), (U32)ss.str().length(), f);
		ss.str(std::string());

		if (UpdateExportProgressAndCheckForCancel(i, numFrames) == true)
		{
			AnalyzerHelpers::EndFile(f);
			return;
		}
	}

	UpdateExportProgressAndCheckForCancel(numFrames, numFrames);
	AnalyzerHelpers::EndFile(f);
}

void SpiAnalyzerResults::AppendCsvMessageEntry(void* file, std::stringstream &ss_csv_head, std::stringstream &ss_csv_body, std::stringstream &ss_csv_tail, ErrorEvent event)
{
	ss_csv_head << CSV_DELIMITER;

	switch (event)
	{
	case ErrorEvent::SpiFragmentationError:
		ss_csv_head << "SPI_ERROR";
		break;
	case ErrorEvent::RetransmitWarning:
		ss_csv_head << "RETRANSMIT";
		break;
	case ErrorEvent::CrcError:
		ss_csv_head << "CRC_ERROR";
		break;
	case ErrorEvent::None:
	default:
		break;
	}

	AnalyzerHelpers::AppendToFile((U8*)ss_csv_head.str().c_str(), (U32)ss_csv_head.str().length(), file);
	AnalyzerHelpers::AppendToFile((U8*)ss_csv_body.str().c_str(), (U32)ss_csv_body.str().length(), file);
	AnalyzerHelpers::AppendToFile((U8*)ss_csv_tail.str().c_str(), (U32)ss_csv_tail.str().length(), file);
}

void SpiAnalyzerResults::AppendCsvSafeString(std::stringstream &ss_csv_data, char* input_data_str)
{
	std::string csvStr;

	csvStr.assign(input_data_str);

	ss_csv_data << CSV_DELIMITER;

	if (csvStr.find('"') != std::string::npos)
	{
		/* Escape double-quotes */
		for (std::string::size_type n = 0;
			 (n = csvStr.find('"', n)) != std::string::npos;
			 n += 2)
		{
			csvStr.replace(n, 1, "\"\"");
		}

		ss_csv_data << '"' << csvStr << '"';
	}
	else if( csvStr.find( CSV_DELIMITER ) != std::string::npos )
	{
		/* Escape delimiter */
		for( std::string::size_type n = 0;
			( n = csvStr.find( CSV_DELIMITER, n ) ) != std::string::npos;
			n += 2 )
		{
			csvStr.replace( n, 1, "\"" + CSV_DELIMITER + "\"" );
		}

		ss_csv_data << '"' << csvStr << '"';
	}
	else if (csvStr.find("COMMA") != std::string::npos)
	{
		/* Replace with comma-character and surround with double quotes */
		ss_csv_data << "\",\"";
	}
	else if (csvStr.find("' '") != std::string::npos)
	{
		/* Replace with space-character */
		ss_csv_data << ' ';
	}
	else
	{
		/* No escaping needed */
		ss_csv_data << csvStr;
	}
}

void SpiAnalyzerResults::ExportMessageDataToFile(const char *file, DisplayBase display_base)
{
	std::stringstream ssMosiHead;
	std::stringstream ssMisoHead;
	std::stringstream ssMisoTail;
	std::stringstream ssMosiTail;
	std::stringstream ssSharedBody;
	char timeStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char dataStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool addMosiEntry;
	bool addMisoEntry;
	bool addLastMosiMsgHeader;
	bool addLastMisoMsgHeader;
	bool mosiFragmentation = false;
	bool misoFragmentation = false;
	bool mosiPreviousFragState = false;
	bool misoPreviousFragState = false;
	void* f = AnalyzerHelpers::StartFile(file);

	U64 triggerSample = mAnalyzer->GetTriggerSample();
	U32 sampleRate = mAnalyzer->GetSampleRate();
	U64 numFrames = GetNumFrames();
	U64 i = 0;

	/* Add header fields */
	ssMosiHead << "Channel" + CSV_DELIMITER +
				  "Time [s]" + CSV_DELIMITER +
				  "Packet ID" + CSV_DELIMITER +
				  "Error Event" + CSV_DELIMITER +
				  "Anybus State" + CSV_DELIMITER +
				  "Application State" + CSV_DELIMITER +
				  "Message Fragmentation" + CSV_DELIMITER +
				  "Message Size [bytes]" + CSV_DELIMITER +
				  "Source ID" + CSV_DELIMITER +
				  "Object" + CSV_DELIMITER +
				  "Instance" + CSV_DELIMITER +
				  "Command" + CSV_DELIMITER +
				  "CmdExt" + CSV_DELIMITER +
				  "Message Data";

	AnalyzerHelpers::AppendToFile((U8*)ssMosiHead.str().c_str(), (U32)ssMosiHead.str().length(), f);
	ssMosiHead.str(std::string());

	while (i < numFrames)
	{
		U64 packetId = GetPacketContainingFrameSequential(i);

		if (packetId != INVALID_RESULT_INDEX)
		{
			U64 firstFrameId;
			U64 lastFrameId;
			ErrorEvent mosiEvent = ErrorEvent::None;
			ErrorEvent misoEvent = ErrorEvent::None;
			bool mosiAppStatReached = false;
			bool misoAnbStatReached = false;

			addMosiEntry = false;
			addMisoEntry = false;
			addLastMosiMsgHeader = true;
			addLastMisoMsgHeader = true;

			GetFramesContainedInPacket(packetId, &firstFrameId, &lastFrameId);

			/* Iterate through packet and extract message header and data
			** stream is written only on receipt of "last fragment". */
			for (U64 frameId = firstFrameId; frameId <= lastFrameId; frameId++)
			{
				Frame frame = GetFrame(frameId);
				if (IS_MOSI_FRAME(frame))
				{
					switch (frame.mType)
					{
					case AbccSpiError::Fragmentation:
						mosiEvent = ErrorEvent::SpiFragmentationError;
						break;
					case AbccMosiStates::SpiControl:
					{
						bool message = ((frame.mData1 & ABP_SPI_CTRL_M) != 0);

						if (message)
						{
							/* Add in the timestamp, packet ID */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMosiHead << std::endl
									   << MOSI_STR + CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
							addMosiEntry = true;
						}

						if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
						{
							mosiEvent = ErrorEvent::RetransmitWarning;
							misoEvent = ErrorEvent::RetransmitWarning;
						}

						ssMosiTail << CSV_DELIMITER;

						if (message)
						{
							if (frame.mData1 & ABP_SPI_CTRL_LAST_FRAG)
							{
								if (mosiFragmentation)
								{
									ssMosiTail << LAST_FRAG_STR;
									mosiFragmentation = false;
								}
							}
							else
							{
								if (!mosiFragmentation)
								{
									mosiFragmentation = true;
									ssMosiTail << FIRST_FRAG_STR;
								}
								else
								{
									ssMosiTail << FRAGMENT_STR;
								}
							}
						}

						break;
					}
					case AbccMosiStates::ApplicationStatus:
					{
						mosiAppStatReached = true;
						GetApplStsString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMosiStates::MessageField_Size:
					case AbccMosiStates::MessageField_SourceId:
					{
						GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case AbccMosiStates::MessageField_Object:
					{
						GetObjectString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case AbccMosiStates::MessageField_Instance:
					{
						NotifEvent_t notification = NotifEvent::None;
						bool found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, dataStr, sizeof(dataStr), &notification, display_base);

						if (!found)
						{
							SNPRINTF(dataStr, sizeof(dataStr), "0x%04X", (U16)frame.mData1);
						}

						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case AbccMosiStates::MessageField_Command:
					{
						GetCmdString((U8)frame.mData1, (U8)frame.mData2, &dataStr[0], sizeof(dataStr), display_base);

						if (((U8)frame.mData1 & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
						{
							ssMosiTail << CSV_DELIMITER << dataStr << ERROR_RESPONSE_STR;
						}
						else
						{
							if (((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
							{
								ssMosiTail << CSV_DELIMITER << dataStr << COMMAND_STR;
							}
							else
							{
								ssMosiTail << CSV_DELIMITER << dataStr << RESPONSE_STR;
							}
						}

						addLastMosiMsgHeader = false;
						break;
					}
					case AbccMosiStates::MessageField_CommandExtension:
					{
						NotifEvent_t notification = NotifEvent::None;
						DisplayBase displayBase = DisplayBase::Hexadecimal;

						if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
							((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
						{
							MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), AttributeAccessMode::Normal, &notification, displayBase);
						}
						else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								 ((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
						{
							MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), AttributeAccessMode::Indexed, &notification, displayBase);
						}
						else
						{
							GetNumberString(frame.mData1, displayBase, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						}

						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case AbccMosiStates::MessageField_Data:
					case AbccMosiStates::MessageField:
					{
						MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

						if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
						{
							if (info->msgDataCnt == 0)
							{
								GetErrorRspString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
							else
							{
								GetErrorRspString((U8)mSettings->mNetworkType, info->msgHeader.obj, (U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
						}
						else
						{
							BaseType type;

							if (((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
								((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
								((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
							{
								type = GetAttrBaseType(info->msgHeader.obj, info->msgHeader.inst, (U8)info->msgHeader.cmdExt);
							}
							else
							{
								type = GetCmdBaseType(info->msgHeader.obj, info->msgHeader.cmd);
							}

							GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), type);
						}

						if (addLastMosiMsgHeader)
						{
							ssMosiTail << CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER;
							addLastMosiMsgHeader = false;
						}

						if ((display_base == DisplayBase::ASCII) ||
							(display_base == DisplayBase::AsciiHex))
						{
							AppendCsvSafeString(ssMosiTail, dataStr);
						}
						else
						{
							ssMosiTail << CSV_DELIMITER << dataStr;
						}

						break;
					}
					case AbccMosiStates::Crc32:
					{
						if ((U32)frame.mData1 != (U32)frame.mData2)
						{
							mosiEvent = ErrorEvent::CrcError;
						}

						break;
					}
					default:
						break;
					}
				}
				else
				{
					/* MISO Frame */
					switch (frame.mType)
					{
					case AbccSpiError::Fragmentation:
						misoEvent = ErrorEvent::SpiFragmentationError;
						break;
					case AbccMisoStates::AnybusStatus:
					{
						misoAnbStatReached = true;
						GetAbccStatusString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMisoStates::SpiStatus:
					{
						bool message = ((frame.mData1 & ABP_SPI_STATUS_M) != 0);

						if (message)
						{
							/* Add in the timestamp, packet ID, and Anybus state */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMisoHead << std::endl
									   << MISO_STR + CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
							addMisoEntry = true;
						}

						ssMisoTail << CSV_DELIMITER;

						if (message)
						{
							if (frame.mData1 & ABP_SPI_STATUS_LAST_FRAG)
							{
								if (misoFragmentation)
								{
									ssMisoTail << LAST_FRAG_STR;
									misoFragmentation = false;
								}
							}
							else
							{
								if (!misoFragmentation)
								{
									misoFragmentation = true;
									ssMisoTail << FIRST_FRAG_STR;
								}
								else
								{
									ssMisoTail << FRAGMENT_STR;
								}
							}
						}

						break;
					}
					case AbccMisoStates::MessageField_Size:
					case AbccMisoStates::MessageField_SourceId:
					{
						GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case AbccMisoStates::MessageField_Object:
					{
						GetObjectString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case AbccMisoStates::MessageField_Instance:
					{
						NotifEvent_t notification = NotifEvent::None;
						bool found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, dataStr, sizeof(dataStr), &notification, display_base);

						if (!found)
						{
							SNPRINTF(dataStr, sizeof(dataStr), "0x%04X", (U16)frame.mData1);
						}

						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case AbccMisoStates::MessageField_Command:
					{
						GetCmdString((U8)frame.mData1, (U8)frame.mData2, &dataStr[0], sizeof(dataStr), display_base);

						if (((U8)frame.mData1 & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
						{
							ssMisoTail << CSV_DELIMITER << dataStr << ERROR_RESPONSE_STR;
						}
						else
						{
							if (((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
							{
								ssMisoTail << CSV_DELIMITER << dataStr << COMMAND_STR;
							}
							else
							{
								ssMisoTail << CSV_DELIMITER << dataStr << RESPONSE_STR;
							}
						}

						addLastMisoMsgHeader = false;
						break;
					}
					case AbccMisoStates::MessageField_CommandExtension:
					{
						NotifEvent_t notification = NotifEvent::None;
						DisplayBase displayBase = DisplayBase::Hexadecimal;

						if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
							((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
						{
							MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), AttributeAccessMode::Normal, &notification, displayBase);
						}
						else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								 ((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
						{
							MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), AttributeAccessMode::Indexed, &notification, displayBase);
						}
						else
						{
							GetNumberString(frame.mData1, displayBase, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						}

						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case AbccMisoStates::MessageField_Data:
					case AbccMisoStates::MessageField:
					{
						MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

						if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
						{
							if (info->msgDataCnt == 0)
							{
								GetErrorRspString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
							else
							{
								GetErrorRspString((U8)mSettings->mNetworkType, info->msgHeader.obj, (U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
						}
						else
						{
							BaseType type;

							if (((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
								((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
								((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								((ABP_MsgCmdType)(info->msgHeader.cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
							{
								type = GetAttrBaseType(info->msgHeader.obj, info->msgHeader.inst, (U8)info->msgHeader.cmdExt);
							}
							else
							{
								type = GetCmdBaseType(info->msgHeader.obj, info->msgHeader.cmd);
							}

							GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), type);
						}

						if (addLastMisoMsgHeader)
						{
							ssMisoTail << CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER + CSV_DELIMITER;
							addLastMisoMsgHeader = false;
						}

						if ((display_base == DisplayBase::ASCII) ||
							(display_base == DisplayBase::AsciiHex))
						{
							AppendCsvSafeString(ssMisoTail, dataStr);
						}
						else
						{
							ssMisoTail << CSV_DELIMITER << dataStr;
						}

						break;
					}
					case AbccMisoStates::Crc32:
					{
						if ((U32)frame.mData1 != (U32)frame.mData2)
						{
							misoEvent = ErrorEvent::CrcError;
							mosiEvent = ErrorEvent::CrcError;
						}

						break;
					}
					default:
						break;
					}
				}
			}

			if ((mosiEvent == ErrorEvent::SpiFragmentationError) ||
				(misoEvent == ErrorEvent::SpiFragmentationError))
			{
				mosiFragmentation = mosiPreviousFragState;
				misoFragmentation = misoPreviousFragState;

				/* Determine if additional tabs need to be added to get correct alignment in CSV */
				if (!mosiAppStatReached)
				{
					ssSharedBody << CSV_DELIMITER;
				}

				if (!misoAnbStatReached)
				{
					ssSharedBody << CSV_DELIMITER;
				}
			}

			if (addMosiEntry)
			{
				if (mosiEvent == ErrorEvent::None)
				{
					mosiPreviousFragState = mosiFragmentation;
				}

				AppendCsvMessageEntry(f, ssMosiHead, ssSharedBody, ssMosiTail, mosiEvent);
			}

			if (addMisoEntry)
			{
				if (misoEvent == ErrorEvent::None)
				{
					misoPreviousFragState = misoFragmentation;
				}

				AppendCsvMessageEntry(f, ssMisoHead, ssSharedBody, ssMisoTail, misoEvent);
			}

			ssMisoHead.str(std::string());
			ssMosiHead.str(std::string());
			ssMisoTail.str(std::string());
			ssMosiTail.str(std::string());
			ssSharedBody.str(std::string());

			/* Jump to the next frame after the processed packet */
			i = lastFrameId + 1;
		}
		else
		{
			/* Jump to next frame */
			i++;
		}

		if (UpdateExportProgressAndCheckForCancel(i, numFrames) == true)
		{
			AnalyzerHelpers::EndFile(f);
			return;
		}
	}

	UpdateExportProgressAndCheckForCancel(numFrames, numFrames);
	AnalyzerHelpers::EndFile(f);
}

void SpiAnalyzerResults::ExportProcessDataToFile(const char* file, DisplayBase display_base)
{
	std::stringstream ssMosiHead;
	std::stringstream ssMisoHead;
	std::stringstream ssMosiTail;
	std::stringstream ssMisoTail;
	std::stringstream ssSharedBody;
	void* f = AnalyzerHelpers::StartFile(file);
	bool addCsvHeader = true;

	U64 triggerSample = mAnalyzer->GetTriggerSample();
	U32 sampleRate = mAnalyzer->GetSampleRate();
	U64 numFrames = GetNumFrames();
	U64 i = 0;

	while (i < numFrames)
	{
		U64 packetId = GetPacketContainingFrameSequential(i);
		char timeStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
		char dataStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE] = "";
		bool addMosiEntry = false;
		bool addMisoEntry = false;

		if (packetId != INVALID_RESULT_INDEX)
		{
			U64 firstFrameId;
			U64 lastFrameId;
			ErrorEvent mosiEvent = ErrorEvent::None;
			ErrorEvent misoEvent = ErrorEvent::None;
			bool mosiAppStatReached = false;
			bool misoAnbStatReached = false;

			GetFramesContainedInPacket(packetId, &firstFrameId, &lastFrameId);

			/* Iterate through packet and extract message header and data
			** stream is written only on receipt of "last fragment". */
			for (U64 frameId = firstFrameId; frameId <= lastFrameId; frameId++)
			{
				Frame frame = GetFrame(frameId);

				if (IS_MOSI_FRAME(frame))
				{
					switch (frame.mType)
					{
					case AbccSpiError::Fragmentation:
						mosiEvent = ErrorEvent::SpiFragmentationError;
						break;
					case AbccMosiStates::ProcessDataLength:
						if (addCsvHeader)
						{
							U32 dwBytes = ((U16)frame.mData1) << 1;
							/* Add header fields */
							std::stringstream ssHeader;
							ssHeader << "Channel" + CSV_DELIMITER +
										"Time [s]" + CSV_DELIMITER +
										"Packet ID" + CSV_DELIMITER +
										"Error Event" + CSV_DELIMITER +
										"Anybus State" + CSV_DELIMITER +
										"Application State" + CSV_DELIMITER +
										"Network Time";

							for (U16 cnt = 0; cnt < dwBytes; cnt++)
							{
								ssHeader << CSV_DELIMITER + "Process Data " << cnt;
								AnalyzerHelpers::AppendToFile((U8*)ssHeader.str().c_str(), (U32)ssHeader.str().length(), f);
								ssHeader.str(std::string());
							}

							AnalyzerHelpers::AppendToFile((U8*)ssHeader.str().c_str(), (U32)ssHeader.str().length(), f);
							addCsvHeader = false;
						}

						break;
					case AbccMosiStates::SpiControl:
					{
						if (frame.mData1 & ABP_SPI_CTRL_WRPD_VALID)
						{
							/* Add in the timestamp, packet ID */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMosiHead << std::endl
									   << MOSI_STR + CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
							addMosiEntry = true;
						}

						break;
					}
					case AbccMosiStates::ApplicationStatus:
					{
						mosiAppStatReached = true;
						GetApplStsString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMosiStates::WriteProcessData:
					{
						GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						ssMosiTail << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMosiStates::Crc32:
					{
						if ((U32)frame.mData1 != (U32)frame.mData2)
						{
							mosiEvent = ErrorEvent::CrcError;
						}

						break;
					}
					default:
						break;
					}
				}
				else
				{
					/* MISO Frame */
					switch (frame.mType)
					{
					case AbccSpiError::Fragmentation:
						misoEvent = ErrorEvent::SpiFragmentationError;
						break;
					case AbccMisoStates::AnybusStatus:
					{
						misoAnbStatReached = true;
						GetAbccStatusString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMisoStates::SpiStatus:
					{
						if (frame.mData1 & ABP_SPI_STATUS_NEW_PD)
						{
							/* Add in the timestamp, packet ID */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMisoHead << std::endl
									   << MISO_STR + CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
							addMisoEntry = true;
						}

						break;
					}
					case AbccMisoStates::NetworkTime:
					{
						/* Append network time stamp to both string streams */
						GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						ssMisoTail << CSV_DELIMITER << dataStr;
						ssMosiTail << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMisoStates::ReadProcessData:
					{
						GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
						ssMisoTail << CSV_DELIMITER << dataStr;
						break;
					}
					case AbccMisoStates::Crc32:
					{
						if ((U32)frame.mData1 != (U32)frame.mData2)
						{
							misoEvent = ErrorEvent::CrcError;
							mosiEvent = ErrorEvent::CrcError;
						}

						break;
					}
					default:
						break;
					}
				}

				if (UpdateExportProgressAndCheckForCancel(i, numFrames) == true)
				{
					AnalyzerHelpers::EndFile(f);
					return;
				}
			}

			if ((mosiEvent == ErrorEvent::SpiFragmentationError) ||
				(misoEvent == ErrorEvent::SpiFragmentationError))
			{
				/* Determine if additional tabs need to be added to get correct alignment in CSV */
				if (!mosiAppStatReached)
				{
					ssSharedBody << CSV_DELIMITER;
				}

				if (!misoAnbStatReached)
				{
					ssSharedBody << CSV_DELIMITER;
				}
			}

			if (addMosiEntry)
			{
				AppendCsvMessageEntry(f, ssMosiHead, ssSharedBody, ssMosiTail, mosiEvent);
			}

			if (addMisoEntry)
			{
				AppendCsvMessageEntry(f, ssMisoHead, ssSharedBody, ssMisoTail, misoEvent);
			}

			ssMisoHead.str(std::string());
			ssMosiHead.str(std::string());
			ssMisoTail.str(std::string());
			ssMosiTail.str(std::string());
			ssSharedBody.str(std::string());

			/* Jump to the next frame after the processed packet */
			i = lastFrameId + 1;
		}
		else
		{
			/* Jump to next frame */
			i++;

			if (UpdateExportProgressAndCheckForCancel(i, numFrames) == true)
			{
				AnalyzerHelpers::EndFile(f);
				return;
			}
		}
	}

	UpdateExportProgressAndCheckForCancel(numFrames, numFrames);
	AnalyzerHelpers::EndFile(f);
}

void SpiAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 export_type_user_id)
{
	switch (static_cast<ExportType>(export_type_user_id))
	{
	case ExportType::Frames:
		/* Export all frame data */
		ExportAllFramesToFile(file, display_base);
		break;
	case ExportType::MessageData:
		/* Export 'valid' message data */
		ExportMessageDataToFile(file, display_base);
		break;
	case ExportType::ProcessData:
		/* Export 'valid' process data */
		ExportProcessDataToFile(file, display_base);
		break;
	}
}

U64 SpiAnalyzerResults::GetFrameIdOfAbccFieldContainedInPacket(U64 packet_index, SpiChannel_t e_channel, U8 type)
{
	U64 frameIndex = INVALID_RESULT_INDEX;
	U64 firstFrameIndex;
	U64 lastFrameIndex;

	if (packet_index != INVALID_RESULT_INDEX)
	{
		GetFramesContainedInPacket(packet_index, &firstFrameIndex, &lastFrameIndex);
		if ((firstFrameIndex != INVALID_RESULT_INDEX) && (lastFrameIndex != INVALID_RESULT_INDEX))
		{
			for (frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++)
			{
				Frame frame = GetFrame(frameIndex);
				if (((e_channel == SpiChannel::MOSI) && IS_MOSI_FRAME(frame) && (frame.mType == type)) ||
					((e_channel == SpiChannel::MISO) && IS_MISO_FRAME(frame) && (frame.mType == type)))
				{
					break;
				}
			}
		}
	}
	return frameIndex;
}

void SpiAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
	ClearTabularText();
	U64 packetId = GetPacketContainingFrame(frame_index);
	Frame frame = GetFrame(frame_index);
	char str[FORMATTED_STRING_BUFFER_SIZE];

	if (mSettings->mErrorIndexing)
	{
		if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
		{
			/* These types of errors effect the bus as a whole and are not
			** specific to a MISO/MOSI channel. For compatibility, only
			** show one for tabular text */
			if (IS_MISO_FRAME(frame))
			{
				switch (frame.mType)
				{
				case AbccSpiError::Fragmentation:
					AddTabularText("!FRAGMENT: ABCC SPI Packet is Fragmented");
					break;
				case AbccSpiError::EndOfTransfer:
					AddTabularText("!CLOCKING: Unexpected ABCC SPI Clocking Behavior");
					break;
				case AbccSpiError::Generic:
				default:
					AddTabularText("!ERROR: General Error in ABCC SPI Communication");
					break;
				}
			}
			return;
		}

		if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
		{
			if (IS_MOSI_FRAME(frame))
			{
				if (frame.mType == AbccMisoStates::Crc32)
				{
					TableBuilder(SpiChannel::MOSI, "CRC32", NotifEvent::Alert);
					return;
				}
				else if (frame.mType == AbccMosiStates::ApplicationStatus)
				{
					/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
					if (GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
					{
						AddTabularText("!Application Status: ", str);
						return;
					}
					else
					{
						if (mSettings->mApplStatusIndexing)
						{
							AddTabularText("Application Status: ", str);
							return;
						}
					}
				}
				else if (frame.mType == AbccMosiStates::MessageField_Size)
				{
					TableBuilder(SpiChannel::MOSI, "Message Size: Exceeds Maximum", NotifEvent::Alert);
					return;
				}
			}
			else /* MISO Frame */
			{
				if (frame.mType == AbccMisoStates::Crc32)
				{
					TableBuilder(SpiChannel::MISO, "CRC32", NotifEvent::Alert);
					return;
				}
				else if (frame.mType == AbccMisoStates::AnybusStatus)
				{
					char tabText[FORMATTED_STRING_BUFFER_SIZE];
					NotifEvent_t notification = NotifEvent::None;
					if (GetAbccStatusString((U8)frame.mData1, &str[0], sizeof(str), display_base))
					{
						notification = NotifEvent::Alert;
					}

					if ((notification == NotifEvent::Alert) || (mSettings->mAnybusStatusIndexing))
					{
						SNPRINTF(tabText, sizeof(tabText), "Anybus Status: (%s)", str);
						TableBuilder(SpiChannel::MISO, tabText, notification);
					}
					return;
				}
				else if (frame.mType == AbccMisoStates::MessageField_Size)
				{
					TableBuilder(SpiChannel::MISO, "Message Size: Exceeds Maximum", NotifEvent::Alert);
					return;
				}
			}
		}
	}

	if ((frame.mType == AbccMisoStates::NetworkTime) && IS_MISO_FRAME(frame))
	{
		bool addEntry = false;

		switch (mSettings->mTimestampIndexing)
		{
		case TimestampIndexing::AllPackets:
			addEntry = true;
			break;
		case TimestampIndexing::WriteProcessDataValid:
			if (((NetworkTimeInfo_t*)&frame.mData2)->wrPdValid)
			{
				addEntry = true;
			}
			break;
		case TimestampIndexing::NewReadProcessData:
			if (((NetworkTimeInfo_t*)&frame.mData2)->newRdPd)
			{
				addEntry = true;
			}
			break;
		default:
		case TimestampIndexing::Disabled:
			break;
		}

		if (addEntry)
		{
			U32 delta = ((NetworkTimeInfo_t*)&frame.mData2)->deltaTime;
			SNPRINTF(str, sizeof(str), "0x%08X (Delta: 0x%08X)", (U32)frame.mData1, delta);
			AddTabularText("Time: ", str);
			SNPRINTF(str, sizeof(str), "Packet: 0x%016llX", packetId);
			AddTabularText(str);
		}

		return;
	}

	if (mSettings->mApplStatusIndexing)
	{
		if ((frame.mType == AbccMosiStates::ApplicationStatus) && IS_MOSI_FRAME(frame))
		{
			if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
			{
				/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
				if (GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
				{
					AddTabularText("!Application Status: ", str);
				}
				else
				{
					AddTabularText("Application Status: ", str);
				}
				return;
			}
		}
	}

	if (mSettings->mAnybusStatusIndexing)
	{
		if ((frame.mType == AbccMisoStates::AnybusStatus) && IS_MISO_FRAME(frame))
		{
			if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
			{
				if (GetAbccStatusString((U8)frame.mData1, &str[0], sizeof(str), display_base))
				{
					AddTabularText("!Anybus Status: ", str);
				}
				else
				{
					AddTabularText("Anybus Status: ", str);
				}
				return;
			}
		}
	}

	/* Since tabular text is sequentially processed and indexed,
	** buffer the "Object", "Instance", "Cmd", and "Ext";
	** then add as a single text entry. */
	if (mSettings->mMessageIndexingVerbosityLevel != MessageIndexing::Disabled)
	{
		if (IS_MISO_FRAME(frame))
		{
			switch (frame.mType)
			{
			case AbccMisoStates::SpiStatus:
				if ((frame.mData1 & ABP_SPI_STATUS_M) != 0)
				{
					mMsgValidFlag[SpiChannel::MISO] = true;
				}
				else
				{
					mMsgValidFlag[SpiChannel::MISO] = false;
				}

				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG)) == SPI_PROTO_EVENT_FLAG)
				{
					TableBuilder(SpiChannel::MISO, "{Write Message Buffer Full}", NotifEvent::None);
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
							TableBuilder(SpiChannel::MISO, "{Message Fragment}", NotifEvent::None);
						}
						else
						{
							/* More fragments follow */
							TableBuilder(SpiChannel::MISO, "{Message Fragment}++", NotifEvent::None);
						}
					}

					return;
				}

				break;
			case AbccMisoStates::MessageField_Size:
				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					SNPRINTF(mMsgSizeStr[SpiChannel::MISO], sizeof(mMsgSizeStr[SpiChannel::MISO]), "!Size: %u Bytes", (U16)frame.mData1);
				}
				else
				{
					SNPRINTF(mMsgSizeStr[SpiChannel::MISO], sizeof(mMsgSizeStr[SpiChannel::MISO]), "Size: %u Bytes", (U16)frame.mData1);
				}

				break;
			case AbccMisoStates::MessageField_SourceId:
				SNPRINTF(mMsgSrcStr[SpiChannel::MISO], sizeof(mMsgSrcStr[SpiChannel::MISO]), "Source ID: %d (0x%02X)", (U8)frame.mData1, (U8)frame.mData1);
				break;
			case AbccMisoStates::MessageField_Object:
				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					NotifEvent_t notification = GetObjectString((U8)frame.mData1, &str[0], sizeof(str), display_base);

					if (notification == NotifEvent::Alert)
					{
						SNPRINTF(mMsgObjStr[SpiChannel::MISO], sizeof(mMsgObjStr[SpiChannel::MISO]), "!Object: %s", str);
					}
					else
					{
						SNPRINTF(mMsgObjStr[SpiChannel::MISO], sizeof(mMsgObjStr[SpiChannel::MISO]), "Object: %s", str);
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					SNPRINTF(mMsgObjStr[SpiChannel::MISO], sizeof(mMsgObjStr[SpiChannel::MISO]), "Obj {%02X:", (U8)frame.mData1);
				}

				break;
			case AbccMisoStates::MessageField_Instance:
				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					bool found = false;
					NotifEvent_t notification = NotifEvent::None;
					found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, str, sizeof(str), &notification, display_base);

					if (!found)
					{
						SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
					}

					if (notification)
					{
						SNPRINTF(mMsgInstStr[SpiChannel::MISO], sizeof(mMsgInstStr[SpiChannel::MISO]), "!Instance: %s", str);
					}
					else
					{
						SNPRINTF(mMsgInstStr[SpiChannel::MISO], sizeof(mMsgInstStr[SpiChannel::MISO]), "Instance: %s", str);
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					SNPRINTF(mMsgInstStr[SpiChannel::MISO], sizeof(mMsgInstStr[SpiChannel::MISO]), "%04Xh}", (U16)frame.mData1);
				}

				break;
			case AbccMisoStates::MessageField_Command:
				if ((frame.mData1 & ABP_MSG_HEADER_E_BIT) != 0)
				{
					mMsgErrorRspFlag[SpiChannel::MISO] = true;
				}
				else
				{
					mMsgErrorRspFlag[SpiChannel::MISO] = false;
				}

				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					NotifEvent_t notification = GetCmdString((U8)frame.mData1, (U8)frame.mData2, &str[0], sizeof(str), display_base);

					if ((mMsgErrorRspFlag[SpiChannel::MISO] == true) || (notification == NotifEvent::Alert))
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MISO], sizeof(mMsgCmdStr[SpiChannel::MISO]), "!Command: %s", str);
						}
						else
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MISO], sizeof(mMsgCmdStr[SpiChannel::MISO]), "!Response: %s", str);
						}
					}
					else
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MISO], sizeof(mMsgCmdStr[SpiChannel::MISO]), "Command: %s", str);
						}
						else
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MISO], sizeof(mMsgCmdStr[SpiChannel::MISO]), "Response: %s", str);
						}
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
					{
						SNPRINTF(mMsgCmdStr[SpiChannel::MISO], sizeof(mMsgCmdStr[SpiChannel::MISO]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
					}
					else
					{
						SNPRINTF(mMsgCmdStr[SpiChannel::MISO], sizeof(mMsgCmdStr[SpiChannel::MISO]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
					}
				}

				break;
			case AbccMisoStates::MessageField_CommandExtension:
				if (mMsgValidFlag[SpiChannel::MISO])
				{
					if ((mSettings->mMessageSrcIdIndexing) ||
						(mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed))
					{
						AddTabularText("-----MISO MESSAGE-----");
					}

					if (mSettings->mMessageSrcIdIndexing)
					{
						TableBuilder(SpiChannel::MISO, mMsgSrcStr[SpiChannel::MISO], NotifEvent::None);
					}

					if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
					{
						NotifEvent_t notification = NotifEvent::None;
						bool found = false;
						bool attrCmd = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
						bool attrIdx = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));

						if (attrCmd)
						{
							MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;

							if (attrIdx)
							{
								found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, str, sizeof(str), AttributeAccessMode::Indexed, &notification, display_base);
							}
							else
							{
								found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, str, sizeof(str), AttributeAccessMode::Normal, &notification, display_base);
							}
						}

						if (!found)
						{
							/* For consistency with Source ID and Instance, only use hex format */
							SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
						}

						if (notification == NotifEvent::Alert)
						{
							SNPRINTF(mMsgExtStr[SpiChannel::MISO], sizeof(mMsgExtStr[SpiChannel::MISO]), "!Extension: %s", str);
						}
						else
						{
							SNPRINTF(mMsgExtStr[SpiChannel::MISO], sizeof(mMsgExtStr[SpiChannel::MISO]), "Extension: %s", str);
						}

						TableBuilder(SpiChannel::MISO, mMsgSizeStr[SpiChannel::MISO], NotifEvent::None);
						TableBuilder(SpiChannel::MISO, mMsgObjStr[SpiChannel::MISO], NotifEvent::None);
						TableBuilder(SpiChannel::MISO, mMsgInstStr[SpiChannel::MISO], NotifEvent::None);
						TableBuilder(SpiChannel::MISO, mMsgCmdStr[SpiChannel::MISO], NotifEvent::None);
						TableBuilder(SpiChannel::MISO, mMsgExtStr[SpiChannel::MISO], NotifEvent::None);

						if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
						{
							TableBuilder(SpiChannel::MISO, "First Fragment; More Follow.", NotifEvent::None);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
					{
						SNPRINTF(mMsgExtStr[SpiChannel::MISO], sizeof(mMsgExtStr[SpiChannel::MISO]), "%04Xh}", (U16)frame.mData1);

						if (mMsgErrorRspFlag[SpiChannel::MISO])
						{
							AddTabularText(MISO_TAG_STR, "!",
										mMsgObjStr[SpiChannel::MISO],
										mMsgInstStr[SpiChannel::MISO],
										mMsgCmdStr[SpiChannel::MISO],
										mMsgExtStr[SpiChannel::MISO]);
						}
						else
						{
							if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								AddTabularText(MISO_TAG_STR,
											mMsgObjStr[SpiChannel::MISO],
											mMsgInstStr[SpiChannel::MISO],
											mMsgCmdStr[SpiChannel::MISO],
											mMsgExtStr[SpiChannel::MISO],
											"++");
							}
							else
							{
								AddTabularText(MISO_TAG_STR,
											mMsgObjStr[SpiChannel::MISO],
											mMsgInstStr[SpiChannel::MISO],
											mMsgCmdStr[SpiChannel::MISO],
											mMsgExtStr[SpiChannel::MISO]);
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
			case AbccMosiStates::SpiControl:
				if ((frame.mData1 & ABP_SPI_CTRL_M) != 0)
				{
					mMsgValidFlag[SpiChannel::MOSI] = true;
				}
				else
				{
					mMsgValidFlag[SpiChannel::MOSI] = false;
				}

				if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG)) == SPI_PROTO_EVENT_FLAG)
				{
					if (frame_index != 0)
					{
						TableBuilder(SpiChannel::MOSI, "{Message Retransmit}", NotifEvent::None);
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
							TableBuilder(SpiChannel::MOSI, "{Message Fragment}", NotifEvent::None);
						}
						else
						{
							/* More fragments follow */
							TableBuilder(SpiChannel::MOSI, "{Message Fragment}++", NotifEvent::None);
						}
					}

					return;
				}

				break;
			case AbccMisoStates::MessageField_Size:
				if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
				{
					SNPRINTF(mMsgSizeStr[SpiChannel::MOSI], sizeof(mMsgSizeStr[SpiChannel::MOSI]), "!Size: %u Bytes", (U16)frame.mData1);
				}
				else
				{
					SNPRINTF(mMsgSizeStr[SpiChannel::MOSI], sizeof(mMsgSizeStr[SpiChannel::MOSI]), "Size: %u Bytes", (U16)frame.mData1);
				}

				break;
			case AbccMosiStates::MessageField_SourceId:
				SNPRINTF(mMsgSrcStr[SpiChannel::MOSI], sizeof(mMsgSrcStr[SpiChannel::MOSI]), "Source ID: %d (0x%02X)", (U8)frame.mData1, (U8)frame.mData1);
				break;
			case AbccMosiStates::MessageField_Object:
				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					NotifEvent_t notification = GetObjectString((U8)frame.mData1, &str[0], sizeof(str), display_base);

					if (notification == NotifEvent::Alert)
					{
						SNPRINTF(mMsgObjStr[SpiChannel::MOSI], sizeof(mMsgObjStr[SpiChannel::MOSI]), "!Object: %s", str);
					}
					else
					{
						SNPRINTF(mMsgObjStr[SpiChannel::MOSI], sizeof(mMsgObjStr[SpiChannel::MOSI]), "Object: %s", str);
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					SNPRINTF(mMsgObjStr[SpiChannel::MOSI], sizeof(mMsgObjStr[SpiChannel::MOSI]), "Obj {%02X:", (U8)frame.mData1);
				}

				break;
			case AbccMosiStates::MessageField_Instance:
				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					bool found = false;
					NotifEvent_t notification = NotifEvent::None;
					found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, str, sizeof(str), &notification, display_base);

					if (!found)
					{
						SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
					}

					if (notification)
					{
						SNPRINTF(mMsgInstStr[SpiChannel::MOSI], sizeof(mMsgInstStr[SpiChannel::MOSI]), "!Instance: %s", str);
					}
					else
					{
						SNPRINTF(mMsgInstStr[SpiChannel::MOSI], sizeof(mMsgInstStr[SpiChannel::MOSI]), "Instance: %s", str);
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					SNPRINTF(mMsgInstStr[SpiChannel::MOSI], sizeof(mMsgInstStr[SpiChannel::MOSI]), "%04Xh}", (U16)frame.mData1);
				}

				break;
			case AbccMosiStates::MessageField_Command:
				if ((frame.mData1 & ABP_MSG_HEADER_E_BIT) != 0)
				{
					mMsgErrorRspFlag[SpiChannel::MOSI] = true;
				}
				else
				{
					mMsgErrorRspFlag[SpiChannel::MOSI] = false;
				}

				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					NotifEvent_t notification = GetCmdString((U8)frame.mData1, (U8)frame.mData2, &str[0], sizeof(str), display_base);

					if ((mMsgErrorRspFlag[SpiChannel::MOSI] == true) || (notification == NotifEvent::Alert))
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MOSI], sizeof(mMsgCmdStr[SpiChannel::MOSI]), "!Command: %s", str);
						}
						else
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MOSI], sizeof(mMsgCmdStr[SpiChannel::MOSI]), "!Response: %s", str);
						}
					}
					else
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MOSI], sizeof(mMsgCmdStr[SpiChannel::MOSI]), "Command: %s", str);
						}
						else
						{
							SNPRINTF(mMsgCmdStr[SpiChannel::MOSI], sizeof(mMsgCmdStr[SpiChannel::MOSI]), "Response: %s", str);
						}
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
					{
						SNPRINTF(mMsgCmdStr[SpiChannel::MOSI], sizeof(mMsgCmdStr[SpiChannel::MOSI]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
					}
					else
					{
						SNPRINTF(mMsgCmdStr[SpiChannel::MOSI], sizeof(mMsgCmdStr[SpiChannel::MOSI]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
					}
				}

				break;
			case AbccMosiStates::MessageField_CommandExtension:
				if (mMsgValidFlag[SpiChannel::MOSI])
				{
					if ((mSettings->mMessageSrcIdIndexing) ||
						(mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed))
					{
						AddTabularText("-----MOSI MESSAGE-----");
					}

					if (mSettings->mMessageSrcIdIndexing)
					{
						TableBuilder(SpiChannel::MOSI, mMsgSrcStr[SpiChannel::MOSI], NotifEvent::None);
					}

					if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
					{
						NotifEvent_t notification = NotifEvent::None;
						bool found = false;
						bool attrCmd = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
						bool attrIdx = (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
										((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));

						if (attrCmd)
						{
							MsgHeaderInfo_t* psMsgHdr = (MsgHeaderInfo_t*)&frame.mData2;

							if (attrIdx)
							{
								found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, str, sizeof(str), AttributeAccessMode::Indexed, &notification, display_base);
							}
							else
							{
								found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, str, sizeof(str), AttributeAccessMode::Normal, &notification, display_base);
							}
						}

						if (!found)
						{
							/* For consistency with Source ID and Instance, only use hex format */
							SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
						}

						if (notification == NotifEvent::Alert)
						{
							SNPRINTF(mMsgExtStr[SpiChannel::MOSI], sizeof(mMsgExtStr[SpiChannel::MOSI]), "!Extension: %s", str);
						}
						else
						{
							SNPRINTF(mMsgExtStr[SpiChannel::MOSI], sizeof(mMsgExtStr[SpiChannel::MOSI]), "Extension: %s", str);
						}

						TableBuilder(SpiChannel::MOSI, mMsgSizeStr[SpiChannel::MOSI], NotifEvent::None);
						TableBuilder(SpiChannel::MOSI, mMsgObjStr[SpiChannel::MOSI], NotifEvent::None);
						TableBuilder(SpiChannel::MOSI, mMsgInstStr[SpiChannel::MOSI], NotifEvent::None);
						TableBuilder(SpiChannel::MOSI, mMsgCmdStr[SpiChannel::MOSI], NotifEvent::None);
						TableBuilder(SpiChannel::MOSI, mMsgExtStr[SpiChannel::MOSI], NotifEvent::None);

						if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
						{
							TableBuilder(SpiChannel::MOSI, "First Fragment; More Follow.", NotifEvent::None);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
					{
						SNPRINTF(mMsgExtStr[SpiChannel::MOSI], sizeof(mMsgExtStr[SpiChannel::MOSI]), "%04Xh}", (U16)frame.mData1);

						if (mMsgErrorRspFlag[SpiChannel::MOSI])
						{
							AddTabularText(MOSI_TAG_STR, "!",
										mMsgObjStr[SpiChannel::MOSI],
										mMsgInstStr[SpiChannel::MOSI],
										mMsgCmdStr[SpiChannel::MOSI],
										mMsgExtStr[SpiChannel::MOSI]);
						}
						else
						{
							if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								AddTabularText(MOSI_TAG_STR,
											mMsgObjStr[SpiChannel::MOSI],
											mMsgInstStr[SpiChannel::MOSI],
											mMsgCmdStr[SpiChannel::MOSI],
											mMsgExtStr[SpiChannel::MOSI],
											"++");
							}
							else
							{
								AddTabularText(MOSI_TAG_STR,
											mMsgObjStr[SpiChannel::MOSI],
											mMsgInstStr[SpiChannel::MOSI],
											mMsgCmdStr[SpiChannel::MOSI],
											mMsgExtStr[SpiChannel::MOSI]);
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

void SpiAnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/) //unreferenced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SpiAnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/) //unreferenced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}

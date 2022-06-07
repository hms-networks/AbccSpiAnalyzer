/******************************************************************************
**  Copyright (C) 2015-2022 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerResults.cpp
**    Summary: Handles various tasks for converting frame and packet data into
**             bubble-text, tabular-text, and exported CSV files.
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

#define IS_MISO_FRAME(frame)  (!frame.HasFlag(SPI_MOSI_FLAG))
#define IS_MOSI_FRAME(frame)  (frame.HasFlag(SPI_MOSI_FLAG))

#define MOSI_TAG_STR          "MOSI-"
#define MISO_TAG_STR          "MISO-"

#define MOSI_STR              "MOSI"
#define MISO_STR              "MISO"

#define FIRST_FRAG_STR        "FIRST_FRAGMENT"
#define FRAGMENT_STR          "FRAGMENT"
#define LAST_FRAG_STR         "LAST_FRAGMENT"

#define ERROR_RESPONSE_STR    " (ERR_RSP)"
#define RESPONSE_STR          " (RSP)"
#define COMMAND_STR           " (CMD)"

#define CSV_DELIMITER         mSettings->mExportDelimiter

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

void SpiAnalyzerResults::WriteBubbleText(const char* tag, const char* value, const char* verbose, NotifEvent_t notification, DisplayPriority disp_priority)
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
			memset(pad, ' ', (size_t)((strLenValue - strLenVerbose) >> 1) + 1);
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

void SpiAnalyzerResults::WriteTabularText(SpiChannel_t channel, const char* text, NotifEvent_t notification)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	const char *prefix;
	const char mosiPrefix[] = MOSI_TAG_STR;
	const char misoPrefix[] = MISO_TAG_STR;
	const char noPrefix[] = "";

	if (channel == SpiChannel::MOSI)
	{
		prefix = mosiPrefix;
	}
	else if (channel == SpiChannel::MISO)
	{
		prefix = misoPrefix;
	}
	else
	{
		prefix = noPrefix;
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

void SpiAnalyzerResults::FormatTabularTextBuffer(char* buffer, size_t buffer_size, const char* tag, const char* text, NotifEvent_t notification)
{
	if (notification == NotifEvent::Alert)
	{
		SNPRINTF(buffer, buffer_size, "!%s: %s", tag, text);
	}
	else
	{
		SNPRINTF(buffer, buffer_size, "%s: %s", tag, text);
	}
}

bool SpiAnalyzerResults::BuildCmdString(U8 command, U8 obj, DisplayBase display_base)
{
	bool errorRspMsg;
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetCmdString(command, obj, verboseStr, sizeof(verboseStr), display_base);

	GetNumberString(command, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::Command), numberStr, sizeof(numberStr), BaseType::Numeric);

	if ((command & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
	{
		errorRspMsg = true;
		WriteBubbleText("ERR_RSP", numberStr, verboseStr, NotifEvent::Alert);
	}
	else
	{
		errorRspMsg = false;
		if ((command & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
		{
			WriteBubbleText("CMD", numberStr, verboseStr, notification);
		}
		else
		{
			WriteBubbleText("RSP", numberStr, verboseStr, notification);
		}
	}

	return errorRspMsg;
}

void SpiAnalyzerResults::BuildInstString(U8 nw_type_idx, U8 obj, U16 inst, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	bool instFound = true;

	instFound = GetInstString(nw_type_idx, obj, inst, verboseStr, sizeof(verboseStr), &notification, display_base);

	GetNumberString(inst, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::CommandExtension), numberStr, sizeof(numberStr), BaseType::Numeric);

	if (instFound)
	{
		WriteBubbleText(GET_MSG_FRAME_TAG(AbccMsgField::Instance), numberStr, verboseStr, notification);
	}
	else
	{
		WriteBubbleText(GET_MSG_FRAME_TAG(AbccMsgField::Instance), numberStr, nullptr, NotifEvent::None);
	}
}

void SpiAnalyzerResults::BuildAttrString(const MsgHeaderInfo_t* msg_header_ptr, U16 attr, AttributeAccessMode_t access_mode, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = NotifEvent::None;
	bool attrFound = true;

	attrFound = GetAttrString(msg_header_ptr->obj, msg_header_ptr->inst, attr, verboseStr, sizeof(verboseStr), access_mode, &notification, display_base);
	GetNumberString(attr, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::CommandExtension), numberStr, sizeof(numberStr), BaseType::Numeric);

	if (attrFound)
	{
		WriteBubbleText(GET_MSG_FRAME_TAG(AbccMsgField::CommandExtension), numberStr, verboseStr, notification);
	}
	else
	{
		WriteBubbleText(GET_MSG_FRAME_TAG(AbccMsgField::CommandExtension), numberStr, nullptr, NotifEvent::None);
	}
}

void SpiAnalyzerResults::BuildObjectString(U8 object_num, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetObjectString(object_num, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(object_num, display_base, GET_MSG_FRAME_BITSIZE(AbccMsgField::Object), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText(GET_MSG_FRAME_TAG(AbccMsgField::Object), numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::BuildSpiCtrlString(U8 spi_control, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetSpiCtrlString(spi_control, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(spi_control, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::SpiControl), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText(GET_MOSI_FRAME_TAG(AbccMosiStates::SpiControl), numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::BuildSpiStsString(U8 spi_status, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetSpiStsString(spi_status, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(spi_status, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::SpiStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText(GET_MISO_FRAME_TAG(AbccMisoStates::SpiStatus), numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 error_code, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetErrorRspString(error_code, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(error_code, display_base, SIZE_IN_BITS(error_code), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText("ERR_CODE", numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::BuildErrorRsp(bool nw_spec_err, U8 nw_type_idx, U8 obj, U8 error_code, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetErrorRspString(nw_spec_err, nw_type_idx, obj, error_code, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(error_code, display_base, SIZE_IN_BITS(error_code), numberStr, sizeof(numberStr), BaseType::Numeric);

	if (nw_spec_err)
	{
		WriteBubbleText("NW_ERR", numberStr, verboseStr, notification);
	}
	else
	{
		WriteBubbleText("OBJ_ERR", numberStr, verboseStr, notification);
	}
}

void SpiAnalyzerResults::BuildIntMask(U8 int_mask, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetIntMaskString(int_mask, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(int_mask, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::InterruptMask), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText(GET_MOSI_FRAME_TAG(AbccMosiStates::InterruptMask), numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::BuildAbccStatus(U8 abcc_status, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetAbccStatusString(abcc_status, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(abcc_status, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::AnybusStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText(GET_MISO_FRAME_TAG(AbccMisoStates::AnybusStatus), numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::BuildApplStatus(U8 appl_status, DisplayBase display_base)
{
	char verboseStr[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	NotifEvent_t notification = GetApplStsString(appl_status, verboseStr, sizeof(verboseStr), display_base);
	GetNumberString(appl_status, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::ApplicationStatus), numberStr, sizeof(numberStr), BaseType::Numeric);
	WriteBubbleText(GET_MOSI_FRAME_TAG(AbccMosiStates::ApplicationStatus), numberStr, verboseStr, notification);
}

void SpiAnalyzerResults::GenerateMessageBubbleText(Frame &frame, DisplayBase display_base)
{
	NotifEvent_t notification = NotifEvent::None;
	AbccSpiStatesUnion_t uState;
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char verboseStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	uState.eMiso = static_cast<AbccMisoStates::Enum>(frame.mType);

	switch (uState.eMiso)
	{
		case AbccMisoStates::MessageField_Size:
		{
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
			{
				SNPRINTF(verboseStr, sizeof(verboseStr), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
				notification = NotifEvent::Alert;
			}
			else
			{
				SNPRINTF(verboseStr, sizeof(verboseStr), "%d Bytes", (U16)frame.mData1);
			}

			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, verboseStr, notification);

			break;
		}

		case AbccMisoStates::MessageField_Reserved1:
		{
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.mData1 != 0)
			{
				notification = NotifEvent::Alert;
			}

			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);

			break;
		}

		case AbccMisoStates::MessageField_SourceId:
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification);
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
		{
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.mData1 != 0)
			{
				notification = NotifEvent::Alert;
			}

			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);

			break;
		}

		case AbccMisoStates::MessageField_CommandExtension:
		{
			U16 cmdExt = static_cast<U16>(frame.mData1);
			MsgHeaderInfo_t* psMsgHdr = reinterpret_cast<MsgHeaderInfo_t*>(&frame.mData2);

			if (IsNonIndexedAttributeCmd(psMsgHdr->cmd))
			{
				BuildAttrString(psMsgHdr, cmdExt, AttributeAccessMode::Normal, display_base);
			}
			else if (IsIndexedAttributeCmd(psMsgHdr->cmd))
			{
				BuildAttrString(psMsgHdr, cmdExt, AttributeAccessMode::Indexed, display_base);
			}
			else
			{
				const char* verboseStrPtr = verboseStr;

				GetNumberString(cmdExt, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
				SegmentationType segmentation = GetMessageSegmentationType(psMsgHdr);

				if (segmentation != SegmentationType::None)
				{
					bool commandMsg = IsCommandMessage(psMsgHdr);
					bool segmentationMsg = IsSegmentedMessage(commandMsg, segmentation);
					U8 cmdExt0 = (cmdExt >> 0) & 0xFF;
					U8 cmdExt1 = (cmdExt >> 8) & 0xFF;
					U8 validFlags = 0;

					if (commandMsg)
					{
						validFlags |= ABP_MSG_CMDEXT1_SEG_ABORT;
					}

					if (segmentationMsg)
					{
						validFlags |= ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST;
					}

					if (cmdExt1 & ~validFlags)
					{
						notification = NotifEvent::Alert;
						SNPRINTF(verboseStr, sizeof(verboseStr), "0x%02X | Segmentation Unknown (0x%02X)", cmdExt0, cmdExt1);
					}
					else if (cmdExt1 & ABP_MSG_CMDEXT1_SEG_ABORT)
					{
						notification = NotifEvent::Alert;
						SNPRINTF(verboseStr, sizeof(verboseStr), "0x%02X | Segmentation Aborted", cmdExt0);
					}
					else if (segmentationMsg)
					{
						cmdExt1 &= (ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST);

						switch (cmdExt1)
						{
						case 0:
							SNPRINTF(verboseStr, sizeof(verboseStr), "0x%02X | SEGMENT", cmdExt0);
							break;
						case ABP_MSG_CMDEXT1_SEG_FIRST:
							SNPRINTF(verboseStr, sizeof(verboseStr), "0x%02X | FIRST_SEGMENT", cmdExt0);
							break;
						case ABP_MSG_CMDEXT1_SEG_LAST:
							SNPRINTF(verboseStr, sizeof(verboseStr), "0x%02X | LAST_SEGMENT", cmdExt0);
							break;
						case (ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST):
						default:
							SNPRINTF(verboseStr, sizeof(verboseStr), "0x%02X | FIRST_SEGMENT | LAST_SEGMENT", cmdExt0);
							break;
						}
					}
					else
					{
						verboseStrPtr = nullptr;
					}
				}
				else
				{
					verboseStrPtr = nullptr;
				}

				WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, verboseStrPtr, notification);
			}

			break;
		}

		case AbccMisoStates::MessageField:
		case AbccMisoStates::MessageField_Data:
		{
			MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
			{
				/* The error response may be composed of one to three bytes.
				** This payload can represent a common error response,
				** an object specific error response, or a network specific
				** error response. */

				if (info->msgDataCnt == 0)
				{
					BuildErrorRsp((U8)frame.mData1, display_base);
				}
				else
				{
					const U16 nwSpecErrCodeOffset = 2;
					U8 nwTypeIdx = 0;
					bool nwSpecificError = (info->msgDataCnt == nwSpecErrCodeOffset);

					if (nwSpecificError)
					{
						nwTypeIdx = static_cast<U8>(mSettings->mNetworkType);
					}

					BuildErrorRsp(nwSpecificError, nwTypeIdx, info->msgHeader.obj, (U8)frame.mData1, display_base);
				}
			}
			else
			{
				BaseType type;
				bool exception = false;
				bool nwObject = (info->msgHeader.obj == ABP_OBJ_NUM_NW);
				bool attribute = IsAttributeCmd(info->msgHeader.cmd);
				bool firstAttributeByte = (attribute && (info->msgDataCnt == 0));

				if (attribute)
				{
					type = GetAttrBaseType(info->msgHeader.obj, info->msgHeader.inst, (U8)info->msgHeader.cmdExt);
				}
				else
				{
					type = GetCmdBaseType(info->msgHeader.obj, info->msgHeader.cmd);
				}

				if (IS_MISO_FRAME(frame) && firstAttributeByte)
				{
					U16 tableIndex;

					if (GetExceptionTableIndex(nwObject, (U8)mSettings->mNetworkType, &info->msgHeader, &tableIndex))
					{
						notification = GetExceptionString(nwObject, tableIndex, (U8)frame.mData1, verboseStr, sizeof(verboseStr), display_base);
						exception = true;
					}
				}

				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), type);

				if (exception)
				{
					if (nwObject)
					{
						WriteBubbleText("EXC_INFO", numberStr, verboseStr, notification);
					}
					else
					{
						WriteBubbleText("EXC_CODE", numberStr, verboseStr, notification);
					}
				}
				else
				{
					SNPRINTF(verboseStr, sizeof(verboseStr), " [%s] Byte #%d ", numberStr, info->msgDataCnt);

					if (mSettings->mMsgDataPriority == DisplayPriority::Value)
					{
						/* Conditionally trim the leading 0x specifier */
						U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
						WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), &numberStr[offset], verboseStr, notification, DisplayPriority::Value);
					}
					else
					{
						WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, verboseStr, notification);
					}
				}

			}

			break;
		}

		default:
			break;
	}
}

void SpiAnalyzerResults::GenerateMisoBubbleText(Frame &frame, DisplayBase display_base)
{
	NotifEvent_t notification = NotifEvent::None;
	AbccSpiStatesUnion_t uState;
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char verboseStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	uState.eMiso = static_cast<AbccMisoStates::Enum>(frame.mType);

	switch (uState.eMiso)
	{
		case AbccMisoStates::Idle:
			break;

		case AbccMisoStates::Reserved1:
		{
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.mData1 != 0)
			{
				notification = NotifEvent::Alert;
			}

			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);

			break;
		}

		case AbccMisoStates::Reserved2:
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", notification);
			break;

		case AbccMisoStates::LedStatus:
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
			notification = GetLedStatusString((U16)frame.mData1, verboseStr, sizeof(verboseStr), display_base);
			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, verboseStr, notification);
			break;

		case AbccMisoStates::AnybusStatus:
			BuildAbccStatus((U8)frame.mData1, display_base);
			break;

		case AbccMisoStates::SpiStatus:
			BuildSpiStsString((U8)frame.mData1, display_base);
			break;

		case AbccMisoStates::NetworkTime:
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification);
			break;

		case AbccMisoStates::MessageField_Size:
		case AbccMisoStates::MessageField_Reserved1:
		case AbccMisoStates::MessageField_SourceId:
		case AbccMisoStates::MessageField_Object:
		case AbccMisoStates::MessageField_Instance:
		case AbccMisoStates::MessageField_Command:
		case AbccMisoStates::MessageField_Reserved2:
		case AbccMisoStates::MessageField_CommandExtension:
		case AbccMisoStates::MessageField:
		case AbccMisoStates::MessageField_Data:
			GenerateMessageBubbleText(frame, display_base);
			break;

		case AbccMisoStates::MessageField_DataNotValid:
			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(AbccMisoStates::MessageField_Data), numberStr, sizeof(numberStr));
			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, nullptr, notification, DisplayPriority::Tag);
			break;

		case AbccMisoStates::ReadProcessData:
		{
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);
			SNPRINTF(verboseStr, sizeof(verboseStr), " [%s] Byte #%lld ", numberStr, frame.mData2);

			if (mSettings->mProcessDataPriority == DisplayPriority::Value)
			{
				/* Conditionally trim the leading 0x specifier */
				U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
				WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), &numberStr[offset], verboseStr, notification, DisplayPriority::Value);
			}
			else
			{
				WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, verboseStr, notification);
			}

			break;
		}

		case AbccMisoStates::Crc32:
		{
			GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG) && frame.HasFlag(DISPLAY_AS_ERROR_FLAG))
			{
				SNPRINTF(verboseStr, sizeof(verboseStr), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1), (U32)(frame.mData2));
				notification = NotifEvent::Alert;
			}
			else
			{
				SNPRINTF(verboseStr, sizeof(verboseStr), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1), (U32)(frame.mData2));
			}

			WriteBubbleText(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, verboseStr, notification);
			break;
		}

		default:
			GetNumberString(frame.mData1, display_base, 8, numberStr, sizeof(numberStr), BaseType::Numeric);
			notification = NotifEvent::Alert;
			WriteBubbleText("UNKNOWN", numberStr, "Internal Error: Unknown State", notification);
			break;
	}
}

void SpiAnalyzerResults::GenerateMosiBubbleText(Frame &frame, DisplayBase display_base)
{
	NotifEvent_t notification = NotifEvent::None;
	AbccSpiStatesUnion_t uState;
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char verboseStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	uState.eMosi = (AbccMosiStates::Enum)frame.mType;

	switch (uState.eMosi)
	{
		case AbccMosiStates::Idle:
			break;

		case AbccMosiStates::SpiControl:
			BuildSpiCtrlString((U8)frame.mData1, display_base);
			break;

		case AbccMosiStates::Reserved1:
		{
			GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.mData1 != 0)
			{
				notification = NotifEvent::Alert;
			}

			WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", notification);

			break;
		}

		case AbccMosiStates::MessageLength:
			GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
			SNPRINTF(verboseStr, sizeof(verboseStr), "%d Words", (U16)frame.mData1);
			WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, verboseStr, notification);
			break;

		case AbccMosiStates::ProcessDataLength:
			GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
			SNPRINTF(verboseStr, sizeof(verboseStr), "%d Words", (U16)frame.mData1);
			WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, verboseStr, notification);
			break;

		case AbccMosiStates::ApplicationStatus:
			BuildApplStatus((U8)frame.mData1, display_base);
			break;

		case AbccMosiStates::InterruptMask:
			BuildIntMask((U8)frame.mData1, display_base);
			break;

		case AbccMosiStates::MessageField_Size:
		case AbccMosiStates::MessageField_Reserved1:
		case AbccMosiStates::MessageField_SourceId:
		case AbccMosiStates::MessageField_Object:
		case AbccMosiStates::MessageField_Instance:
		case AbccMosiStates::MessageField_Command:
		case AbccMosiStates::MessageField_Reserved2:
		case AbccMosiStates::MessageField_CommandExtension:
		case AbccMosiStates::MessageField:
		case AbccMosiStates::MessageField_Data:
			GenerateMessageBubbleText(frame, display_base);
			break;

		case AbccMosiStates::MessageField_DataNotValid:
			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(AbccMosiStates::MessageField_Data), numberStr, sizeof(numberStr));
			WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, nullptr, notification, DisplayPriority::Tag);
			break;

		case AbccMosiStates::WriteProcessData:
		{
			GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);
			SNPRINTF(verboseStr, sizeof(verboseStr), " [%s] Byte #%lld ", numberStr, frame.mData2);

			if (mSettings->mProcessDataPriority == DisplayPriority::Value)
			{
				/* Conditionally trim the leading 0x specifier */
				U8 offset = (display_base == DisplayBase::Hexadecimal) ? 2 : 0;
				WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), &numberStr[offset], verboseStr, notification, DisplayPriority::Value);
			}
			else
			{
				WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, verboseStr, notification);
			}

			break;
		}

		case AbccMosiStates::Crc32:
		{
			GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG) && frame.HasFlag(DISPLAY_AS_ERROR_FLAG))
			{
				SNPRINTF(verboseStr, sizeof(verboseStr), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
				notification = NotifEvent::Alert;
			}
			else
			{
				SNPRINTF(verboseStr, sizeof(verboseStr), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
			}

			WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, verboseStr, notification);

			break;
		}

		case AbccMosiStates::Pad:
		{
			GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr), BaseType::Numeric);

			if (frame.mData1 != 0)
			{
				notification = NotifEvent::Alert;
			}

			WriteBubbleText(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, nullptr, notification);

			break;
		}

		default:
			GetNumberString(frame.mData1, display_base, 8, numberStr, sizeof(numberStr), BaseType::Numeric);
			notification = NotifEvent::Alert;
			WriteBubbleText("UNKNOWN", numberStr, "Internal Error: Unknown State", notification);
			break;
	}
}

void SpiAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel &channel, DisplayBase display_base)
{
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);
	AbccSpiStatesUnion_t uState;
	uState.eMosi = (AbccMosiStates::Enum)frame.mType;

	if (frame.HasFlag(SPI_ERROR_FLAG))
	{
		NotifEvent_t notification = NotifEvent::Alert;

		switch (frame.mType)
		{
			case AbccSpiError::Fragmentation:
			{
				if ((IS_MISO_FRAME(frame) && (channel == mSettings->mMisoChannel)) ||
					(IS_MOSI_FRAME(frame) && (channel == mSettings->mMosiChannel)))
				{
					WriteBubbleText("FRAGMENT", nullptr, "Fragmented ABCC SPI Packet.", notification);
				}

				break;
			}

			case AbccSpiError::EndOfTransfer:
				WriteBubbleText("CLOCKING", nullptr, "ABCC SPI Clocking. The analyzer expects one transaction per 'Active Enable' phase.", notification);
				break;

			case AbccSpiError::Generic:
			default:
				WriteBubbleText("ERROR", nullptr, "ABCC SPI Error.", notification);
				break;
		}
	}
	else
	{
		if ((channel == mSettings->mMosiChannel) && IS_MOSI_FRAME(frame))
		{
			GenerateMosiBubbleText(frame, display_base);
		}
		else if ((channel == mSettings->mMisoChannel) && IS_MISO_FRAME(frame))
		{
			GenerateMisoBubbleText(frame, display_base);
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

		if (frame.HasFlag(SPI_ERROR_FLAG))
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

		if (frame.HasFlag(SPI_ERROR_FLAG))
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

void SpiAnalyzerResults::AppendCsvHeaderDelimeters(std::stringstream &ss_csv_data, U8 count, bool& add_header_delims)
{
	if (add_header_delims)
	{
		for (U8 i = 0U; i < count; i++)
		{
			ss_csv_data << CSV_DELIMITER;
		}

		add_header_delims = false;
	}
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

void SpiAnalyzerResults::AppendCsvSafeString(std::stringstream &ss_csv_data, char* input_data_str, DisplayBase display_base)
{
	const char doubleQuote[] = "\"";
	std::string csvStr;
	bool isStringData = (
		(display_base == DisplayBase::ASCII) ||
		(display_base == DisplayBase::AsciiHex));

	if (!isStringData)
	{
		ss_csv_data << CSV_DELIMITER << input_data_str;
		return;
	}

	csvStr.assign(input_data_str);

	ss_csv_data << CSV_DELIMITER;

	// The first two cases below are to reverse the Saleae Logic SDK's handling
	// of the 'space' and 'comma' ASCII characters. In this case the plugin will
	// escape the characters only if necessary based on the state of the
	// CSV delimiter.

	if (csvStr.find("COMMA") != std::string::npos)
	{
		std::string comma = ",";

		if (CSV_DELIMITER.compare(comma) == 0)
		{
			/* Replace with comma-character and surround with double quotes */
			ss_csv_data << doubleQuote << comma << doubleQuote;
		}
		else
		{
			/* Replace with space-character */
			ss_csv_data << comma;
		}
	}
	else if (csvStr.find("' '") != std::string::npos)
	{
		std::string space = " ";

		if (CSV_DELIMITER.compare(space) == 0)
		{
			/* Replace with space-character and surround with double quotes */
			ss_csv_data << doubleQuote << space << doubleQuote;
		}
		else
		{
			/* Replace with space-character */
			ss_csv_data << space;
		}
	}
	else
	{
		if (csvStr.find('"') != std::string::npos)
		{
			/* Escape double-quotes */
			for (std::string::size_type n = 0;
				(n = csvStr.find(doubleQuote, n)) != std::string::npos;
				n += 2)
			{
				csvStr.replace(n, 1, "\"\"");
			}

			/* Wrap element in double quotes */
			csvStr.insert(0, doubleQuote);
			csvStr.append(doubleQuote);
		}

		if (csvStr.find(CSV_DELIMITER) != std::string::npos)
		{
			/* Wrap element in double quotes */
			csvStr.insert(0, doubleQuote);
			csvStr.append(doubleQuote);
		}

		/* No escaping needed */
		ss_csv_data << csvStr;
	}
}

void SpiAnalyzerResults::BufferCsvMessageMsgEntry(
	Frame& frame,
	std::stringstream& ss_csv_data,
	bool& align_msg_fields,
	DisplayBase display_base)
{
	char dataStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

	// NOTE: AbccMosiStates and AbccMisoStates are assumed to have aligned
	// values for AbccMosiStates::MessageField* and AbccMisoStates::MessageField*.
	// This means only one of the enums will be used for the cases defined below.

	switch (frame.mType)
	{
	case AbccMisoStates::MessageField_Size:
	{
		GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
		ss_csv_data << CSV_DELIMITER << dataStr;
		align_msg_fields = false;
		break;
	}

	case AbccMisoStates::MessageField_SourceId:
	{
		GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
		AppendCsvHeaderDelimeters(ss_csv_data, 1, align_msg_fields);
		ss_csv_data << CSV_DELIMITER << dataStr;
		break;
	}

	case AbccMisoStates::MessageField_Object:
	{
		GetObjectString((U8)frame.mData1, dataStr, sizeof(dataStr), display_base);
		AppendCsvHeaderDelimeters(ss_csv_data, 2, align_msg_fields);
		ss_csv_data << CSV_DELIMITER << dataStr;
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

		AppendCsvHeaderDelimeters(ss_csv_data, 3, align_msg_fields);
		ss_csv_data << CSV_DELIMITER << dataStr;

		break;
	}

	case AbccMisoStates::MessageField_Command:
	{
		GetCmdString((U8)frame.mData1, (U8)frame.mData2, dataStr, sizeof(dataStr), display_base);
		AppendCsvHeaderDelimeters(ss_csv_data, 4, align_msg_fields);

		if (((U8)frame.mData1 & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			ss_csv_data << CSV_DELIMITER << dataStr << ERROR_RESPONSE_STR;
		}
		else
		{
			if (((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
			{
				ss_csv_data << CSV_DELIMITER << dataStr << COMMAND_STR;
			}
			else
			{
				ss_csv_data << CSV_DELIMITER << dataStr << RESPONSE_STR;
			}
		}

		break;
	}

	case AbccMisoStates::MessageField_CommandExtension:
	{
		NotifEvent_t notification = NotifEvent::None;
		DisplayBase displayBase = DisplayBase::Hexadecimal;
		U16 cmdExt = static_cast<U16>(frame.mData1);
		MsgHeaderInfo_t* psMsgHdr = reinterpret_cast<MsgHeaderInfo_t*>(&frame.mData2);

		if (IsNonIndexedAttributeCmd(psMsgHdr->cmd))
		{
			GetAttrString(psMsgHdr->obj, psMsgHdr->inst, cmdExt, dataStr, sizeof(dataStr), AttributeAccessMode::Normal, &notification, displayBase);
		}
		else if (IsIndexedAttributeCmd(psMsgHdr->cmd))
		{
			GetAttrString(psMsgHdr->obj, psMsgHdr->inst, cmdExt, dataStr, sizeof(dataStr), AttributeAccessMode::Indexed, &notification, displayBase);
		}
		else
		{
			SegmentationType segmentation = GetMessageSegmentationType(psMsgHdr);

			if (segmentation != SegmentationType::None)
			{
				bool commandMsg = IsCommandMessage(psMsgHdr);
				bool segmentationMsg = IsSegmentedMessage(commandMsg, segmentation);
				U8 cmdExt0 = (cmdExt >> 0) & 0xFF;
				U8 cmdExt1 = (cmdExt >> 8) & 0xFF;
				U8 validFlags = 0;

				notification = NotifEvent::None;

				if (commandMsg)
				{
					validFlags |= ABP_MSG_CMDEXT1_SEG_ABORT;
				}

				if (segmentationMsg)
				{
					validFlags |= ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST;
				}

				if (cmdExt1 & ~validFlags)
				{
					notification = NotifEvent::Alert;
					SNPRINTF(dataStr, sizeof(dataStr), "0x%02X | Segmentation Unknown (0x%02X)", cmdExt0, cmdExt1);
				}
				else if (cmdExt1 & ABP_MSG_CMDEXT1_SEG_ABORT)
				{
					notification = NotifEvent::Alert;
					SNPRINTF(dataStr, sizeof(dataStr), "0x%02X | Segmentation Aborted", cmdExt0);
				}
				else if (segmentationMsg)
				{
					cmdExt1 &= (ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST);

					switch (cmdExt1)
					{
					case 0:
						SNPRINTF(dataStr, sizeof(dataStr), "0x%02X | SEGMENT", cmdExt0);
						break;
					case ABP_MSG_CMDEXT1_SEG_FIRST:
						SNPRINTF(dataStr, sizeof(dataStr), "0x%02X | FIRST_SEGMENT", cmdExt0);
						break;
					case ABP_MSG_CMDEXT1_SEG_LAST:
						SNPRINTF(dataStr, sizeof(dataStr), "0x%02X | LAST_SEGMENT", cmdExt0);
						break;
					case (ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST):
					default:
						SNPRINTF(dataStr, sizeof(dataStr), "0x%02X | FIRST_SEGMENT | LAST_SEGMENT", cmdExt0);
						break;
					}
				}
				else
				{
					// Message relates to segmentation but is either a command for
					// "segmented response" or a response to a "segmented command".
					GetNumberString(cmdExt, displayBase, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
				}
			}
			else
			{
				GetNumberString(cmdExt, displayBase, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), BaseType::Numeric);
			}
		}

		AppendCsvHeaderDelimeters(ss_csv_data, 5, align_msg_fields);
		ss_csv_data << CSV_DELIMITER << dataStr;

		break;
	}

	case AbccMisoStates::MessageField_Data:
	case AbccMisoStates::MessageField:
	{
		MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

		if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
		{
			const U16 nwSpecErrCodeOffset = 2;

			/* The error response may be composed of one to three bytes.
			** This payload can represent a common error response,
			** an object specific error response, or a network specific
			** error response. */

			if (info->msgDataCnt == 0)
			{
				GetErrorRspString((U8)frame.mData1, dataStr, sizeof(dataStr), display_base);
			}
			else if (info->msgDataCnt <= nwSpecErrCodeOffset)
			{
				U8 nwTypeIdx = 0;
				bool nwSpecificError = (info->msgDataCnt == nwSpecErrCodeOffset);

				if (nwSpecificError)
				{
					nwTypeIdx = static_cast<U8>(mSettings->mNetworkType);
				}

				GetErrorRspString(nwSpecificError, nwTypeIdx, info->msgHeader.obj, (U8)frame.mData1, dataStr, sizeof(dataStr), display_base);

				// Explicitly set display_base to ASCII to ensure proper escaping
				// is performed when calling AppendCsvSafeString().
				display_base = DisplayBase::ASCII;
			}
		}
		else
		{
			BaseType type;
			bool exception = false;
			bool attribute = IsAttributeCmd(info->msgHeader.cmd);
			bool firstAttributeByte = (attribute && (info->msgDataCnt == 0));

			if (attribute)
			{
				type = GetAttrBaseType(info->msgHeader.obj, info->msgHeader.inst, (U8)info->msgHeader.cmdExt);
			}
			else
			{
				type = GetCmdBaseType(info->msgHeader.obj, info->msgHeader.cmd);
			}

			if (IS_MISO_FRAME(frame) && firstAttributeByte)
			{
				bool nwObject = (info->msgHeader.obj == ABP_OBJ_NUM_NW);
				U16 tableIndex;

				if (GetExceptionTableIndex(nwObject, (U8)mSettings->mNetworkType, &info->msgHeader, &tableIndex))
				{
					GetExceptionString(nwObject, tableIndex, (U8)frame.mData1, dataStr, sizeof(dataStr), display_base);

					// Explicitly set display_base to ASCII to ensure proper escaping
					// is performed when calling AppendCsvSafeString().
					display_base = DisplayBase::ASCII;
					exception = true;
				}
			}

			if (!exception)
			{
				GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr), type);
			}
		}

		AppendCsvHeaderDelimeters(ss_csv_data, 6, align_msg_fields);

		AppendCsvSafeString(ss_csv_data, dataStr, display_base);

		break;
	}

	default:
		break;
	}
}

void SpiAnalyzerResults::BufferCsvMessageMisoEntry(
	U32 sample_rate,
	U64 trigger_sample,
	U64 packet_id,
	Frame& frame,
	std::stringstream& ss_csv_head,
	std::stringstream& ss_csv_body,
	std::stringstream& ss_csv_tail,
	ErrorEvent& mosi_event,
	ErrorEvent& miso_event,
	bool& fragmentation,
	bool& anb_stat_reached,
	bool& align_msg_fields,
	bool& add_entry,
	DisplayBase display_base)
{
	switch (frame.mType)
	{
	case AbccSpiError::Fragmentation:
		miso_event = ErrorEvent::SpiFragmentationError;
		break;

	case AbccMisoStates::AnybusStatus:
	{
		char dataStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

		anb_stat_reached = true;
		GetAbccStatusString((U8)frame.mData1, dataStr, sizeof(dataStr), display_base);
		ss_csv_body << CSV_DELIMITER << dataStr;

		break;
	}

	case AbccMisoStates::SpiStatus:
	{
		bool message = ((frame.mData1 & ABP_SPI_STATUS_M) != 0);

		if (message)
		{
			char timeStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

			/* Add in the timestamp, packet ID, and Anybus state */
			AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
			ss_csv_head << std::endl
				<< MISO_STR + CSV_DELIMITER << timeStr << CSV_DELIMITER << packet_id;
			add_entry = true;
		}

		ss_csv_tail << CSV_DELIMITER;

		if (message)
		{
			if (frame.mData1 & ABP_SPI_STATUS_LAST_FRAG)
			{
				if (fragmentation)
				{
					ss_csv_tail << LAST_FRAG_STR;
					fragmentation = false;
				}
			}
			else
			{
				if (!fragmentation)
				{
					fragmentation = true;
					ss_csv_tail << FIRST_FRAG_STR;
				}
				else
				{
					ss_csv_tail << FRAGMENT_STR;
				}
			}
		}

		break;
	}

	case AbccMisoStates::MessageField_Size:
	case AbccMisoStates::MessageField_SourceId:
	case AbccMisoStates::MessageField_Object:
	case AbccMisoStates::MessageField_Instance:
	case AbccMisoStates::MessageField_Command:
	case AbccMisoStates::MessageField_CommandExtension:
	case AbccMisoStates::MessageField_Data:
	case AbccMisoStates::MessageField:
		BufferCsvMessageMsgEntry(
			frame,
			ss_csv_tail,
			align_msg_fields,
			display_base);

		break;

	case AbccMisoStates::Crc32:
	{
		if ((U32)frame.mData1 != (U32)frame.mData2)
		{
			miso_event = ErrorEvent::CrcError;
			mosi_event = ErrorEvent::CrcError;
		}

		break;
	}

	default:
		break;
	}
}

void SpiAnalyzerResults::BufferCsvMessageMosiEntry(
	U32 sample_rate,
	U64 trigger_sample,
	U64 packet_id,
	Frame &frame,
	std::stringstream &ss_csv_head,
	std::stringstream &ss_csv_body,
	std::stringstream &ss_csv_tail,
	ErrorEvent &mosi_event,
	ErrorEvent &miso_event,
	bool &fragmentation,
	bool &app_stat_reached,
	bool &align_msg_fields,
	bool &add_entry,
	DisplayBase display_base)
{
	switch (frame.mType)
	{
		case AbccSpiError::Fragmentation:
			mosi_event = ErrorEvent::SpiFragmentationError;
			break;

		case AbccMosiStates::SpiControl:
		{
			char timeStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
			bool message = ((frame.mData1 & ABP_SPI_CTRL_M) != 0);

			if (message)
			{
				/* Add in the timestamp, packet ID */
				AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
				ss_csv_head << std::endl
							<< MOSI_STR + CSV_DELIMITER << timeStr << CSV_DELIMITER << packet_id;
				add_entry = true;
			}

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
			{
				mosi_event = ErrorEvent::RetransmitWarning;
				miso_event = ErrorEvent::RetransmitWarning;
			}

			ss_csv_tail << CSV_DELIMITER;

			if (message)
			{
				if (frame.mData1 & ABP_SPI_CTRL_LAST_FRAG)
				{
					if (fragmentation)
					{
						ss_csv_tail << LAST_FRAG_STR;
						fragmentation = false;
					}
				}
				else
				{
					if (!fragmentation)
					{
						fragmentation = true;
						ss_csv_tail << FIRST_FRAG_STR;
					}
					else
					{
						ss_csv_tail << FRAGMENT_STR;
					}
				}
			}

			break;
		}

		case AbccMosiStates::ApplicationStatus:
		{
			char dataStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];

			app_stat_reached = true;
			GetApplStsString((U8)frame.mData1, dataStr, sizeof(dataStr), display_base);
			ss_csv_body << CSV_DELIMITER << dataStr;

			break;
		}

		case AbccMosiStates::MessageField_Size:
		case AbccMosiStates::MessageField_SourceId:
		case AbccMosiStates::MessageField_Object:
		case AbccMosiStates::MessageField_Instance:
		case AbccMosiStates::MessageField_Command:
		case AbccMosiStates::MessageField_CommandExtension:
		case AbccMosiStates::MessageField_Data:
		case AbccMosiStates::MessageField:
			BufferCsvMessageMsgEntry(
				frame,
				ss_csv_tail,
				align_msg_fields,
				display_base);

			break;

		case AbccMosiStates::Crc32:
		{
			if ((U32)frame.mData1 != (U32)frame.mData2)
			{
				mosi_event = ErrorEvent::CrcError;
			}

			break;
		}

		default:
			break;
	}
}

void SpiAnalyzerResults::ExportMessageDataToFile(const char *file, DisplayBase display_base)
{
	std::stringstream ssMosiHead;
	std::stringstream ssMisoHead;
	std::stringstream ssMisoTail;
	std::stringstream ssMosiTail;
	std::stringstream ssSharedBody;

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
			bool alignMosiMsgFields = true;
			bool alignMisoMsgFields = true;
			bool addMosiEntry = false;
			bool addMisoEntry = false;

			GetFramesContainedInPacket(packetId, &firstFrameId, &lastFrameId);

			/* Iterate through packet and extract message header and data
			** stream is written only on receipt of "last fragment". */
			for (U64 frameId = firstFrameId; frameId <= lastFrameId; frameId++)
			{
				Frame frame = GetFrame(frameId);

				if (IS_MOSI_FRAME(frame))
				{
					BufferCsvMessageMosiEntry(
						sampleRate,
						triggerSample,
						packetId,
						frame,
						ssMosiHead,
						ssSharedBody,
						ssMosiTail,
						mosiEvent,
						misoEvent,
						mosiFragmentation,
						mosiAppStatReached,
						alignMosiMsgFields,
						addMosiEntry,
						display_base);
				}
				else
				{
					 BufferCsvMessageMisoEntry(
						sampleRate,
						triggerSample,
						packetId,
						frame,
						ssMisoHead,
						ssSharedBody,
						ssMisoTail,
						mosiEvent,
						misoEvent,
						misoFragmentation,
						misoAnbStatReached,
						alignMisoMsgFields,
						addMisoEntry,
						display_base);
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
						GetApplStsString((U8)frame.mData1, dataStr, sizeof(dataStr), display_base);
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
						GetAbccStatusString((U8)frame.mData1, dataStr, sizeof(dataStr), display_base);
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
	default:
		break;
	}
}

U64 SpiAnalyzerResults::GetFrameIdOfAbccFieldContainedInPacket(U64 packet_index, SpiChannel_t channel, U8 type)
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
				if (((channel == SpiChannel::MOSI) && IS_MOSI_FRAME(frame) && (frame.mType == type)) ||
					((channel == SpiChannel::MISO) && IS_MISO_FRAME(frame) && (frame.mType == type)))
				{
					break;
				}
			}
		}
	}
	return frameIndex;
}

void SpiAnalyzerResults::GenerateMessageTabularText(SpiChannel_t channel, Frame &frame, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];

	// NOTE: AbccMosiStates and AbccMisoStates are assumed to have aligned
	// values for AbccMosiStates::MessageField* and AbccMisoStates::MessageField*.
	// This means only one of the enums will be used for the cases defined below.

	switch (frame.mType)
	{
		case AbccMisoStates::MessageField_Size:
		{
			bool sizeError = frame.HasFlag(SPI_PROTO_EVENT_FLAG);

			if (sizeError && mSettings->mErrorIndexing)
			{
				WriteTabularText(channel, "Message Size: Exceeds Maximum", NotifEvent::Alert);
			}
			else
			{
				if (sizeError)
				{
					SNPRINTF(mMsgSizeStr[channel], sizeof(mMsgSizeStr[channel]), "!Size: %u Bytes", (U16)frame.mData1);
				}
				else
				{
					SNPRINTF(mMsgSizeStr[channel], sizeof(mMsgSizeStr[channel]), "Size: %u Bytes", (U16)frame.mData1);
				}
			}

			break;
		}

		case AbccMisoStates::MessageField_SourceId:
		{
			SNPRINTF(mMsgSrcStr[channel], sizeof(mMsgSrcStr[channel]), "Source ID: %d (0x%02X)", (U8)frame.mData1, (U8)frame.mData1);
			break;
		}

		case AbccMisoStates::MessageField_Object:
		{
			if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
			{
				NotifEvent_t notification = GetObjectString((U8)frame.mData1, str, sizeof(str), display_base);

				if (notification == NotifEvent::Alert)
				{
					SNPRINTF(mMsgObjStr[channel], sizeof(mMsgObjStr[channel]), "!Object: %s", str);
				}
				else
				{
					SNPRINTF(mMsgObjStr[channel], sizeof(mMsgObjStr[channel]), "Object: %s", str);
				}
			}
			else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
			{
				SNPRINTF(mMsgObjStr[channel], sizeof(mMsgObjStr[channel]), "Obj {%02X:", (U8)frame.mData1);
			}

			break;
		}

		case AbccMisoStates::MessageField_Instance:
		{
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
					SNPRINTF(mMsgInstStr[channel], sizeof(mMsgInstStr[channel]), "!Instance: %s", str);
				}
				else
				{
					SNPRINTF(mMsgInstStr[channel], sizeof(mMsgInstStr[channel]), "Instance: %s", str);
				}
			}
			else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
			{
				SNPRINTF(mMsgInstStr[channel], sizeof(mMsgInstStr[channel]), "%04Xh}", (U16)frame.mData1);
			}

			break;
		}

		case AbccMisoStates::MessageField_Command:
		{
			mMsgErrorRspFlag[channel] = ((frame.mData1 & ABP_MSG_HEADER_E_BIT) != 0);

			if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
			{
				NotifEvent_t notification = GetCmdString((U8)frame.mData1, (U8)frame.mData2, str, sizeof(str), display_base);

				if ((mMsgErrorRspFlag[channel] == true) || (notification == NotifEvent::Alert))
				{
					if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
					{
						SNPRINTF(mMsgCmdStr[channel], sizeof(mMsgCmdStr[channel]), "!Command: %s", str);
					}
					else
					{
						SNPRINTF(mMsgCmdStr[channel], sizeof(mMsgCmdStr[channel]), "!Response: %s", str);
					}
				}
				else
				{
					if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
					{
						SNPRINTF(mMsgCmdStr[channel], sizeof(mMsgCmdStr[channel]), "Command: %s", str);
					}
					else
					{
						SNPRINTF(mMsgCmdStr[channel], sizeof(mMsgCmdStr[channel]), "Response: %s", str);
					}
				}
			}
			else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
			{
				if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
				{
					SNPRINTF(mMsgCmdStr[channel], sizeof(mMsgCmdStr[channel]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
				}
				else
				{
					SNPRINTF(mMsgCmdStr[channel], sizeof(mMsgCmdStr[channel]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
				}
			}

			break;
		}

		case AbccMisoStates::MessageField_CommandExtension:
		{
			if (mMsgValidFlag[channel])
			{
				U16 cmdExt = static_cast<U16>(frame.mData1);

				if ((mSettings->mMessageSrcIdIndexing) ||
					(mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed))
				{
					const char misoMsgBanner[] = "-----MISO MESSAGE-----";
					const char mosiMsgBanner[] = "-----MOSI MESSAGE-----";
					const char* msgBanner = (channel == SpiChannel::MISO) ? misoMsgBanner : mosiMsgBanner;

					AddTabularText(msgBanner);
				}

				if (mSettings->mMessageSrcIdIndexing)
				{
					WriteTabularText(channel, mMsgSrcStr[channel], NotifEvent::None);
				}

				if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Detailed)
				{
					NotifEvent_t notification = NotifEvent::None;
					MsgHeaderInfo_t* psMsgHdr = reinterpret_cast<MsgHeaderInfo_t*>(&frame.mData2);
					bool found = false;

					if (IsIndexedAttributeCmd(psMsgHdr->cmd))
					{
						found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, cmdExt, str, sizeof(str), AttributeAccessMode::Indexed, &notification, display_base);
					}
					else if (IsNonIndexedAttributeCmd(psMsgHdr->cmd))
					{
						found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, cmdExt, str, sizeof(str), AttributeAccessMode::Normal, &notification, display_base);
					}

					if (!found)
					{
						/* For consistency with Source ID and Instance, only use hex format */
						SNPRINTF(str, sizeof(str), "%d (0x%04X)", cmdExt, cmdExt);
					}

					if (notification == NotifEvent::Alert)
					{
						SNPRINTF(mMsgExtStr[channel], sizeof(mMsgExtStr[channel]), "!Extension: %s", str);
					}
					else
					{
						SNPRINTF(mMsgExtStr[channel], sizeof(mMsgExtStr[channel]), "Extension: %s", str);
					}

					WriteTabularText(channel, mMsgSizeStr[channel], NotifEvent::None);
					WriteTabularText(channel, mMsgObjStr[channel], NotifEvent::None);
					WriteTabularText(channel, mMsgInstStr[channel], NotifEvent::None);
					WriteTabularText(channel, mMsgCmdStr[channel], NotifEvent::None);
					WriteTabularText(channel, mMsgExtStr[channel], NotifEvent::None);

					SegmentationType segmentation = GetMessageSegmentationType(psMsgHdr);

					if (segmentation != SegmentationType::None)
					{
						bool addSegment = true;
						bool commandMsg = IsCommandMessage(psMsgHdr);
						bool segmentationMsg = IsSegmentedMessage(commandMsg, segmentation);
						U8 cmdExt1 = (cmdExt >> 8) & 0xFF;
						U8 validFlags = 0;

						notification = NotifEvent::None;

						if (commandMsg)
						{
							validFlags |= ABP_MSG_CMDEXT1_SEG_ABORT;
						}

						if (segmentationMsg)
						{
							validFlags |= ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST;
						}

						if (cmdExt1 & ~validFlags)
						{
							notification = NotifEvent::Alert;
							SNPRINTF(str, sizeof(str), "Segmentation Unknown (0x%02X).", cmdExt1);
						}
						else if (cmdExt1 & ABP_MSG_CMDEXT1_SEG_ABORT)
						{
							notification = NotifEvent::Alert;
							SNPRINTF(str, sizeof(str), "%s", "Segmentation Aborted.");
						}
						else if (segmentationMsg)
						{
							cmdExt1 &= (ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST);

							switch (cmdExt1)
							{
							case 0:
								SNPRINTF(str, sizeof(str), "%s", "Segment; More Follow.");
								break;
							case ABP_MSG_CMDEXT1_SEG_FIRST:
								SNPRINTF(str, sizeof(str), "%s", "First Segment; More Follow.");
								break;
							case ABP_MSG_CMDEXT1_SEG_LAST:
								SNPRINTF(str, sizeof(str), "%s", "Last Segment.");
								break;
							case (ABP_MSG_CMDEXT1_SEG_FIRST | ABP_MSG_CMDEXT1_SEG_LAST):
							default:
								addSegment = false;
								break;
							}
						}
						else
						{
							addSegment = false;
						}

						if (addSegment)
						{
							WriteTabularText(channel, str, notification);
						}
					}

					if (frame.HasFlag(SPI_MSG_FIRST_FRAG_FLAG))
					{
						WriteTabularText(channel, "First Fragment; More Follow.", NotifEvent::None);
					}
				}
				else if (mSettings->mMessageIndexingVerbosityLevel == MessageIndexing::Compact)
				{
					const char misoTag[] = MISO_TAG_STR;
					const char mosiTag[] = MOSI_TAG_STR;
					const char* tag = (channel == SpiChannel::MISO) ? misoTag : mosiTag;

					SNPRINTF(mMsgExtStr[channel], sizeof(mMsgExtStr[channel]), "%04Xh}", cmdExt);

					if (mMsgErrorRspFlag[channel])
					{
						AddTabularText(tag, "!",
							mMsgObjStr[channel],
							mMsgInstStr[channel],
							mMsgCmdStr[channel],
							mMsgExtStr[channel]);
					}
					else
					{
						if (frame.HasFlag(SPI_MSG_FIRST_FRAG_FLAG))
						{
							AddTabularText(tag,
								mMsgObjStr[channel],
								mMsgInstStr[channel],
								mMsgCmdStr[channel],
								mMsgExtStr[channel],
								"++");
						}
						else
						{
							AddTabularText(tag,
								mMsgObjStr[channel],
								mMsgInstStr[channel],
								mMsgCmdStr[channel],
								mMsgExtStr[channel]);
						}
					}
				}

				return;
			}

			break;
		}

		case AbccMisoStates::MessageField_Data:
		{
			NotifEvent_t notification = NotifEvent::None;
			MsgDataFrameData2_t* info = (MsgDataFrameData2_t*)&frame.mData2;

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
			{
				const U16 nwSpecErrCodeOffset = 2;
				char errorStr[FORMATTED_STRING_BUFFER_SIZE];

				// The error response may be composed of one to three bytes.
				// This payload can represent a common error response,
				// an object specific error response, or a network specific
				// error response.

				if (info->msgDataCnt == 0)
				{
					notification = GetErrorRspString((U8)frame.mData1, str, sizeof(str), display_base);
					SNPRINTF(errorStr, sizeof(errorStr), "Error Code: %s", str);
					WriteTabularText(channel, errorStr, notification);
				}
				else if (info->msgDataCnt <= nwSpecErrCodeOffset)
				{
					U8 nwTypeIdx = 0;
					bool nwSpecificError = (info->msgDataCnt == nwSpecErrCodeOffset);

					if (nwSpecificError)
					{
						nwTypeIdx = static_cast<U8>(mSettings->mNetworkType);
					}

					notification = GetErrorRspString(nwSpecificError, nwTypeIdx, info->msgHeader.obj, (U8)frame.mData1, str, sizeof(str), display_base);

					if (nwSpecificError)
					{
						SNPRINTF(errorStr, sizeof(errorStr), "Network Error: %s", str);
					}
					else
					{
						SNPRINTF(errorStr, sizeof(errorStr), "Object Error: %s", str);
					}

					WriteTabularText(channel, errorStr, notification);
				}
			}
			else if (channel == SpiChannel::MISO)
			{
				bool nwObject = (info->msgHeader.obj == ABP_OBJ_NUM_NW);
				bool exception = false;
				bool attribute = IsAttributeCmd(info->msgHeader.cmd);
				bool firstAttributeByte = (attribute && (info->msgDataCnt == 0));

				if (firstAttributeByte)
				{
					U16 tableIndex;

					if (GetExceptionTableIndex(nwObject, (U8)mSettings->mNetworkType, &info->msgHeader, &tableIndex))
					{
						notification = GetExceptionString(nwObject, tableIndex, (U8)frame.mData1, str, sizeof(str), display_base);
						exception = true;
					}
				}

				if (exception)
				{
					char excepStr[FORMATTED_STRING_BUFFER_SIZE];

					if (nwObject)
					{
						SNPRINTF(excepStr, sizeof(excepStr), "Exception Info: %s", str);
						WriteTabularText(channel, excepStr, notification);
					}
					else
					{
						SNPRINTF(excepStr, sizeof(excepStr), "Exception Code: %s", str);
						WriteTabularText(channel, excepStr, notification);
					}
				}
			}

			break;
		}

		default:
			break;
	}
}

void SpiAnalyzerResults::GenerateMosiTabularText(U64 frame_index, Frame &frame, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];

	switch (frame.mType)
	{
		case AbccMosiStates::ApplicationStatus:
		{
			bool statusChanged = frame.HasFlag(SPI_PROTO_EVENT_FLAG);

			if (statusChanged)
			{
				char appStsStr[FORMATTED_STRING_BUFFER_SIZE];
				NotifEvent_t notification = GetApplStsString((U8)frame.mData1, appStsStr, sizeof(appStsStr), display_base);
				bool addEntry = mSettings->mApplStatusIndexing || ((notification == NotifEvent::Alert) && mSettings->mErrorIndexing);

				if (addEntry)
				{
					SNPRINTF(str, sizeof(str), "Application Status: %s", appStsStr);
					WriteTabularText(SpiChannel::NotSpecified, str, notification);
				}
			}

			break;
		}

		case AbccMosiStates::SpiControl:
		{
			mMsgValidFlag[SpiChannel::MOSI] = ((frame.mData1 & ABP_SPI_CTRL_M) != 0);

			if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
			{
				if (frame_index != 0)
				{
					WriteTabularText(SpiChannel::MOSI, "{Message Retransmit}", NotifEvent::None);
					return;
				}
			}
			else if (frame.HasFlag(SPI_MSG_FRAG_FLAG) && !frame.HasFlag(SPI_MSG_FIRST_FRAG_FLAG))
			{
				/* Fragmentation is in progress */
				if (frame.mData1 & ABP_SPI_CTRL_M)
				{
					if (frame.mData1 & ABP_SPI_CTRL_LAST_FRAG)
					{
						/* Last fragment */
						WriteTabularText(SpiChannel::MOSI, "{Message Fragment}", NotifEvent::None);
					}
					else
					{
						/* More fragments follow */
						WriteTabularText(SpiChannel::MOSI, "{Message Fragment}++", NotifEvent::None);
					}
				}

				return;
			}

			break;
		}

		case AbccMosiStates::MessageField_Size:
		case AbccMosiStates::MessageField_SourceId:
		case AbccMosiStates::MessageField_Object:
		case AbccMosiStates::MessageField_Instance:
		case AbccMosiStates::MessageField_Command:
		case AbccMosiStates::MessageField_CommandExtension:
		case AbccMosiStates::MessageField_Data:
		{
			/* Since tabular text is sequentially processed and indexed,
			** buffer the "Object", "Instance", "Cmd", and "Ext";
			** then add as a single text entry. */
			if (mSettings->mMessageIndexingVerbosityLevel != MessageIndexing::Disabled)
			{
				GenerateMessageTabularText(SpiChannel::MOSI, frame, display_base);
			}

			break;
		}

		case AbccMosiStates::Crc32:
		{
			bool crcError = frame.HasFlag(SPI_PROTO_EVENT_FLAG);

			if (crcError && mSettings->mErrorIndexing)
			{
				WriteTabularText(SpiChannel::MOSI, "CRC32", NotifEvent::Alert);
			}

			break;
		}

		default:
			break;

	}

}

void SpiAnalyzerResults::GenerateMisoTabularText(U64 frame_index, Frame &frame, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];

	switch (frame.mType)
	{
		case AbccMisoStates::NetworkTime:
		{
			bool addEntry;

			switch (mSettings->mTimestampIndexing)
			{
				case TimestampIndexing::AllPackets:
					addEntry = true;
					break;

				case TimestampIndexing::WriteProcessDataValid:
					addEntry = ((NetworkTimeInfo_t*)&frame.mData2)->wrPdValid;
					break;

				case TimestampIndexing::NewReadProcessData:
					addEntry = ((NetworkTimeInfo_t*)&frame.mData2)->newRdPd;
					break;

				default:
				case TimestampIndexing::Disabled:
					addEntry = false;
					break;
			}

			if (addEntry)
			{
				U64 packetId = GetPacketContainingFrame(frame_index);
				U32 delta = ((NetworkTimeInfo_t*)&frame.mData2)->deltaTime;
				SNPRINTF(str, sizeof(str), "0x%08X (Delta: 0x%08X)", (U32)frame.mData1, delta);
				AddTabularText("Time: ", str);
				SNPRINTF(str, sizeof(str), "Packet: 0x%016llX", packetId);
				AddTabularText(str);
			}

			break;
		}

		case AbccMisoStates::AnybusStatus:
		{
			bool statusChanged = frame.HasFlag(SPI_PROTO_EVENT_FLAG);
			char anbSts[FORMATTED_STRING_BUFFER_SIZE];
			NotifEvent_t notification = GetAbccStatusString((U8)frame.mData1, anbSts, sizeof(anbSts), display_base);
			bool addEntry = statusChanged && (mSettings->mAnybusStatusIndexing || ((notification == NotifEvent::Alert) && mSettings->mErrorIndexing));

			if (addEntry)
			{
				SNPRINTF(str, sizeof(str), "Anybus Status: (%s)", anbSts);
				WriteTabularText(SpiChannel::MISO, str, notification);
			}

			break;
		}

		case AbccMisoStates::SpiStatus:
		{
			if (mSettings->mMessageIndexingVerbosityLevel != MessageIndexing::Disabled)
			{
				mMsgValidFlag[SpiChannel::MISO] = ((frame.mData1 & ABP_SPI_STATUS_M) != 0);

				if (frame.HasFlag(SPI_PROTO_EVENT_FLAG))
				{
					WriteTabularText(SpiChannel::MISO, "{Write Message Buffer Full}", NotifEvent::None);
				}
				else if (frame.HasFlag(SPI_MSG_FRAG_FLAG) && !frame.HasFlag(SPI_MSG_FIRST_FRAG_FLAG))
				{
					/* Fragmentation is in progress */
					if (frame.mData1 & ABP_SPI_STATUS_M)
					{
						if (frame.mData1 & ABP_SPI_STATUS_LAST_FRAG)
						{
							/* Last fragment */
							WriteTabularText(SpiChannel::MISO, "{Message Fragment}", NotifEvent::None);
						}
						else
						{
							/* More fragments follow */
							WriteTabularText(SpiChannel::MISO, "{Message Fragment}++", NotifEvent::None);
						}
					}
				}
			}

			break;
		}

		case AbccMisoStates::MessageField_Size:
		case AbccMisoStates::MessageField_SourceId:
		case AbccMisoStates::MessageField_Object:
		case AbccMisoStates::MessageField_Instance:
		case AbccMisoStates::MessageField_Command:
		case AbccMisoStates::MessageField_CommandExtension:
		case AbccMisoStates::MessageField_Data:
		{
			/* Since tabular text is sequentially processed and indexed,
			** buffer the "Object", "Instance", "Cmd", and "Ext";
			** then add as a single text entry. */
			if (mSettings->mMessageIndexingVerbosityLevel != MessageIndexing::Disabled)
			{
				GenerateMessageTabularText(SpiChannel::MISO, frame, display_base);
			}

			break;
		}

		case AbccMisoStates::Crc32:
		{
			bool crcError = frame.HasFlag(SPI_PROTO_EVENT_FLAG);

			if (crcError && mSettings->mErrorIndexing)
			{
				WriteTabularText(SpiChannel::MISO, "CRC32", NotifEvent::Alert);
			}

			break;
		}

		default:
			break;

	}
}

void SpiAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
	ClearTabularText();
	Frame frame = GetFrame(frame_index);

	if (mSettings->mErrorIndexing)
	{
		if (frame.HasFlag(SPI_ERROR_FLAG))
		{
			// These types of errors affect the bus as a whole and are not
			// specific to a MISO/MOSI channel. For compatibility, only
			// show one for tabular text.
			if (IS_MISO_FRAME(frame))
			{
				switch (frame.mType)
				{
					case AbccSpiError::Fragmentation:
						WriteTabularText(SpiChannel::NotSpecified, "FRAGMENT: ABCC SPI Packet is Fragmented", NotifEvent::Alert);
						break;
					case AbccSpiError::EndOfTransfer:
						WriteTabularText(SpiChannel::NotSpecified, "CLOCKING: Unexpected ABCC SPI Clocking Behavior", NotifEvent::Alert);
						break;
					case AbccSpiError::Generic:
					default:
						WriteTabularText(SpiChannel::NotSpecified, "ERROR: General Error in ABCC SPI Communication", NotifEvent::Alert);
						break;
				}
			}

			return;
		}
	}

	if (IS_MOSI_FRAME(frame))
	{
		GenerateMosiTabularText(frame_index, frame, display_base);
	}
	else
	{
		GenerateMisoTabularText(frame_index, frame, display_base);
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

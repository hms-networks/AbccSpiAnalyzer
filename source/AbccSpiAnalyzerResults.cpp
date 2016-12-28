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
#include "AbccSpiAnalyzerLookup.h"
#include <iostream>
#include <sstream>

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#define IS_MISO_FRAME(frame) 			((frame.mFlags & SPI_MOSI_FLAG)!=SPI_MOSI_FLAG)
#define IS_MOSI_FRAME(frame)			((frame.mFlags & SPI_MOSI_FLAG)==SPI_MOSI_FLAG)

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
					tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, false, display_base);
				}
				else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
				{
					tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, true, display_base);
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
					tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, false, display_base);
				}
				else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
					((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
				{
					tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
					BuildAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, true, display_base);
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
	U64 packet_id = GetPacketContainingFrame(frame_index);
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
			char str[FORMATTED_STRING_BUFFER_SIZE];
			switch(mSettings->mTimestampIndexing)
			{
			case e_TIMESTAMP_ALL_PACKETS:
				SNPRINTF(str, sizeof(str), "0x%08X (Delta : 0x%08X)", (U32)frame.mData1, (U32)frame.mData2);
				AddTabularText("Time : ", str);
				SNPRINTF(str, sizeof(str), "Packet : 0x%016llX", packet_id);
				AddTabularText(str);
				break;
			case e_TIMESTAMP_WRITE_PROCESS_DATA_VALID:
				if (((tNetworkTimeInfo*)&frame.mData2)->wrPdValid)
				{
					//TODO joca: adapt delta, to measure delta between "New Write Process Data"
					SNPRINTF(str, sizeof(str), "0x%08X (Delta : 0x%08X)", (U32)frame.mData1, ((tNetworkTimeInfo*)&frame.mData2)->deltaTime);
					//SNPRINTF(tmp, sizeof(tmp), "0x%08X (Delta : 0x%08X)\nPacket : 0x%016llX", (U32)frame.mData1, ((tNetworkTimeInfo*)&frame.mData2)->deltaTime, packet_index_lookup);
					AddTabularText("Time : ", str);
					SNPRINTF(str, sizeof(str), "Packet : 0x%016llX", packet_id);
					AddTabularText(str);
				}
				break;
			case e_TIMESTAMP_NEW_READ_PROCESS_DATA:
				if (((tNetworkTimeInfo*)&frame.mData2)->newRdPd)
				{
					//TODO joca: adapt delta, to measure delta between "New Write Process Data"
					SNPRINTF(str, sizeof(str), "0x%08X (Delta : 0x%08X)", (U32)frame.mData1, ((tNetworkTimeInfo*)&frame.mData2)->deltaTime);
					//SNPRINTF(str, sizeof(str), "0x%08X (Delta : 0x%08X)\nPacket : 0x%016llX", (U32)frame.mData1, ((tNetworkTimeInfo*)&frame.mData2)->deltaTime, packet_index_lookup);
					AddTabularText("Time : ", str);
					SNPRINTF(str, sizeof(str), "Packet : 0x%016llX", packet_id);
					AddTabularText(str);
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
								tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
								found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, str, sizeof(str), attrIdx, &alert, display_base);
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
								tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
								found = GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, str, sizeof(str), attrIdx, &alert, display_base);
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
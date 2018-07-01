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

#define CSV_DELIMITER			"\t"

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
	memset(acMsgSizeStr, 0, sizeof(acMsgSizeStr));
	memset(acMsgSrcStr, 0, sizeof(acMsgSrcStr));
	memset(acMsgObjStr, 0, sizeof(acMsgObjStr));
	memset(acMsgInstStr, 0, sizeof(acMsgInstStr));
	memset(acMsgCmdStr, 0, sizeof(acMsgCmdStr));
	memset(acMsgExtStr, 0, sizeof(acMsgExtStr));
	fMsgValid[0]    = false;
	fMsgValid[1]    = false;
	fMsgErrorRsp[0] = false;
	fMsgErrorRsp[1] = false;
}

SpiAnalyzerResults::~SpiAnalyzerResults()
{
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* verbose, bool alert)
{
	StringBuilder(tag, value, verbose, alert, false);
}

void SpiAnalyzerResults::StringBuilder(char* tag, char* value, char* verbose, bool alert, bool prioritize_value)
{
	const char alertStr[] = "!ALERT - ";
	U16 strLenValue, strLenVerbose;
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char pad[32] = "";
	bool applyPad = false;

	if (verbose && value)
	{
		strLenValue = (U16)strlen(value);
		if(alert)
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
		if (alert)
		{
			str[0] = '!';
		}
		else
		{
			str[0] = tag[0];
		}

		if (alert || !prioritize_value)
		{
			str[1] = '\0';
			AddResultString(str);
		}

		if (prioritize_value)
		{
			if (value)
			{
				if (alert)
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

			if (alert)
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
			if (alert)
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
				if (alert)
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
			if (alert)
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

void SpiAnalyzerResults::TableBuilder(bool is_mosi_channel, char* text, bool alert)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char* prefix;
	char mosiPrefix[] = MOSI_TAG_STR;
	char misoPrefix[] = MISO_TAG_STR;

	if(is_mosi_channel)
	{
		prefix = &mosiPrefix[0];
	}
	else
	{
		prefix = &misoPrefix[0];
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
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetCmdString(val, obj, &str[0], sizeof(str), display_base);

	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MSG_CMD), numberStr, sizeof(numberStr));

	if ((val & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
	{
		errorRspMsg = true;
		StringBuilder("ERR_RSP", numberStr, str, true);
	}
	else
	{
		errorRspMsg = false;
		if ((val & ABP_MSG_HEADER_C_BIT) == ABP_MSG_HEADER_C_BIT)
		{
			StringBuilder("CMD", numberStr, str, alert);
		}
		else
		{
			StringBuilder("RSP", numberStr, str, alert);
		}
	}

	return errorRspMsg;
}

void SpiAnalyzerResults::BuildInstString(U8 nw_type_idx, U8 obj, U16 val, DisplayBase display_base)
{
	char verbose_str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = false;
	bool objFound = true;

	objFound = GetInstString(nw_type_idx, obj, val, verbose_str, sizeof(verbose_str), &alert, display_base);

	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MSG_CMD_EXT), numberStr, sizeof(numberStr));

	if (objFound)
	{
		StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_INST), numberStr, verbose_str, alert);
	}
	else
	{
		StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_INST), numberStr, NULL, false);
	}
}

void SpiAnalyzerResults::BuildAttrString(U8 obj, U16 inst, U16 val, bool indexed, DisplayBase display_base)
{
	char verbose_str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = false;
	bool objFound = true;

	objFound = GetAttrString(obj, inst, val, verbose_str, sizeof(verbose_str), indexed, &alert, display_base);

	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MSG_CMD_EXT), numberStr, sizeof(numberStr));

	if (objFound)
	{
		StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_CMD_EXT), numberStr, verbose_str, alert);
	}
	else
	{
		StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_CMD_EXT), numberStr, NULL, false);
	}
}

void SpiAnalyzerResults::BuildObjectString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetObjectString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MSG_FRAME_BITSIZE(e_ABCC_MSG_OBJECT), numberStr, sizeof(numberStr));
	StringBuilder(GET_MSG_FRAME_TAG(e_ABCC_MSG_OBJECT), numberStr, str, alert);
}

void SpiAnalyzerResults::BuildSpiCtrlString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetSpiCtrlString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(e_ABCC_MOSI_SPI_CTRL), numberStr, sizeof(numberStr));
	StringBuilder(GET_MOSI_FRAME_TAG(e_ABCC_MOSI_SPI_CTRL), numberStr, str, alert);
}

void SpiAnalyzerResults::BuildSpiStsString(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetSpiStsString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MISO_SPI_STAT), numberStr, sizeof(numberStr));
	StringBuilder(GET_MISO_FRAME_TAG(e_ABCC_MISO_SPI_STAT), numberStr, str, alert);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetErrorRspString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr));
	StringBuilder("ERR_CODE", numberStr, str, alert);
}

void SpiAnalyzerResults::BuildErrorRsp(U8 nw_type_idx, U8 obj, U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetErrorRspString(nw_type_idx, obj, val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, SIZE_IN_BITS(val), numberStr, sizeof(numberStr));
	if( nw_type_idx == 0 )
	{
		StringBuilder("OBJ_ERR", numberStr, str, alert);
	}
	else
	{
		StringBuilder("NW_ERR", numberStr, str, alert);
	}
}

void SpiAnalyzerResults::BuildIntMask(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetIntMaskString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(e_ABCC_MOSI_INT_MASK), numberStr, sizeof(numberStr));
	StringBuilder(GET_MOSI_FRAME_TAG(e_ABCC_MOSI_INT_MASK), numberStr, str, alert);
}

void SpiAnalyzerResults::BuildAbccStatus(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	bool alert = GetAbccStatusString(val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MISO_ANB_STAT), numberStr, sizeof(numberStr));
	StringBuilder(GET_MISO_FRAME_TAG(e_ABCC_MISO_ANB_STAT), numberStr, str, alert);
}

void SpiAnalyzerResults::BuildApplStatus(U8 val, DisplayBase display_base)
{
	char str[FORMATTED_STRING_BUFFER_SIZE];
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	/* Note ABCC documentation shows U16 datatype for status code, but SPI telegram is U8 */
	bool alert = GetApplStsString((U8)val, &str[0], sizeof(str), display_base);
	AnalyzerHelpers::GetNumberString(val, display_base, GET_MOSI_FRAME_BITSIZE(e_ABCC_MOSI_APP_STAT), numberStr, sizeof(numberStr));
	StringBuilder(GET_MOSI_FRAME_TAG(e_ABCC_MOSI_APP_STAT), numberStr, str, alert);
}

void SpiAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)
{
	ClearResultStrings();
	char numberStr[DISPLAY_NUMERIC_STRING_BUFFER_SIZE];
	char str[FORMATTED_STRING_BUFFER_SIZE];
	bool alert = false;
	Frame frame = GetFrame(frame_index);
	uAbccSpiStates uState;
	uState.eMosi = (tAbccMosiStates)frame.mType;

	if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
	{
		alert = true;
		switch (frame.mType)
		{
			case e_ABCC_SPI_ERROR_FRAGMENTATION:
				if ((IS_MISO_FRAME(frame) && (channel == mSettings->mMisoChannel)) ||
					(IS_MOSI_FRAME(frame) && (channel == mSettings->mMosiChannel)))
				{
					StringBuilder("FRAGMENT", NULL, "Fragmented ABCC SPI Packet.", alert);
				}
				break;
			case e_ABCC_SPI_ERROR_END_OF_TRANSFER:
				StringBuilder("CLOCKING", NULL, "ABCC SPI Clocking. The analyzer expects one transaction per 'Active Enable' phase.", alert);
				break;
			case e_ABCC_SPI_ERROR_GENERIC:
			default:
				StringBuilder("ERROR", NULL, "ABCC SPI Error.", alert);
				break;
		}
	}
	else
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
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", alert);
					break;
				case e_ABCC_MOSI_MSG_LEN:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, alert);
					break;
				case e_ABCC_MOSI_PD_LEN:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					SNPRINTF(str, sizeof(str), "%d Words", (U16)frame.mData1);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, alert);
					break;
				case e_ABCC_MOSI_APP_STAT:
					BuildApplStatus((U8)frame.mData1, display_base);
					break;
				case e_ABCC_MOSI_INT_MASK:
					BuildIntMask((U8)frame.mData1, display_base);
					break;
				case e_ABCC_MOSI_WR_MSG_FIELD:
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
					if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
					{
						if (((U8)frame.mData2) == 0)
						{
							BuildErrorRsp((U8)frame.mData1, display_base);
						}
						else
						{
							U8 nw_type_idx = 0;
							U8 obj = (U8)(frame.mData2 >> (8 * sizeof(U32)));

							/* Bytes 1 and onward are always understood as object-specific
							** or network-specific error codes. The current implementation
							** supports only one extra byte in the error response message
							** data past the object specific or network-specific error */
							if (((U8)frame.mData2) == 2)
							{
								nw_type_idx = (U8)mSettings->mNetworkType;
							}
							BuildErrorRsp(nw_type_idx, obj, (U8)frame.mData1, display_base);
						}
					}
					else
					{
						AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
						SNPRINTF(str, sizeof(str), " [%s] Byte #%d ", numberStr, (U32)frame.mData2);
						if( (mSettings->mMsgDataPriority == e_MSG_DATA_PRIORITIZE_DATA) )
						{
							/* Conditionally trim the leading 0x specifier */
							U8 offset = ( display_base == DisplayBase::Hexadecimal ) ? 2 : 0;
							StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), &numberStr[offset], str, alert, true);
						}
						else
						{
							StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, alert);
						}
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MOSI_WR_MSG_SUBFIELD_data), numberStr, sizeof(numberStr));
					StringBuilder("--", numberStr, NULL, alert, false);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
					{
						SNPRINTF(str, sizeof(str), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
						alert = true;
					}
					else
					{
						SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
					}
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, alert);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_res1:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", alert);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, NULL, alert);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
					BuildObjectString((U8)frame.mData1, display_base);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
					BuildInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, display_base);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
					BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_res2:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, "Reserved", alert);
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
						AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
						StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, NULL, alert);
					}
					break;
				case e_ABCC_MOSI_WR_PD_FIELD:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", numberStr, frame.mData2);
					if( (mSettings->mProcessDataPriority == e_PROCESS_DATA_PRIORITIZE_DATA) )
					{
						/* Conditionally trim the leading 0x specifier */
						U8 offset = ( display_base == DisplayBase::Hexadecimal ) ? 2 : 0;
						StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), &numberStr[offset], str, alert, true);
					}
					else
					{
						StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, alert);
					}
					break;
				case e_ABCC_MOSI_CRC32:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
					{
						SNPRINTF(str, sizeof(str), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
						alert = true;
					}
					else
					{
						SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1 & 0xFFFFFFFF), (U32)(frame.mData2 & 0xFFFFFFFF));
					}
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, str, alert);
					break;
				case e_ABCC_MOSI_PAD:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(uState.eMosi), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MOSI_FRAME_TAG(uState.eMosi), numberStr, NULL, alert);
					break;
				default:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, numberStr, sizeof(numberStr));
					alert = true;
					StringBuilder("UNKOWN", numberStr, "Internal Error: Unknown State", alert);
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
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", alert);
					break;
				case e_ABCC_MISO_Reserved2:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));

					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", alert);
					break;
				case e_ABCC_MISO_LED_STAT:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					alert = GetLedStatusString((U16)frame.mData1, str, sizeof(str), display_base);
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, alert);
				break;
				case e_ABCC_MISO_ANB_STAT:
					BuildAbccStatus((U8)frame.mData1, display_base);
					break;
				case e_ABCC_MISO_SPI_STAT:
					BuildSpiStsString((U8)frame.mData1, display_base);
					break;
				case e_ABCC_MISO_NET_TIME:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, NULL, alert);
					break;
				case e_ABCC_MISO_RD_MSG_FIELD:
				case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
					if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
					{
						if (((U8)frame.mData2) == 0)
						{
							BuildErrorRsp((U8)frame.mData1, display_base);
						}
						else
						{
							U8 nw_type_idx = 0;
							U8 obj = (U8)(frame.mData2 >> (8 * sizeof(U32)));

							/* Bytes 1 and onward are always understood as object-specific
							** or network-specific error codes. The current implementation
							** supports only one extra byte in the error response message
							** data past the object specific or network-specific error */
							if (((U8)frame.mData2) == 2)
							{
								nw_type_idx = (U8)mSettings->mNetworkType;
							}
							BuildErrorRsp(nw_type_idx, obj, (U8)frame.mData1, display_base);
						}
					}
					else
					{
						AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
						SNPRINTF(str, sizeof(str), " [%s] Byte #%d ", numberStr, (U32)frame.mData2);
						if( (mSettings->mMsgDataPriority == e_MSG_DATA_PRIORITIZE_DATA) )
						{
							/* Conditionally trim the leading 0x specifier */
							U8 offset = ( display_base == DisplayBase::Hexadecimal ) ? 2 : 0;
							StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), &numberStr[offset], str, alert, true);
						}
						else
						{
							StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, alert);
						}
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(e_ABCC_MISO_RD_MSG_SUBFIELD_data), numberStr, sizeof(numberStr));
					StringBuilder("--", numberStr, NULL, alert, false);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
					{
						SNPRINTF(str, sizeof(str), "%d Bytes, Exceeds Maximum Size of %d", (U16)frame.mData1, ABP_MAX_MSG_DATA_BYTES);
						alert = true;
					}
					else
					{
						SNPRINTF(str, sizeof(str), "%d Bytes", (U16)frame.mData1);
					}
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, alert);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_res1:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", alert);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, NULL, alert);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
					BuildObjectString((U8)frame.mData1, display_base);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
					BuildInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, display_base);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
					BuildCmdString((U8)frame.mData1, (U8)frame.mData2, display_base);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_res2:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					alert = (frame.mData1 != 0);
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, "Reserved", alert);
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
						AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
						StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, NULL, alert);
					}
					break;
				case e_ABCC_MISO_RD_PD_FIELD:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					SNPRINTF(str, sizeof(str), " [%s] Byte #%lld ", numberStr, frame.mData2);
					if( (mSettings->mProcessDataPriority == e_PROCESS_DATA_PRIORITIZE_DATA) )
					{
						/* Conditionally trim the leading 0x specifier */
						U8 offset = ( display_base == DisplayBase::Hexadecimal ) ? 2 : 0;
						StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), &numberStr[offset], str, alert, true);
					}
					else
					{
						StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, alert);
					}
					break;
				case e_ABCC_MISO_CRC32:
				{
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(uState.eMiso), numberStr, sizeof(numberStr));
					if ((frame.mFlags & (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG)) == (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG))
					{
						SNPRINTF(str, sizeof(str), "ERROR - Received 0x%08X != Calculated 0x%08X", (U32)(frame.mData1), (U32)(frame.mData2));
						alert = true;
					}
					else
					{
						SNPRINTF(str, sizeof(str), "Received 0x%08X == Calculated 0x%08X", (U32)(frame.mData1), (U32)(frame.mData2));
					}
					StringBuilder(GET_MISO_FRAME_TAG(uState.eMiso), numberStr, str, alert);
					break;
				}
				default:
					AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, numberStr, sizeof(numberStr));
					alert = true;
					StringBuilder("UNKOWN", numberStr, "Internal Error: Unknown State", alert);
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

	ss << "Channel" CSV_DELIMITER
		  "Time [s]" CSV_DELIMITER
		  "Packet ID" CSV_DELIMITER
		  "Frame Type" CSV_DELIMITER
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
			ss << CSV_DELIMITER CSV_DELIMITER;
		}

		if ((frame.mFlags & SPI_ERROR_FLAG) == SPI_ERROR_FLAG)
		{
			switch (frame.mType)
			{
			case e_ABCC_SPI_ERROR_FRAGMENTATION:
				ss << "FRAGMENT";
				break;
			case e_ABCC_SPI_ERROR_END_OF_TRANSFER:
				ss << "CLOCKING";
				break;
			case e_ABCC_SPI_ERROR_GENERIC:
			default:
				ss << "GENERIC";
				break;
			}
		}
		else
		{
			if (IS_MOSI_FRAME(frame))
			{
				ss << GET_MOSI_FRAME_TAG((tAbccMosiStates)frame.mType);
			}
			else
			{
				ss << GET_MISO_FRAME_TAG((tAbccMisoStates)frame.mType);
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
	ssMosiHead << "Channel" CSV_DELIMITER
				  "Time [s]" CSV_DELIMITER
				  "Packet ID" CSV_DELIMITER
				  "Error Event" CSV_DELIMITER
				  "Anybus State" CSV_DELIMITER
				  "Application State" CSV_DELIMITER
				  "Message Fragmentation" CSV_DELIMITER
				  "Message Size [bytes]" CSV_DELIMITER
				  "Source ID" CSV_DELIMITER
				  "Object" CSV_DELIMITER
				  "Instance" CSV_DELIMITER
				  "Command" CSV_DELIMITER
				  "CmdExt" CSV_DELIMITER
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
					case e_ABCC_SPI_ERROR_FRAGMENTATION:
						mosiEvent = ErrorEvent::SpiFragmentationError;
						break;
					case e_ABCC_MOSI_SPI_CTRL:
					{
						bool message = ((frame.mData1 & ABP_SPI_CTRL_M) != 0);

						if (message)
						{
							/* Add in the timestamp, packet ID */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMosiHead << std::endl
									   << MOSI_STR CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
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
					case e_ABCC_MOSI_APP_STAT:
					{
						mosiAppStatReached = true;
						GetApplStsString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
					{
						AnalyzerHelpers::GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
					{
						GetObjectString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
					{
						bool alert = false;
						bool found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, dataStr, sizeof(dataStr), &alert, display_base);

						if (!found)
						{
							SNPRINTF(dataStr, sizeof(dataStr), "0x%04X", (U16)frame.mData1);
						}

						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
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
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
					{
						bool alert = false;
						DisplayBase displayBase = DisplayBase::Hexadecimal;

						if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
							((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
						{
							tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), false, &alert, displayBase);
						}
						else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								 ((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
						{
							tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), true, &alert, displayBase);
						}
						else
						{
							AnalyzerHelpers::GetNumberString(frame.mData1, displayBase, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						}

						ssMosiTail << CSV_DELIMITER << dataStr;
						addLastMosiMsgHeader = false;
						break;
					}
					case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
					case e_ABCC_MOSI_WR_MSG_FIELD:
					{
						if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
						{
							if ((U32)frame.mData2 == 0)
							{
								GetErrorRspString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
							else
							{
								U8 obj = (U8)(frame.mData2 >> (8 * sizeof(U32)));
								GetErrorRspString((U8)mSettings->mNetworkType, obj, (U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
						}
						else
						{
							AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						}

						if (addLastMosiMsgHeader)
						{
							ssMosiTail << CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER;
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
					case e_ABCC_MOSI_CRC32:
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
					case e_ABCC_SPI_ERROR_FRAGMENTATION:
						misoEvent = ErrorEvent::SpiFragmentationError;
						break;
					case e_ABCC_MISO_ANB_STAT:
					{
						misoAnbStatReached = true;
						GetAbccStatusString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MISO_SPI_STAT:
					{
						bool message = ((frame.mData1 & ABP_SPI_STATUS_M) != 0);

						if (message)
						{
							/* Add in the timestamp, packet ID, and Anybus state */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMisoHead << std::endl
									   << MISO_STR CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
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
					case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
					case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
					{
						AnalyzerHelpers::GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
					{
						GetObjectString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
					{
						bool alert = false;
						bool found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, dataStr, sizeof(dataStr), &alert, display_base);

						if (!found)
						{
							SNPRINTF(dataStr, sizeof(dataStr), "0x%04X", (U16)frame.mData1);
						}

						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
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
					case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
					{
						bool alert = false;
						DisplayBase displayBase = DisplayBase::Hexadecimal;

						if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
							((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR))
						{
							tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), false, &alert, displayBase);
						}
						else if (((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
								 ((ABP_MsgCmdType)(frame.mData2 & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR))
						{
							tMsgHeaderInfo* psMsgHdr = (tMsgHeaderInfo*)&frame.mData2;
							GetAttrString(psMsgHdr->obj, psMsgHdr->inst, (U16)frame.mData1, dataStr, sizeof(dataStr), true, &alert, displayBase);
						}
						else
						{
							AnalyzerHelpers::GetNumberString(frame.mData1, displayBase, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						}

						ssMisoTail << CSV_DELIMITER << dataStr;
						addLastMisoMsgHeader = false;
						break;
					}
					case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
					case e_ABCC_MISO_RD_MSG_FIELD:
					{
						if ((frame.mFlags & SPI_PROTO_EVENT_FLAG) == SPI_PROTO_EVENT_FLAG)
						{
							if ((U32)frame.mData2 == 0)
							{
								GetErrorRspString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
							else
							{
								U8 obj = (U8)(frame.mData2 >> (8 * sizeof(U32)));
								GetErrorRspString((U8)mSettings->mNetworkType, obj, (U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
							}
						}
						else
						{
							AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						}

						if (addLastMisoMsgHeader)
						{
							ssMisoTail << CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER CSV_DELIMITER;
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
					case e_ABCC_MISO_CRC32:
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
					case e_ABCC_SPI_ERROR_FRAGMENTATION:
						mosiEvent = ErrorEvent::SpiFragmentationError;
						break;
					case e_ABCC_MOSI_PD_LEN:
						if (addCsvHeader)
						{
							U32 dwBytes = ((U16)frame.mData1) << 1;
							/* Add header fields */
							std::stringstream ssHeader;
							ssHeader << "Channel" CSV_DELIMITER
										"Time [s]" CSV_DELIMITER
										"Packet ID" CSV_DELIMITER
										"Error Event" CSV_DELIMITER
										"Anybus State" CSV_DELIMITER
										"Application State" CSV_DELIMITER
										"Network Time";

							for (U16 cnt = 0; cnt < dwBytes; cnt++)
							{
								ssHeader << CSV_DELIMITER "Process Data " << cnt;
								AnalyzerHelpers::AppendToFile((U8*)ssHeader.str().c_str(), (U32)ssHeader.str().length(), f);
								ssHeader.str(std::string());
							}

							AnalyzerHelpers::AppendToFile((U8*)ssHeader.str().c_str(), (U32)ssHeader.str().length(), f);
							addCsvHeader = false;
						}

						break;
					case e_ABCC_MOSI_SPI_CTRL:
					{
						if (frame.mData1 & ABP_SPI_CTRL_WRPD_VALID)
						{
							/* Add in the timestamp, packet ID */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMosiHead << std::endl
									   << MOSI_STR CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
							addMosiEntry = true;
						}

						break;
					}
					case e_ABCC_MOSI_APP_STAT:
					{
						mosiAppStatReached = true;
						GetApplStsString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MOSI_WR_PD_FIELD:
					{
						AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MOSI_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						ssMosiTail << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MOSI_CRC32:
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
					case e_ABCC_SPI_ERROR_FRAGMENTATION:
						misoEvent = ErrorEvent::SpiFragmentationError;
						break;
					case e_ABCC_MISO_ANB_STAT:
					{
						misoAnbStatReached = true;
						GetAbccStatusString((U8)frame.mData1, &dataStr[0], sizeof(dataStr), display_base);
						ssSharedBody << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MISO_SPI_STAT:
					{
						if (frame.mData1 & ABP_SPI_STATUS_NEW_PD)
						{
							/* Add in the timestamp, packet ID */
							AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, triggerSample, sampleRate, timeStr, DISPLAY_NUMERIC_STRING_BUFFER_SIZE);
							ssMisoHead << std::endl
									   << MISO_STR CSV_DELIMITER << timeStr << CSV_DELIMITER << packetId;
							addMisoEntry = true;
						}

						break;
					}
					case e_ABCC_MISO_NET_TIME:
					{
						/* Append network time stamp to both string streams */
						AnalyzerHelpers::GetNumberString(frame.mData1, DisplayBase::Decimal, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						ssMisoTail << CSV_DELIMITER << dataStr;
						ssMosiTail << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MISO_RD_PD_FIELD:
					{
						AnalyzerHelpers::GetNumberString(frame.mData1, display_base, GET_MISO_FRAME_BITSIZE(frame.mType), dataStr, sizeof(dataStr));
						ssMisoTail << CSV_DELIMITER << dataStr;
						break;
					}
					case e_ABCC_MISO_CRC32:
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
	switch (export_type_user_id)
	{
		case e_EXPORT_FRAMES:
			/* Export all frame data */
			ExportAllFramesToFile(file, display_base);
			break;
		case e_EXPORT_MESSAGE_DATA:
			/* Export 'valid' message data */
			ExportMessageDataToFile(file, display_base);
			break;
		case e_EXPORT_PROCESS_DATA:
			/* Export 'valid' process data */
			ExportProcessDataToFile(file, display_base);
			break;
	}
}

U64 SpiAnalyzerResults::GetFrameIdOfAbccFieldContainedInPacket(U64 packet_index, bool is_mosi_channel, U8 type)
{
	U64 frameIndex = INVALID_RESULT_INDEX;
	U64 firstFrameIndex;
	U64 lastFrameIndex;

	if(packet_index != INVALID_RESULT_INDEX)
	{
		GetFramesContainedInPacket(packet_index, &firstFrameIndex, &lastFrameIndex);
		if((firstFrameIndex != INVALID_RESULT_INDEX) && (lastFrameIndex != INVALID_RESULT_INDEX))
		{
			for(frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++)
			{
				Frame frame = GetFrame(frameIndex);
				if( ((is_mosi_channel) && IS_MOSI_FRAME(frame) && (frame.mType == type)) ||
					((!is_mosi_channel) && IS_MISO_FRAME(frame) && (frame.mType == type)) )
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

	if (mSettings->mErrorIndexing == true)
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
					case e_ABCC_SPI_ERROR_FRAGMENTATION:
						AddTabularText("!FRAGMENT: ABCC SPI Packet is Fragmented");
						break;
					case e_ABCC_SPI_ERROR_END_OF_TRANSFER:
						AddTabularText("!CLOCKING: Unexpected ABCC SPI Clocking Behavior");
						break;
					case e_ABCC_SPI_ERROR_GENERIC:
					default:
						AddTabularText("!ERROR: General Error in ABCC SPI Communication");
						break;
				}
			}
			return;
		}

		if (frame.mFlags & SPI_PROTO_EVENT_FLAG)
		{
			if (frame.mType == e_ABCC_MISO_CRC32)
			{
				TableBuilder(IS_MOSI_FRAME(frame), "CRC32", true);
				return;
			}
			else if ((frame.mType == e_ABCC_MOSI_APP_STAT) && IS_MOSI_FRAME(frame))
			{
				/* Note ABCC documentation show U16 data type of status code, but SPI telegram is U8 */
				if (GetApplStsString((U8)frame.mData1, str, sizeof(str), display_base))
				{
					AddTabularText("!Application Status: ", str);
					return;
				}
				else
				{
					if (mSettings->mApplStatusIndexing == true)
					{
						AddTabularText("Application Status: ", str);
						return;
					}
				}
			}
			else if ((frame.mType == e_ABCC_MISO_ANB_STAT) && IS_MISO_FRAME(frame))
			{
				char tabText[FORMATTED_STRING_BUFFER_SIZE];
				bool alert = false;
				if (GetAbccStatusString((U8)frame.mData1, &str[0], sizeof(str), display_base))
				{
					alert = true;
				}

				if (alert || (mSettings->mAnybusStatusIndexing == true))
				{
					SNPRINTF(tabText, sizeof(tabText), "Anybus Status: (%s)", str);
					TableBuilder(true, tabText, alert);
				}
				return;
			}
			else if ((frame.mType == e_ABCC_MISO_RD_MSG_SUBFIELD_size) && IS_MISO_FRAME(frame))
			{
				TableBuilder(false, "Message Size: Exceeds Maximum", true);
				return;
			}
			else if ((frame.mType == e_ABCC_MOSI_WR_MSG_SUBFIELD_size) && IS_MOSI_FRAME(frame))
			{
				TableBuilder(true, "Message Size: Exceeds Maximum", true);
				return;
			}
		}
	}

	if ((frame.mType == e_ABCC_MISO_NET_TIME) && IS_MISO_FRAME(frame))
	{
		bool addEntry = false;

		switch(mSettings->mTimestampIndexing)
		{
		case e_TIMESTAMP_ALL_PACKETS:
			addEntry = true;
			break;
		case e_TIMESTAMP_WRITE_PROCESS_DATA_VALID:
			if (((tNetworkTimeInfo*)&frame.mData2)->wrPdValid)
			{
				addEntry = true;
			}
			break;
		case e_TIMESTAMP_NEW_READ_PROCESS_DATA:
			if (((tNetworkTimeInfo*)&frame.mData2)->newRdPd)
			{
				addEntry = true;
			}
			break;
		default:
		case e_TIMESTAMP_DISABLED:
			break;
		}

		if (addEntry)
		{
			U32 delta = ((tNetworkTimeInfo*)&frame.mData2)->deltaTime;
			SNPRINTF(str, sizeof(str), "0x%08X (Delta: 0x%08X)", (U32)frame.mData1, delta);
			AddTabularText("Time: ", str);
			SNPRINTF(str, sizeof(str), "Packet: 0x%016llX", packetId);
			AddTabularText(str);
		}

		return;
	}

	if (mSettings->mApplStatusIndexing == true)
	{
		if ((frame.mType == e_ABCC_MOSI_APP_STAT) && IS_MOSI_FRAME(frame))
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

	if (mSettings->mAnybusStatusIndexing == true)
	{
		if ((frame.mType == e_ABCC_MISO_ANB_STAT) && IS_MISO_FRAME(frame))
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
	if (mSettings->mMessageIndexingVerbosityLevel != e_VERBOSITY_LEVEL_DISABLED)
	{
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
						SNPRINTF(acMsgSizeStr[ABCC_MISO_CHANNEL], sizeof(acMsgSizeStr[ABCC_MISO_CHANNEL]), "!Size: %u Bytes", (U16)frame.mData1);
					}
					else
					{
						SNPRINTF(acMsgSizeStr[ABCC_MISO_CHANNEL], sizeof(acMsgSizeStr[ABCC_MISO_CHANNEL]), "Size: %u Bytes", (U16)frame.mData1);
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
					SNPRINTF(acMsgSrcStr[ABCC_MISO_CHANNEL], sizeof(acMsgSrcStr[ABCC_MISO_CHANNEL]), "Source ID: %d (0x%02X)", (U8)frame.mData1, (U8)frame.mData1);
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						bool alert = GetObjectString((U8)frame.mData1, &str[0], sizeof(str), display_base);
						if (alert)
						{
							SNPRINTF(acMsgObjStr[ABCC_MISO_CHANNEL], sizeof(acMsgObjStr[ABCC_MISO_CHANNEL]), "!Object: %s", str);
						}
						else
						{
							SNPRINTF(acMsgObjStr[ABCC_MISO_CHANNEL], sizeof(acMsgObjStr[ABCC_MISO_CHANNEL]), "Object: %s", str);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(acMsgObjStr[ABCC_MISO_CHANNEL], sizeof(acMsgObjStr[ABCC_MISO_CHANNEL]), "Obj {%02X:", (U8)frame.mData1);
					}
					break;
				case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						bool found = false;
						bool alert = false;
						found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, str, sizeof(str), &alert, display_base);
						if (!found)
						{
							SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
						}
						if (alert)
						{
							SNPRINTF(acMsgInstStr[ABCC_MISO_CHANNEL], sizeof(acMsgInstStr[ABCC_MISO_CHANNEL]), "!Instance: %s", str);
						}
						else
						{
							SNPRINTF(acMsgInstStr[ABCC_MISO_CHANNEL], sizeof(acMsgInstStr[ABCC_MISO_CHANNEL]), "Instance: %s", str);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(acMsgInstStr[ABCC_MISO_CHANNEL], sizeof(acMsgInstStr[ABCC_MISO_CHANNEL]), "%04Xh}", (U16)frame.mData1);
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
						bool alert = GetCmdString((U8)frame.mData1, (U8)frame.mData2, &str[0], sizeof(str), display_base);
						if ((fMsgErrorRsp[ABCC_MISO_CHANNEL] == true) || (alert == true))
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(acMsgCmdStr[ABCC_MISO_CHANNEL], sizeof(acMsgCmdStr[ABCC_MISO_CHANNEL]), "!Command: %s", str);
							}
							else
							{
								SNPRINTF(acMsgCmdStr[ABCC_MISO_CHANNEL], sizeof(acMsgCmdStr[ABCC_MISO_CHANNEL]), "!Response: %s", str);
							}
						}
						else
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(acMsgCmdStr[ABCC_MISO_CHANNEL], sizeof(acMsgCmdStr[ABCC_MISO_CHANNEL]), "Command: %s", str);
							}
							else
							{
								SNPRINTF(acMsgCmdStr[ABCC_MISO_CHANNEL], sizeof(acMsgCmdStr[ABCC_MISO_CHANNEL]), "Response: %s", str);
							}
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(acMsgCmdStr[ABCC_MISO_CHANNEL], sizeof(acMsgCmdStr[ABCC_MISO_CHANNEL]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
						}
						else
						{
							SNPRINTF(acMsgCmdStr[ABCC_MISO_CHANNEL], sizeof(acMsgCmdStr[ABCC_MISO_CHANNEL]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
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
							TableBuilder(false, acMsgSrcStr[ABCC_MISO_CHANNEL], false);
						}
						if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
						{
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
								/* For consistency with Source ID and Instance, only use hex format */
								SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
							}
							if (alert)
							{
								SNPRINTF(acMsgExtStr[ABCC_MISO_CHANNEL], sizeof(acMsgExtStr[ABCC_MISO_CHANNEL]), "!Extension: %s", str);
							}
							else
							{
								SNPRINTF(acMsgExtStr[ABCC_MISO_CHANNEL], sizeof(acMsgExtStr[ABCC_MISO_CHANNEL]), "Extension: %s", str);
							}
							TableBuilder(false, acMsgSizeStr[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, acMsgObjStr[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, acMsgInstStr[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, acMsgCmdStr[ABCC_MISO_CHANNEL], false);
							TableBuilder(false, acMsgExtStr[ABCC_MISO_CHANNEL], false);
							if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								TableBuilder(false, "First Fragment; More Follow.", false);
							}
						}
						else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
						{
							SNPRINTF(acMsgExtStr[ABCC_MISO_CHANNEL], sizeof(acMsgExtStr[ABCC_MISO_CHANNEL]), "%04Xh}", (U16)frame.mData1);
							if (fMsgErrorRsp[ABCC_MISO_CHANNEL])
							{
								AddTabularText(MISO_TAG_STR, "!", acMsgObjStr[ABCC_MISO_CHANNEL], acMsgInstStr[ABCC_MISO_CHANNEL], acMsgCmdStr[ABCC_MISO_CHANNEL], acMsgExtStr[ABCC_MISO_CHANNEL]);
							}
							else
							{
								if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
								{
									AddTabularText(MISO_TAG_STR, acMsgObjStr[ABCC_MISO_CHANNEL], acMsgInstStr[ABCC_MISO_CHANNEL], acMsgCmdStr[ABCC_MISO_CHANNEL], acMsgExtStr[ABCC_MISO_CHANNEL], "++");
								}
								else
								{
									AddTabularText(MISO_TAG_STR, acMsgObjStr[ABCC_MISO_CHANNEL], acMsgInstStr[ABCC_MISO_CHANNEL], acMsgCmdStr[ABCC_MISO_CHANNEL], acMsgExtStr[ABCC_MISO_CHANNEL]);
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
						SNPRINTF(acMsgSizeStr[ABCC_MOSI_CHANNEL], sizeof(acMsgSizeStr[ABCC_MOSI_CHANNEL]), "!Size: %u Bytes", (U16)frame.mData1);
					}
					else
					{
						SNPRINTF(acMsgSizeStr[ABCC_MOSI_CHANNEL], sizeof(acMsgSizeStr[ABCC_MOSI_CHANNEL]), "Size: %u Bytes", (U16)frame.mData1);
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
					SNPRINTF(acMsgSrcStr[ABCC_MOSI_CHANNEL], sizeof(acMsgSrcStr[ABCC_MOSI_CHANNEL]), "Source ID: %d (0x%02X)", (U8)frame.mData1, (U8)frame.mData1);
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						bool alert = GetObjectString((U8)frame.mData1, &str[0], sizeof(str), display_base);
						if (alert)
						{
							SNPRINTF(acMsgObjStr[ABCC_MOSI_CHANNEL], sizeof(acMsgObjStr[ABCC_MOSI_CHANNEL]), "!Object: %s", str);
						}
						else
						{
							SNPRINTF(acMsgObjStr[ABCC_MOSI_CHANNEL], sizeof(acMsgObjStr[ABCC_MOSI_CHANNEL]), "Object: %s", str);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(acMsgObjStr[ABCC_MOSI_CHANNEL], sizeof(acMsgObjStr[ABCC_MOSI_CHANNEL]), "Obj {%02X:", (U8)frame.mData1);
					}
					break;
				case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
					if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
					{
						bool found = false;
						bool alert = false;
						found = GetInstString((U8)mSettings->mNetworkType, (U8)frame.mData2, (U16)frame.mData1, str, sizeof(str), &alert, display_base);
						if (!found)
						{
							SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
						}
						if (alert)
						{
							SNPRINTF(acMsgInstStr[ABCC_MOSI_CHANNEL], sizeof(acMsgInstStr[ABCC_MOSI_CHANNEL]), "!Instance: %s", str);
						}
						else
						{
							SNPRINTF(acMsgInstStr[ABCC_MOSI_CHANNEL], sizeof(acMsgInstStr[ABCC_MOSI_CHANNEL]), "Instance: %s", str);
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						SNPRINTF(acMsgInstStr[ABCC_MOSI_CHANNEL], sizeof(acMsgInstStr[ABCC_MOSI_CHANNEL]), "%04Xh}", (U16)frame.mData1);
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
						bool alert = GetCmdString((U8)frame.mData1, (U8)frame.mData2, &str[0], sizeof(str), display_base);
						if ((fMsgErrorRsp[ABCC_MOSI_CHANNEL] == true) || (alert == true))
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(acMsgCmdStr[ABCC_MOSI_CHANNEL], sizeof(acMsgCmdStr[ABCC_MOSI_CHANNEL]), "!Command: %s", str);
							}
							else
							{
								SNPRINTF(acMsgCmdStr[ABCC_MOSI_CHANNEL], sizeof(acMsgCmdStr[ABCC_MOSI_CHANNEL]), "!Response: %s", str);
							}
						}
						else
						{
							if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
							{
								SNPRINTF(acMsgCmdStr[ABCC_MOSI_CHANNEL], sizeof(acMsgCmdStr[ABCC_MOSI_CHANNEL]), "Command: %s", str);
							}
							else
							{
								SNPRINTF(acMsgCmdStr[ABCC_MOSI_CHANNEL], sizeof(acMsgCmdStr[ABCC_MOSI_CHANNEL]), "Response: %s", str);
							}
						}
					}
					else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
					{
						if ((U8)frame.mData1 & ABP_MSG_HEADER_C_BIT)
						{
							SNPRINTF(acMsgCmdStr[ABCC_MOSI_CHANNEL], sizeof(acMsgCmdStr[ABCC_MOSI_CHANNEL]), ", Cmd {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
						}
						else
						{
							SNPRINTF(acMsgCmdStr[ABCC_MOSI_CHANNEL], sizeof(acMsgCmdStr[ABCC_MOSI_CHANNEL]), ", Rsp {%02X:", (U8)(frame.mData1 & ABP_MSG_HEADER_CMD_BITS));
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
							TableBuilder(true, acMsgSrcStr[ABCC_MOSI_CHANNEL], false);
						}
						if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_DETAILED)
						{
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
								/* For consistency with Source ID and Instance, only use hex format */
								SNPRINTF(str, sizeof(str), "%d (0x%04X)", (U16)frame.mData1, (U16)frame.mData1);
							}
							if (alert)
							{
								SNPRINTF(acMsgExtStr[ABCC_MOSI_CHANNEL], sizeof(acMsgExtStr[ABCC_MOSI_CHANNEL]), "!Extension: %s", str);
							}
							else
							{
								SNPRINTF(acMsgExtStr[ABCC_MOSI_CHANNEL], sizeof(acMsgExtStr[ABCC_MOSI_CHANNEL]), "Extension: %s", str);
							}
							TableBuilder(true, acMsgSizeStr[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, acMsgObjStr[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, acMsgInstStr[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, acMsgCmdStr[ABCC_MOSI_CHANNEL], false);
							TableBuilder(true, acMsgExtStr[ABCC_MOSI_CHANNEL], false);
							if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
							{
								TableBuilder(true, "First Fragment; More Follow.", false);
							}
						}
						else if (mSettings->mMessageIndexingVerbosityLevel == e_VERBOSITY_LEVEL_COMPACT)
						{
							SNPRINTF(acMsgExtStr[ABCC_MOSI_CHANNEL], sizeof(acMsgExtStr[ABCC_MOSI_CHANNEL]), "%04Xh}", (U16)frame.mData1);
							if (fMsgErrorRsp[ABCC_MOSI_CHANNEL])
							{
								AddTabularText(MISO_TAG_STR, "!", acMsgObjStr[ABCC_MOSI_CHANNEL], acMsgInstStr[ABCC_MOSI_CHANNEL], acMsgCmdStr[ABCC_MOSI_CHANNEL], acMsgExtStr[ABCC_MOSI_CHANNEL]);
							}
							else
							{
								if (frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG)
								{
									AddTabularText(MISO_TAG_STR, acMsgObjStr[ABCC_MOSI_CHANNEL], acMsgInstStr[ABCC_MOSI_CHANNEL], acMsgCmdStr[ABCC_MOSI_CHANNEL], acMsgExtStr[ABCC_MOSI_CHANNEL], "++");
								}
								else
								{
									AddTabularText(MISO_TAG_STR, acMsgObjStr[ABCC_MOSI_CHANNEL], acMsgInstStr[ABCC_MOSI_CHANNEL], acMsgCmdStr[ABCC_MOSI_CHANNEL], acMsgExtStr[ABCC_MOSI_CHANNEL]);
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

void SpiAnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/)  //unreferenced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SpiAnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/)  //unreferenced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}

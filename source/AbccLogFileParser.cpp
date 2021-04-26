/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccLogFileParser.cpp
**    Summary: Defines helper class for parsing a standard ABCC SDK log
**             for messages and Anybus status events.
**
*******************************************************************************
******************************************************************************/

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "AnalyzerHelpers.h"
#include "abcc_td.h"
#include "abcc_abp/abp.h"
#include "AbccLogFileParser.h"

static inline void StringReplace(std::string& line, const char search_ch, const char replace_ch)
{
	size_t offset = 0;

	while (true)
	{
		offset = line.find(search_ch, offset);

		if (offset == std::string::npos)
		{
			break;
		}

		line[offset++] = replace_ch;
	}
}

AbccLogFileParser::AbccLogFileParser(const std::string& filepath, const ABP_AnbStateType state)
{
	mLogFileStream.open(filepath);
	mAnbState = state;
}

AbccLogFileParser::~AbccLogFileParser()
{
	if (mLogFileStream.is_open())
	{
		mLogFileStream.close();
	}
}

bool AbccLogFileParser::IsOpen()
{
	return mLogFileStream.is_open();
}

ABP_AnbStateType AbccLogFileParser::GetAnbStatus()
{
	return mAnbState;
}

MessageReturnType AbccLogFileParser::GetNextMessage(ABP_MsgType& message)
{
	MessageReturnType msgType = MessageReturnType::EndOfFile;
	std::string line;

	if (!mLogFileStream.is_open())
	{
		return MessageReturnType::IoError;
	}

	while (getline(mLogFileStream, line))
	{
		const char* msgRx = "Msg received:";
		const char* msgTx = "Msg sent:";
		const char* anbStatus = "ANB_STATUS:";
		bool parseMessage = false;

		if (line.find(msgTx) != std::string::npos)
		{
			parseMessage = true;
			msgType = MessageReturnType::Tx;
		}
		else if (line.find(msgRx) != std::string::npos)
		{
			parseMessage = true;
			msgType = MessageReturnType::Rx;
		}

		if (parseMessage)
		{
			if (ParseMessage(message))
			{
				return msgType;
			}
			else if (msgType == MessageReturnType::Tx)
			{
				msgType = MessageReturnType::TxError;
			}
			else if (msgType == MessageReturnType::Rx)
			{
				msgType = MessageReturnType::RxError;
			}

			break;
		}
		else if (line.find(anbStatus) != std::string::npos)
		{
			// Line indicates the Anybus State
			S8 newStatus = ParseAnbState(line);

			msgType = MessageReturnType::StateChange;

			if (newStatus >= 0)
			{
				mAnbState = static_cast<ABP_AnbStateType>(newStatus);
			}

			break;
		}
	}

	return msgType;
}

bool AbccLogFileParser::ParseMessage(ABP_MsgType& message)
{
	std::string line;
	UINT16 lineCount = 0;
	UINT16 dataCount = 0;
	UINT32 dataStartCount = 0;
	bool checkForStartToken = true;

	while (getline(mLogFileStream, line))
	{
		const char* line1Format = "[ MsgBuf:0x%llx Size:0x%x SrcId  :0x%x DestObj:0x%x";
		const char* line2Format = "  Inst  :0x%x     Cmd :0x%x   CmdExt0:0x%x CmdExt1:0x%x ]";
		const int line1ExpectedMatches = 4;
		const int line2ExpectedMatches = 4;

		UINT64 dummy;
		unsigned int parsedInts[4];
		int matches;

		switch (lineCount)
		{
			case 0:
				memset(&message, 0, sizeof(ABP_MsgType));
				matches = SSCANF(
					line.c_str(),
					line1Format,
					&dummy,
					&parsedInts[0],
					&parsedInts[1],
					&parsedInts[2]);

				if (matches != line1ExpectedMatches)
				{
					return false;
				}

				if ((parsedInts[0] > UINT16_MAX) ||
					(parsedInts[1] > UINT8_MAX) ||
					(parsedInts[2] > UINT8_MAX))
				{
					// Parsed value exceeds max expected value.
					return false;
				}

				message.sHeader.iDataSize = static_cast<UINT16>(parsedInts[0]);
				message.sHeader.bSourceId = static_cast<UINT8>(parsedInts[1]);
				message.sHeader.bDestObj = static_cast<UINT8>(parsedInts[2]);

				if (message.sHeader.iDataSize > ABP_MAX_MSG_DATA_BYTES)
				{
					message.sHeader.iDataSize = ABP_MAX_MSG_DATA_BYTES;
					return false;
				}

				break;

			case 1:
				matches = SSCANF(
					line.c_str(),
					line2Format,
					&parsedInts[0],
					&parsedInts[1],
					&parsedInts[2],
					&parsedInts[3]);

				if (matches != line2ExpectedMatches)
				{
					return false;
				}

				if ((parsedInts[0] > UINT16_MAX) ||
					(parsedInts[1] > UINT8_MAX) ||
					(parsedInts[2] > UINT8_MAX) ||
					(parsedInts[3] > UINT8_MAX))
				{
					// Parsed value exceeds max expected value.
					return false;
				}

				message.sHeader.iInstance = static_cast<UINT16>(parsedInts[0]);
				message.sHeader.bCmd = static_cast<UINT8>(parsedInts[1]);
				message.sHeader.bCmdExt0 = static_cast<UINT8>(parsedInts[2]);
				message.sHeader.bCmdExt1 = static_cast<UINT8>(parsedInts[3]);

				break;

			default:
			{
				const unsigned int strIntLen = 4;
				const char delim = ' ';
				bool endOfMessage = false;
				bool byteParsed = false;
				size_t strIntBeginOffset = 0;
				size_t strIntEndOffset = line.find(']');

				dataStartCount += static_cast<UINT32>(std::count(line.begin(), line.end(), '['));

				if (dataStartCount > 1)
				{
					// BRACKET ERROR: Bracket count. Only one '[' allowed.
					return false;
				}

				if (checkForStartToken)
				{
					checkForStartToken = false;

					if (line[0] != '[')
					{
						// BRACKET ERROR: '[' must be first character.
						return false;
					}

					strIntBeginOffset++;
				}

				if (strIntEndOffset == std::string::npos)
				{
					strIntEndOffset = line.length();
				}
				else
				{
					endOfMessage = true;
					strIntEndOffset--;
				}

				// Convert any tabs to spaces.
				StringReplace(line, '\t', ' ');

				std::stringstream ss(line.substr(strIntBeginOffset, (strIntEndOffset - strIntBeginOffset)));
				std::string token;

				// Tokenize the substring and parse each as a hexadecimal integer
				while (getline(ss, token, delim))
				{
					if (token.length() == 0)
					{
						continue;
					}
					else if (token.length() > strIntLen)
					{
						// Unexpected integer length.
						return false;
					}

					matches = SSCANF(token.c_str(), "0x%02x", &parsedInts[0]);

					if (matches != 1)
					{
						// Unexpected integer format.
						return false;
					}

					message.abData[dataCount++] = static_cast<UINT8>(parsedInts[0]);
					byteParsed = true;
				}

				if (!(byteParsed || endOfMessage))
				{
					// BRACKET ERROR: Termination missing.
					return false;
				}

				if (endOfMessage)
				{
					bool matchingDataSize = (dataCount == message.sHeader.iDataSize);
					return (matchingDataSize);
				}

				break;
			}
		}

		lineCount++;
	}

	return false;
}

INT8 AbccLogFileParser::ParseAnbState(const std::string& line)
{
	const UINT8 abAnybusStsValues[] =
	{
		ABP_ANB_STATE_SETUP,
		ABP_ANB_STATE_NW_INIT,
		ABP_ANB_STATE_WAIT_PROCESS,
		ABP_ANB_STATE_IDLE,
		ABP_ANB_STATE_PROCESS_ACTIVE,
		ABP_ANB_STATE_ERROR,
		ABP_ANB_STATE_EXCEPTION
	};

	const char* asAnybusStsNames[] =
	{
		"ABP_ANB_STATE_SETUP",
		"ABP_ANB_STATE_NW_INIT",
		"ABP_ANB_STATE_WAIT_PROCESS",
		"ABP_ANB_STATE_IDLE",
		"ABP_ANB_STATE_PROCESS_ACTIVE",
		"ABP_ANB_STATE_ERROR",
		"ABP_ANB_STATE_EXCEPTION"
	};

	INT8 state = -1;

	for (int i = 0; i < sizeof(abAnybusStsValues); i++)
	{
		size_t matchOffset = line.find(asAnybusStsNames[i]);

		if ((matchOffset != std::string::npos) &&
			((line.length() - matchOffset) == strlen(asAnybusStsNames[i])))
		{
			state = abAnybusStsValues[i];
			break;
		}
	}

	return state;
}

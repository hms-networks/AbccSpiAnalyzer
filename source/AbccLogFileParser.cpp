/******************************************************************************
**  Copyright (C) 2015-2022 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccLogFileParser.cpp
**    Summary: Defines helper class for parsing a standard ABCC SDK log
**             for messages and Anybus status events.
**
*******************************************************************************
******************************************************************************/
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <locale>
#include <codecvt>

#include "abcc_td.h"
#include "abcc_abp/abp.h"
#include "AbccLogFileParser.h"
#include "AbccSpiAnalyzerHelpers.h"
#include <iostream>

// Convert a wide Unicode string to UTF8
static void Utf16ToUtf8(const std::wstring& wstr, std::string& str)
{
	if (wstr.empty())
	{
		str.clear();
	}
	else
	{
		str.assign(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr));
	}
}

AbccLogFileParser::AbccLogFileParser(const std::string& filepath, const ABP_AnbStateType state)
{
	mLineDelimiter = L'\n';
	mLogFileStream.open(filepath);
	DetectFileEncoding();

	if ((mEncoding == FileEncoding::Utf16Be) || (mEncoding == FileEncoding::Utf16Le))
	{
		const int bom = 0xFEFF;

		if (mLogFileStream.is_open())
		{
			mLogFileStream.close();
		}

		mLogFileWStream.open(filepath, std::ios::binary);
		mLogFileWStream.imbue(std::locale(mLogFileWStream.getloc(),
			new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));
		mSwapEndianness = mLogFileWStream.get() != bom;
	}

	if (mSwapEndianness)
	{
		mLineDelimiter = bswap_16(mLineDelimiter);
	}

	mAnbState = state;
}

AbccLogFileParser::~AbccLogFileParser()
{
	if (mLogFileStream.is_open())
	{
		mLogFileStream.close();
	}

	if (mLogFileWStream.is_open())
	{
		mLogFileWStream.close();
	}
}

bool AbccLogFileParser::IsOpen()
{
	return (mLogFileStream.is_open() || mLogFileWStream.is_open());
}

void AbccLogFileParser::DetectFileEncoding()
{
	if (!mLogFileStream.is_open())
	{
		mEncoding = FileEncoding::Unknown;
		return;
	}

	int ch1 = mLogFileStream.get();
	int ch2 = mLogFileStream.get();

	// Check for Byte Order Mark 0xFEFF in LE and BE form
	if (ch1 == 0xFF && ch2 == 0xFE)
	{
		mEncoding = FileEncoding::Utf16Le;
	}
	else if (ch1 == 0xFE && ch2 == 0xFF)
	{
		mEncoding = FileEncoding::Utf16Be;
	}
	else
	{
		int ch3 = mLogFileStream.get();
		bool bomPresent = (ch1 == 0xEF && ch2 == 0xBB && ch3 == 0xBF);
		mEncoding = FileEncoding::Utf8;

		if (!bomPresent)
		{
			// Start from beginning before parsing log.
			mLogFileStream.seekg(0);
		}
	}
}

bool AbccLogFileParser::GetLine(std::string& line)
{
	bool error = false;
	bool continueReading = false;

	line.clear();

	if (mEncoding == FileEncoding::Utf8)
	{
		getline(mLogFileStream, line, static_cast<char>(mLineDelimiter));
		error = mLogFileStream.bad() || mLogFileStream.fail();
		continueReading = !(error || mLogFileStream.eof());
	}
	else if (mEncoding == FileEncoding::Utf16Be || mEncoding == FileEncoding::Utf16Le)
	{
		std::wstring wstr;

		if (mSwapEndianness)
		{
			getline(mLogFileWStream, wstr, mLineDelimiter);

			for (size_t i = 0; i < wstr.length(); i++)
			{
				wstr[i] = bswap_16(wstr[i]);
			}
		}
		else
		{
			getline(mLogFileWStream, wstr);
		}

		Utf16ToUtf8(wstr, line);
		error = mLogFileWStream.bad() || mLogFileWStream.fail();
		continueReading = !(error || mLogFileWStream.eof());
	}

	if (error)
	{
		line.clear();
	}

	return continueReading;
}

ABP_AnbStateType AbccLogFileParser::GetAnbStatus()
{
	return mAnbState;
}

MessageReturnType AbccLogFileParser::GetNextMessage(ABP_MsgType& message)
{
	MessageReturnType msgType = MessageReturnType::EndOfFile;
	std::string line;

	if (!(mLogFileStream.is_open() || mLogFileWStream.is_open()))
	{
		return MessageReturnType::IoError;
	}

	while (GetLine(line))
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
			INT8 newStatus = ParseAnbState(line);

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

	while (GetLine(line))
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
					TrimString(token);

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
	const UINT8 anybusStsValues[] =
	{
		ABP_ANB_STATE_SETUP,
		ABP_ANB_STATE_NW_INIT,
		ABP_ANB_STATE_WAIT_PROCESS,
		ABP_ANB_STATE_IDLE,
		ABP_ANB_STATE_PROCESS_ACTIVE,
		ABP_ANB_STATE_ERROR,
		ABP_ANB_STATE_EXCEPTION
	};

	const char* anybusStsNames[] =
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

	for (int i = 0; i < sizeof(anybusStsValues); i++)
	{
		size_t matchOffset = line.find(anybusStsNames[i]);

		if ((matchOffset != std::string::npos) &&
			((line.length() - matchOffset) == strlen(anybusStsNames[i])))
		{
			state = anybusStsValues[i];
			break;
		}
	}

	return state;
}

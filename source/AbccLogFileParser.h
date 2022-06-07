/******************************************************************************
**  Copyright (C) 2015-2022 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccLogFileParser.h
**    Summary: Defines helper class for parsing a standard ABCC SDK log
**             for messages and Anybus status events.
**
*******************************************************************************
******************************************************************************/
#pragma once

#ifndef ABCC_SPI_SIMULATION_FILE_PARSER_H
#define ABCC_SPI_SIMULATION_FILE_PARSER_H

#include <string>
#include <fstream>

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#ifdef _MSC_VER
	#include <stdlib.h>
	#define bswap_16(x) _byteswap_ushort(x)
	#define SSCANF sscanf_s
#elif defined(__APPLE__)
	#include <libkern/OSByteOrder.h>
	#define bswap_16(x) OSSwapInt16(x)
	#define SSCANF sscanf
#else
	#include <byteswap.h>
	#define SSCANF sscanf
#endif

/*
** @brief Enum class indicating the type of message or event parsed from the ABCC SDK log file.
*/
enum class MessageReturnType
{
	EndOfFile,
	StateChange,
	Tx,
	Rx,
	TxError,
	RxError,
	IoError
};

/*
** @brief Enum class indicating the file encoding.
*/
enum class FileEncoding
{
	Unknown,
	Utf8,
	Utf16Le,
	Utf16Be
};

/*
** @brief Helper class for parsing an ABCC SDK log file.
*/
class AbccLogFileParser
{
public:

	/*******************************************************************************
	** @brief Construct a new Abcc Log File Parser object.
	**
	** @param filepath - The log file to parse.
	** @param state    - The default Anybus state to assume at the start of parsing
	**                   the log file.
	*/
	AbccLogFileParser(const std::string& filepath, const ABP_AnbStateType state = ABP_ANB_STATE_SETUP);

	/*******************************************************************************
	** @brief Destroy the Abcc Log File Parser object.
	*/
	~AbccLogFileParser();

	/*******************************************************************************
	** @brief Checks if the log file is open.
	**
	** @retval True  - The log file is open.
	** @retval False - The log file is closed. The reason the log file is closed
	**                 may be due to the file not existing or access rights issues.
	*/
	bool IsOpen();

	/*******************************************************************************
	** @brief Get the next ABCC message from the log file.
	**
	** @param  message           - The parsed ABCC message.
	** @return MessageReturnType - Indicates the type of message (or event) parsed.
	*/
	MessageReturnType GetNextMessage(ABP_MsgType& message);

	/*******************************************************************************
	** @brief Get the Anybus Status
	**
	** @return ABP_AnbStateType - The last parsed Anybus status. If there have been
	**                            no parsed events this method will return the
	**                            default state specified during construction of
	**                            the object.
	*/
	ABP_AnbStateType GetAnbStatus();

private:

	/*
	** @brief The ABCC SDK log file stream.
	*/
	std::ifstream mLogFileStream;

	/*
	** @brief The ABCC SDK log file stream (for wide-char support).
	*/
	std::wifstream mLogFileWStream;

	/*
	** @brief The Anybus State.
	*/
	ABP_AnbStateType mAnbState;

	/*
	** @brief The encoding to use for parsing the log file.
	*/
	FileEncoding mEncoding;

	/*
	** @brief Indicates that the file stream endianness must be swapped for processing.
	*/
	bool mSwapEndianness;

	/*
	** @brief The line delimiter to use when reading a line via getline().
	*/
	wchar_t mLineDelimiter;

	/*******************************************************************************
	** @brief Attempts to detect whether the file encoding is UTF8, UTF16,
	**        and whether or not a BOM is present.
	*/
	void DetectFileEncoding();

	/*******************************************************************************
	** @brief Gets a line from the file stream.
	**
	** @param  line    - The line read from the stream.
	** @retval True    - A line was read and EOF has not been reached.
	** @retval False   - A line was not read.
	*/
	bool GetLine(std::string& line);

	/*******************************************************************************
	** @brief Parses an ABCC SDK log file message.
	**
	** @param  message - The parsed message.
	** @retval True    - Message was successfully parsed.
	** @retval False   - Message parsing error.
	*/
	bool ParseMessage(ABP_MsgType& message);

	/*******************************************************************************
	** @brief Parses the Anybus state from the specified line buffer.
	**
	** @param  line - The line buffer containing the Anybus state.
	** @return INT8 - The Anybus state. Returns a value < 0 on failure.
	*/
	INT8 ParseAnbState(const std::string& line);
};

#endif

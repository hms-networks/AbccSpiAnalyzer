/******************************************************************************
**  Copyright (C) 2015-2022 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzer.h
**    Summary: Shared types used across the analyzer sources.
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_ANALYZER_TYPES_H
#define ABCC_SPI_ANALYZER_TYPES_H
#include "Analyzer.h"

/* Indicates a SPI settings error (e.g. CPOL, CPHA, EN Active Hi/Lo) */
#define SPI_ERROR_FLAG						( 1 << 0 )

/* Direction flag. When asserted, MOSI, when de-asserted MISO */
#define SPI_MOSI_FLAG						( 1 << 1 )

/* Indicates the first message in a fragmented message transfer */
#define SPI_MSG_FIRST_FRAG_FLAG				( 1 << 2 )

/* Indicates that message fragmentation is in progress */
#define SPI_MSG_FRAG_FLAG					( 1 << 3 )

/* Event flag to indicate any critical events that are part of the ABCC SPI protocol
** This flag is field-specific.
** This flag is relevant for the following fields:
**   - SPI_CTL: signals a toggle error (retransmission event)
**   - ANB_STS: signals an Anybus status changed event
**   - SPI_STS: signals a toggle error (retransmission event)
**   - APP_STS: signals an application status changed event
**   - MD_SIZE: signals that the value in this field is out-of-spec
**   - CMD/RSP: signals an error response message
**   - CRC32: signals a checksum error */
#define SPI_PROTO_EVENT_FLAG				( 1 << 5 )

#define GET_MSG_FRAME_TAG(x)				(asMsgStates[static_cast<U32>(x)].tag)
#define GET_MOSI_FRAME_TAG(x)				(asMosiStates[x].tag)
#define GET_MISO_FRAME_TAG(x)				(asMisoStates[x].tag)

#define GET_MSG_FRAME_SIZE(x)				(asMsgStates[static_cast<U32>(x)].frameSize)
#define GET_MOSI_FRAME_SIZE(x)				(asMosiStates[x].frameSize)
#define GET_MISO_FRAME_SIZE(x)				(asMisoStates[x].frameSize)

#define GET_MSG_FRAME_BITSIZE(x)			((asMsgStates[static_cast<U32>(x)].frameSize)*8)
#define GET_MOSI_FRAME_BITSIZE(x)			((asMosiStates[x].frameSize)*8)
#define GET_MISO_FRAME_BITSIZE(x)			((asMisoStates[x].frameSize)*8)

enum class DisplayPriority : U32
{
	Value,
	Tag,
	SizeOfEnum
};

typedef enum NotifEvent {
	None,
	Alert
} NotifEvent_t;

/* Enum for indicating which SPI channel to operate on */
typedef enum SpiChannel {
	MOSI,
	MISO,
	NotSpecified
} SpiChannel_t;

typedef enum AttributeAccessMode {
	Normal,
	Indexed
} AttributeAccessMode_t;

/* Enum used as an extension to AbccMosiStates_t and AbccMisoStates_t */
typedef enum AbccSpiError
{
	Generic			= 0x80,
	Fragmentation	= 0x81,
	EndOfTransfer	= 0x82
} AbccSpiError_t;

namespace AbccMosiStates
{
	typedef enum
	{
		Idle,
		SpiControl,
		Reserved1,
		MessageLength,
		ProcessDataLength,
		ApplicationStatus,
		InterruptMask,

		// NOTE: Alignment of MOSI/MISO MessageField Enums must be maintained for shared logic to work.
		MessageField,
		MessageField_Size,
		MessageField_Reserved1,
		MessageField_SourceId,
		MessageField_Object,
		MessageField_Instance,
		MessageField_Command,
		MessageField_Reserved2,
		MessageField_CommandExtension,
		MessageField_Data,

		WriteProcessData,
		Crc32,
		Pad,
		MessageField_DataNotValid
	} Enum;
};

namespace AbccMisoStates
{
	typedef enum
	{
		Idle,
		Reserved1,
		Reserved2,
		LedStatus,
		AnybusStatus,
		SpiStatus,
		NetworkTime,

		// NOTE: Alignment of MOSI/MISO MessageField Enums must be maintained for shared logic to work.
		MessageField,
		MessageField_Size,
		MessageField_Reserved1,
		MessageField_SourceId,
		MessageField_Object,
		MessageField_Instance,
		MessageField_Command,
		MessageField_Reserved2,
		MessageField_CommandExtension,
		MessageField_Data,

		ReadProcessData,
		Crc32,
		MessageField_DataNotValid
	} Enum;
};

enum class AbccMsgField
{
	Size,
	Reserved1,
	SourceId,
	Object,
	Instance,
	Command,
	Reserved2,
	CommandExtension,
	Data
};

typedef struct AbccMsgInfo
{
	AbccMsgField eMsgState;
	const char* tag;
	U8 frameSize;
} AbccMsgInfo_t;

typedef struct AbccMosiInfo
{
	AbccMosiStates::Enum eMosiState;
	const char* tag;
	U8 frameSize;
} AbccMosiInfo_t;

typedef struct AbccMisoInfo
{
	AbccMisoStates::Enum eMisoState;
	const char* tag;
	U8 frameSize;
} AbccMisoInfo_t;

typedef struct MsgHeaderInfo
{
	U8  cmd;
	U8  obj;
	U16 inst;
	U16 cmdExt;
} MsgHeaderInfo_t;

typedef struct NetworkTimeInfo
{
	U32 deltaTime;
	U16 pad;
	bool newRdPd;
	bool wrPdValid;
} NetworkTimeInfo_t;

typedef struct MsgDataFrameData2
{
	U16 msgDataCnt;
	MsgHeaderInfo_t msgHeader;
} MsgDataFrameData2_t;

#endif /* ABCC_SPI_ANALYZER_TYPES_H */

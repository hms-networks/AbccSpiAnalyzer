/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzer.h
**    Summary: DLL-Analyzer header
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_ANALYZER_H
#define ABCC_SPI_ANALYZER_H

#include <stdio.h>
#include <Analyzer.h>
#include "AbccSpiAnalyzerResults.h"
#include "AbccSpiSimulationDataGenerator.h"
#include "AbccCrc.h"

#ifdef _WIN32
#define SNPRINTF sprintf_s
#else
#define SNPRINTF snprintf
#endif

#define FORMATTED_STRING_BUFFER_SIZE		256
#define DISPLAY_NUMERIC_STRING_BUFFER_SIZE	128

#define MIN_IDLE_GAP_TIME					10.0e-6f
#define MAX_CLOCK_IDLE_HI_TIME				5.0e-6f

#define ABCC_STATUS_RESERVED_MASK			0xF0
#define ABCC_STATUS_SUP_MASK				0x08
#define ABCC_STATUS_CODE_MASK				0x07

/* Macros to access information stored in arrays */
#define ABCC_MOSI_CHANNEL 0
#define ABCC_MISO_CHANNEL 1

#define GET_MSG_FRAME_TAG(x)			(asMsgStates[x].tag)
#define GET_MOSI_FRAME_TAG(x)			(asMosiStates[x].tag)
#define GET_MISO_FRAME_TAG(x)			(asMisoStates[x].tag)

#define GET_MSG_FRAME_SIZE(x)			(asMsgStates[x].frameSize)
#define GET_MOSI_FRAME_SIZE(x)			(asMosiStates[x].frameSize)
#define GET_MISO_FRAME_SIZE(x)			(asMisoStates[x].frameSize)

#define GET_MSG_FRAME_BITSIZE(x)		((asMsgStates[x].frameSize)*8)
#define GET_MOSI_FRAME_BITSIZE(x)		((asMosiStates[x].frameSize)*8)
#define GET_MISO_FRAME_BITSIZE(x)		((asMisoStates[x].frameSize)*8)

typedef enum tGetWordStatus
{
	e_GET_WORD_OK,		/* WORD was successfully read */
	e_GET_WORD_ERROR,	/* Reading WORD resulted in a logical error (requires statemachine reset) */
	e_GET_WORD_RESET	/* Reading WORD resulted in a event that requires state machine reset */
}tGetWordStatus;

typedef enum tPacketType
{
	e_NULL_PACKET,
	e_COMMAND_PACKET,
	e_RESPONSE_PACKET,
	e_MSG_FRAGMENT_PACKET,
	e_ERROR_RESPONSE_PACKET,
	e_PROTOCOL_ERROR_PACKET,
	e_MULTI_PACKET
}tPacketType;

typedef enum tAbccMosiStates
{
	e_ABCC_MOSI_IDLE,
	e_ABCC_MOSI_SPI_CTRL,
	e_ABCC_MOSI_RESERVED1,
	e_ABCC_MOSI_MSG_LEN,
	e_ABCC_MOSI_PD_LEN,
	e_ABCC_MOSI_APP_STAT,
	e_ABCC_MOSI_INT_MASK,
	e_ABCC_MOSI_WR_MSG_FIELD,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_size,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_res1,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_obj,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_inst,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_res2,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_data,
	e_ABCC_MOSI_WR_PD_FIELD,
	e_ABCC_MOSI_CRC32,
	e_ABCC_MOSI_PAD,
	e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid
}tAbccMosiStates;

typedef enum tAbccMisoStates
{
	e_ABCC_MISO_IDLE,
	e_ABCC_MISO_Reserved1,
	e_ABCC_MISO_Reserved2,
	e_ABCC_MISO_LED_STAT,
	e_ABCC_MISO_ANB_STAT,
	e_ABCC_MISO_SPI_STAT,
	e_ABCC_MISO_NET_TIME,
	e_ABCC_MISO_RD_MSG_FIELD,
	e_ABCC_MISO_RD_MSG_SUBFIELD_size,
	e_ABCC_MISO_RD_MSG_SUBFIELD_res1,
	e_ABCC_MISO_RD_MSG_SUBFIELD_srcId,
	e_ABCC_MISO_RD_MSG_SUBFIELD_obj,
	e_ABCC_MISO_RD_MSG_SUBFIELD_inst,
	e_ABCC_MISO_RD_MSG_SUBFIELD_cmd,
	e_ABCC_MISO_RD_MSG_SUBFIELD_res2,
	e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt,
	e_ABCC_MISO_RD_MSG_SUBFIELD_data,
	e_ABCC_MISO_RD_PD_FIELD,
	e_ABCC_MISO_CRC32,
	e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid
}tAbccMisoStates;

typedef enum tAbccSpiError
{
	e_ABCC_SPI_ERROR_GENERIC			= 0x80,
	e_ABCC_SPI_ERROR_FRAGMENTATION		= 0x81,
	e_ABCC_SPI_ERROR_END_OF_TRANSFER	= 0x82
}tAbccSpiError;

typedef union uAbccSpiStates
{
	U8				bState;
	tAbccMisoStates	eMiso;
	tAbccMosiStates	eMosi;
}uAbccSpiStates;

typedef enum tAbccMsgField
{
	e_ABCC_MSG_SIZE,
	e_ABCC_MSG_RESERVED1,
	e_ABCC_MSG_SOURCE_ID,
	e_ABCC_MSG_OBJECT,
	e_ABCC_MSG_INST,
	e_ABCC_MSG_CMD,
	e_ABCC_MSG_RESERVED2,
	e_ABCC_MSG_CMD_EXT,
	e_ABCC_MSG_DATA
}tAbccMsgField;

//TODO joca consider adding a "full name" element here that would be used for printing to tabular text
typedef struct tAbccMosiInfo
{
	tAbccMosiStates eMosiState;
	char* tag;
	U8 frameSize;
}tAbccMosiInfo;

typedef struct tAbccMisoInfo
{
	tAbccMisoStates eMisoState;
	char* tag;
	U8 frameSize;
}tAbccMisoInfo;

typedef struct tAbccMsgInfo
{
	tAbccMsgField eMsgState;
	char* tag;
	U8 frameSize;
}tAbccMsgInfo;

typedef struct tValueName
{
	U16		value;
	char*	name;
	bool	alert;
}tValueName;

typedef struct tMsgHeaderInfo
{
	U8  cmd;
	U8  obj;
	U16 inst;
	U16 cmdExt;
}tMsgHeaderInfo;

typedef struct tNetworkTimeInfo
{
	U32 deltaTime;
	U16 pad;
	bool newRdPd;
	bool wrPdValid;
}tNetworkTimeInfo;

extern const tAbccMosiInfo asMosiStates[];
extern const tAbccMisoInfo asMisoStates[];
extern const tAbccMsgInfo asMsgStates[];

class SpiAnalyzerSettings;
#if _DEBUG
class SpiAnalyzer : public Analyzer2
#else
class ANALYZER_EXPORT SpiAnalyzer : public Analyzer2
#endif
{
public:
	SpiAnalyzer();
	virtual ~SpiAnalyzer();
	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels);
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: /* functions */
	void Setup();
	void AdvanceToActiveEnableEdge();
	void AdvanceToActiveEnableEdgeWithCorrectClockPolarity();

	bool IsEnableActive(void);
	bool IsInitialClockPolarityCorrect();

	bool WouldAdvancingTheClockToggleEnable();

	tGetWordStatus GetWord(U64* plMosiData, U64* plMisoData, U64* plFirstSample);

	void CheckForIdleAfterPacket(void);
	void AddFragFrame(bool fMosi, U64 lFirstSample, U64 lLastSample);
	void SignalReadyForNewPacket(bool fMosiChannel, tPacketType ePacketType);

	void ProcessMosiFrame(tAbccMosiStates eMosiState, U64 lFrameData, S64 lFramesFirstSample);
	void ProcessMisoFrame(tAbccMisoStates eMisoState, U64 lFrameData, S64 lFramesFirstSample);

	bool RunAbccMosiStateMachine(bool fReset, bool fError, U64 lMosiData, S64 lFirstSample);
	bool RunAbccMisoStateMachine(bool fReset, bool fError, U64 lMisoData, S64 lFirstSample);

	bool RunAbccMisoMsgSubStateMachine(bool fReset, bool* pfAddFrame, tAbccMisoStates* peMisoMsgSubState);
	bool RunAbccMosiMsgSubStateMachine(bool fReset, bool* pfAddFrame, tAbccMosiStates* peMosiMsgSubState);

	bool Is3WireIdleCondition(float rIdleTimeCondition);


#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class
protected: /* variables */
	std::auto_ptr< SpiAnalyzerSettings > mSettings;
	std::auto_ptr< SpiAnalyzerResults > mResults;
	bool mSimulationInitialized;
	SpiSimulationDataGenerator mSimulationDataGenerator;

	AnalyzerChannelData* mMosi;
	AnalyzerChannelData* mMiso;
	AnalyzerChannelData* mClock;
	AnalyzerChannelData* mEnable;

	U64 mCurrentSample;
	std::vector<U64> mArrowLocations;

	tAbccMosiStates eMosiState;
	tAbccMisoStates eMisoState;

	AbccCrc mMisoChecksum;
	AbccCrc mMosiChecksum;

	U32 dwMisoPdLen;
	U32 dwMosiPdLen;

	U32 dwMisoMsgLen;
	U32 dwMosiMsgLen;

	bool fMisoNewMsg;
	bool fMosiNewMsg;

	bool fMisoErrorRsp;
	bool fMosiErrorRsp;

	bool fMisoFragmentation;
	bool fMosiFragmentation;

	bool fMisoFirstFrag;
	bool fMosiFirstFrag;

	bool fMisoLastFrag;
	bool fMosiLastFrag;

	bool fMisoNewRdPd;
	bool fMosiWrPdValid;

	U8 bSettingsChangeID;

	bool fMisoReadyForNewPacket;
	bool fMosiReadyForNewPacket;
	tPacketType eMisoPacketType;
	tPacketType eMosiPacketType;

	tMsgHeaderInfo sMisoMsgHeader;
	U32 dwMisoPdCnt;
	U32 dwMisoMdCnt;
	U16 wMisoMdSize;

	tMsgHeaderInfo sMosiMsgHeader;
	U32 dwMosiPdCnt;
	U32 dwMosiMdCnt;
	U16 wMosiMdSize;

	U8 bLastAnbSts;
	U8 bLastApplSts;
	U8 bMosiLastToggleState;

	U32 dwLastMisoTimestamp;

	tAbccMisoStates eMisoMsgSubState;
	U32 dwMisoByteCnt;
	U64 lMisoFrameData;
	S64 lMisoFramesFirstSample;

	tAbccMosiStates eMosiMsgSubState;
	U32 dwMosiByteCnt;
	U64 lMosiFrameData;
	S64 lMosiFramesFirstSample;

	U8 bMisoByteCnt2;
	U8 bMosiByteCnt2;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif /* SPI_ANALYZER_H */

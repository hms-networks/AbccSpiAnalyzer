/******************************************************************************
**  Copyright (C) 2015-2022 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzer.h
**    Summary: Responsible for processing the raw samples and converting the
**             sampled data into frame and packets.
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_ANALYZER_H
#define ABCC_SPI_ANALYZER_H

#include <stdio.h>

#include "Analyzer.h"
#include "AbccSpiAnalyzerTypes.h"
#include "AbccSpiAnalyzerResults.h"
#include "AbccSpiSimulationDataGenerator.h"
#include "AbccCrc.h"

#ifdef _WIN32
#define SNPRINTF sprintf_s
#else
#define SNPRINTF snprintf
#endif

#define SIZE_IN_BITS(var)					( sizeof(var) * 8 )

#define FORMATTED_STRING_BUFFER_SIZE		256
#define DISPLAY_NUMERIC_STRING_BUFFER_SIZE	128

#define MIN_IDLE_GAP_TIME					10.0e-6f
#define MAX_CLOCK_IDLE_HI_TIME				5.0e-6f

#define ABCC_STATUS_RESERVED_MASK			0xF0
#define ABCC_STATUS_SUP_MASK				0x08
#define ABCC_STATUS_CODE_MASK				0x07

enum class GetByteStatus : U32
{
	OK,		// BYTE was successfully read
	Error,	// Reading BYTE resulted in a logical error (requires statemachine reset)
	Reset,	// Reading BYTE resulted in a event that requires state machine reset
	Skip,	// Enable line was toggle with no data clocked. It is an empty packet and can be skipped.
	SizeOfEnum
};

// Enum for indicating when to reset a statemachine
enum class StateOperation : U32
{
	Run,
	Reset,
	SizeOfEnum
};

// Enum for indicating whether or not the analyzer succeeded in acquiring an SPI byte
enum class AcquisitionStatus : U32
{
	OK,
	Reset,
	Error,
	SizeOfEnum
};

enum class PacketType : U32
{
	Empty,
	Command,
	Response,
	MessageFragment,
	ErrorResponse,
	ProtocolError,
	ProtocolEvent,
	ChecksumError,
	MultiEvent,
	MultiEventWithError,
	Cancel,
	SizeOfEnum
};

extern const AbccMosiInfo_t asMosiStates[];
extern const AbccMisoInfo_t asMisoStates[];
extern const AbccMsgInfo_t asMsgStates[];

class SpiAnalyzerSettings;
#ifdef _DEBUG
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

protected: /* Enums, Classes, Types */

	typedef struct MosiVars
	{
		S64 lFramesFirstSample;
		U64 lFrameData;
		PacketType ePacketType;
		AbccMosiStates::Enum eState;
		AbccMosiStates::Enum eMsgSubState;
		MsgHeaderInfo_t sMsgHeader;
		AbccCrc oChecksum;
		U32 dwPdLen;
		U32 dwPdCnt;
		U32 dwMsgLen;
		U32 dwMsgLenCnt;
		U32 dwByteCnt;
		U8 bFrameSizeCnt;
		U16 wMdCnt;
		U16 wMdSize;
		U8 bLastToggleState;
		U8 bLastApplSts;
		bool fNewMsg;
		bool fErrorRsp;
		bool fFragmentation;
		bool fFirstFrag;
		bool fLastFrag;
		bool fWrPdValid;
		bool fReadyForNewPacket;
	} MosiVars_t;

	typedef struct MisoVars
	{
		S64 lFramesFirstSample;
		U64 lFrameData;
		PacketType ePacketType;
		AbccMisoStates::Enum eState;
		AbccMisoStates::Enum eMsgSubState;
		MsgHeaderInfo_t sMsgHeader;
		AbccCrc oChecksum;
		U32 dwLastTimestamp;
		U32 dwPdLen;
		U32 dwPdCnt;
		U32 dwMsgLen;
		U32 dwMsgLenCnt;
		U32 dwByteCnt;
		U8 bFrameSizeCnt;
		U16 wMdCnt;
		U16 wMdSize;
		U8 bLastAnbSts;
		bool fNewMsg;
		bool fErrorRsp;
		bool fFragmentation;
		bool fFirstFrag;
		bool fLastFrag;
		bool fNewRdPd;
		bool fReadyForNewPacket;
	} MisoVars_t;

#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'SpiAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

protected: // Members

	std::unique_ptr<SpiAnalyzerSettings> mSettings;
	std::unique_ptr<SpiAnalyzerResults> mResults;

	SpiSimulationDataGenerator mSimulationDataGenerator;

	AnalyzerChannelData* mMosi;
	AnalyzerChannelData* mMiso;
	AnalyzerChannelData* mClock;
	AnalyzerChannelData* mEnable;

	U64 mCurrentSample;
	S32 mClockingErrorCount;
	std::vector<U64> mArrowLocations;
	U8 mSettingsChangeID;

	MosiVars_t mMosiVars;
	MisoVars_t mMisoVars;

	// Backup variables to recover from error cases
	// that require knowledge of the last valid state
	MosiVars_t mPreviousMosiVars;
	MisoVars_t mPreviousMisoVars;

	bool mSimulationInitialized;

#pragma warning( pop )

protected: // Methods

	inline void ProcessSample(AnalyzerChannelData* chn_data, DataBuilder& data, Channel& chn);

	void Setup();
	void AdvanceToActiveEnableEdge();
	void AdvanceToActiveEnableEdgeWithCorrectClockPolarity();

	bool IsEnableActive();
	bool IsInitialClockPolarityCorrect();

	bool WouldAdvancingTheClockToggleEnable();

	GetByteStatus GetByte(U64* mosi_data_ptr, U64* miso_data_ptr, U64* first_sample_ptr);

	void CheckForIdleAfterPacket();
	void AddFragFrame(SpiChannel_t channel, U64 first_sample, U64 last_sample);
	void SignalReadyForNewPacket(SpiChannel_t channel);

	void SetMosiPacketType(PacketType packet_type);
	void SetMisoPacketType(PacketType packet_type);
	AnalyzerResults::MarkerType GetPacketMarkerType();

	void ProcessMosiFrame(AbccMosiStates::Enum state, U64 frame_data, S64 frames_first_sample);
	void ProcessMisoFrame(AbccMisoStates::Enum state, U64 frame_data, S64 frames_first_sample);

	bool RunAbccMosiStateMachine(StateOperation operation, AcquisitionStatus acquisition_status, U64 mosi_data, S64 first_sample);
	bool RunAbccMisoStateMachine(StateOperation operation, AcquisitionStatus acquisition_status, U64 miso_data, S64 first_sample);

	bool RunAbccMisoMsgSubStateMachine(StateOperation operation, bool* add_frame_ptr, AbccMisoStates::Enum* substate_ptr);
	bool RunAbccMosiMsgSubStateMachine(StateOperation operation, bool* add_frame_ptr, AbccMosiStates::Enum* substate_ptr);

	bool Is3WireIdleCondition(float idle_time_condition);
	void RestorePreviousStateVars();
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif /* SPI_ANALYZER_H */

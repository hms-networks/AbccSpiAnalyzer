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

#ifdef _WIN32
#define SNPRINTF sprintf_s
#else
#define SNPRINTF snprintf
#endif

#define FORMATTED_STRING_BUFFER_SIZE		256
#define DISPLAY_NUMERIC_STRING_BUFFER_SIZE	128

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

#define ABCC_MSG_SIZE_FIELD_SIZE		2
#define ABCC_MSG_RES1_FIELD_SIZE		2
#define ABCC_MSG_SRC_ID_FIELD_SIZE		1
#define ABCC_MSG_OBJ_FIELD_SIZE			1
#define ABCC_MSG_INST_FIELD_SIZE		2
#define ABCC_MSG_CMD_FIELD_SIZE			1
#define ABCC_MSG_RES2_FIELD_SIZE		1
#define ABCC_MSG_CMDEXT_FIELD_SIZE		2
#define ABCC_MSG_CMDEXT0_FIELD_SIZE		1
#define ABCC_MSG_CMDEXT1_FIELD_SIZE		1
#define ABCC_MSG_DATA_FIELD_SIZE		1

typedef enum tGetWordStatus
{
	e_GET_WORD_OK,		/* WORD was successfully read */
	e_GET_WORD_ERROR,	/* Reading WORD resulted in a logical error (requires statemachine reset) */
	e_GET_WORD_RESET	/* Reading WORD resulted in a event that requires state machine reset */
}tGetWordStatus;

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

extern tAbccMosiInfo asMosiStates[];
extern tAbccMisoInfo asMisoStates[];
extern tAbccMsgInfo asMsgStates[];

class SpiAnalyzerSettings;
class ANALYZER_EXPORT SpiAnalyzer : public Analyzer2
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

	void AddFragFrame(bool fMosi, U8 bState, U64 lFirstSample, U64 lLastSample);
	void SignalReadyForNewPacket(bool fMosiChannel, bool fErrorPacket);

	void ProcessMosiFrame(tAbccMosiStates eMosiState, U64 lFrameData, S64 lFramesFirstSample);
	void ProcessMisoFrame(tAbccMisoStates eMisoState, U64 lFrameData, S64 lFramesFirstSample);

	bool RunAbccMosiStateMachine(bool fReset, bool fError, U64 lMosiData, S64 lFirstSample);
	bool RunAbccMisoStateMachine(bool fReset, bool fError, U64 lMisoData, S64 lFirstSample);

	bool RunAbccMisoMsgSubStateMachine(bool fReset, bool* pfAddFrame, tAbccMisoStates* peMisoMsgSubState);
	bool RunAbccMosiMsgSubStateMachine(bool fReset, bool* pfAddFrame, tAbccMosiStates* peMosiMsgSubState);


#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class
protected: /* variables */
	std::auto_ptr< SpiAnalyzerSettings > mSettings;
	std::auto_ptr< SpiAnalyzerResults > mResults;
	bool mSimulationInitilized;
	SpiSimulationDataGenerator mSimulationDataGenerator;

	AnalyzerChannelData* mMosi;
	AnalyzerChannelData* mMiso;
	AnalyzerChannelData* mClock;
	AnalyzerChannelData* mEnable;

	U64 mCurrentSample;
	AnalyzerResults::MarkerType mArrowMarker;
	std::vector<U64> mArrowLocations;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif /* SPI_ANALYZER_H */

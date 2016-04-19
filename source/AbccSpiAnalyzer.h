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

#ifndef SPI_ANALYZER_H
#define SPI_ANALYZER_H

#include <stdio.h>
#include <Analyzer.h>
#include "AbccSpiAnalyzerResults.h"
#include "AbccSpiSimulationDataGenerator.h"

#ifdef _WIN32
	#define SNPRINTF sprintf_s 
#else
	#define SNPRINTF snprintf
#endif

typedef enum
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
	e_ABCC_MOSI_PAD
}t_ABCC_MOSI;

typedef enum
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
	e_ABCC_MISO_CRC32
}t_ABCC_MISO;


typedef enum
{
	e_ABCC_MSG_SIZE,
	e_ABCC_RESERVED1,
	e_ABCC_SOURCE_ID,
	e_ABCC_OBJECT,
	e_ABCC_INST,
	e_ABCC_SPECIFIER,
    e_ABCC_RESERVED2,
	e_ABCC_CMD_EXT_0,
	e_ABCC_CMD_EXT_1,
	e_ABCC_DATA	
}t_ABCC_MSG_FIELD;

class SpiAnalyzerSettings;
class ANALYZER_EXPORT SpiAnalyzer : public Analyzer2
{
public:
	SpiAnalyzer();
	virtual ~SpiAnalyzer();
	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //functions
	void Setup();
	void AdvanceToActiveEnableEdge();
	void AdvanceToActiveEnableEdgeWithCorrectClockPolarity();

	bool IsEnableActive(void);
	bool IsInitialClockPolarityCorrect();
		
	bool WouldAdvancingTheClockToggleEnable();
	
	bool GetWord(U64* plMosiData, U64* plMisoData, U64* plFirstSample);
	

	U32 CRC_Crc32(U32 iInitCrc, U8* pbBufferStart, U16 iLength);
	U32 CRC_FormatCrc32(U32 lCrc);

	void AddFragFrame(bool fMosi, U8 bState, U64 lFirstSample, U64 lLastSample);

	void ProcessMosiFrame(t_ABCC_MOSI eMosiState, U64 lFrameData, S64 lFramesFirstSample);
	void ProcessMisoFrame(t_ABCC_MISO eMisoState, U64 lFrameData, S64 lFramesFirstSample);
	
	bool RunAbccMosiStateMachine(bool fReset, bool fError, U64 lMosiData, S64 lFirstSample);
	bool RunAbccMisoStateMachine(bool fReset, bool fError, U64 lMisoData, S64 lFirstSample);
	
	bool RunAbccMisoMsgSubStateMachine(bool fReset, bool* pfAddFrame, t_ABCC_MISO *peMisoMsgSubState);
	bool RunAbccMosiMsgSubStateMachine(bool fReset, bool* pfAddFrame, t_ABCC_MOSI *peMosiMsgSubState);

	
#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class
protected: //vars
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
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //SPI_ANALYZER_H

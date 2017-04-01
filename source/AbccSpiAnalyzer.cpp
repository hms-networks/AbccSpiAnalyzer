/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzer.cpp
**    Summary: DLL-Analyzer source
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include "AbccSpiAnalyzer.h"
#include "AbccSpiAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include "AbccCrc.h"

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#define PROCESS_SAMPLE(x,y,z) \
		if( x != NULL ) \
		{ \
			x->AdvanceToAbsPosition( mCurrentSample ); \
			y.AddBit( x->GetBitState() ); \
			if (x->GetBitState() == BIT_HIGH) \
			{ \
				mResults->AddMarker(mCurrentSample, AnalyzerResults::One, mSettings->z); \
			} \
			else \
			{ \
				mResults->AddMarker(mCurrentSample, AnalyzerResults::Zero, mSettings->z); \
			} \
		}

SpiAnalyzer::SpiAnalyzer()
	: Analyzer2(),
	mSettings(new SpiAnalyzerSettings()),
	mSimulationInitialized(false),
	mMosi(NULL),
	mMiso(NULL),
	mClock(NULL),
	mEnable(NULL)
{
	SetAnalyzerSettings(mSettings.get());

	bSettingsChangeID   = mSettings->mChangeID;

	eMosiState              = e_ABCC_MOSI_IDLE;
	eMisoState              = e_ABCC_MISO_IDLE;
	bLastAnbSts             = 0xFF;
	bLastApplSts            = 0xFF;
	bMosiLastToggleState    = 0xFF;
	dwMosiMsgLen            = 0;
	dwMosiPdLen             = 0;
	dwMisoMsgLen            = 0;
	dwMisoPdLen             = 0;

	fMisoNewMsg             = false;
	fMisoErrorRsp           = true;
	fMisoFragmentation      = false;
	fMisoFirstFrag          = false;
	fMisoLastFrag           = false;
	fMisoNewRdPd            = false;
	fMosiNewMsg             = false;
	fMosiErrorRsp           = true;
	fMosiFragmentation      = false;
	fMosiFirstFrag          = false;
	fMosiLastFrag           = false;
	fMosiWrPdValid          = false;

	fMisoReadyForNewPacket  = false;
	fMosiReadyForNewPacket  = false;

	memset(&sMisoMsgHeader, 0, sizeof(sMisoMsgHeader));
	dwMisoPdCnt = 0;
	dwMisoMdCnt = 0;
	wMisoMdSize = 0;

	memset(&sMosiMsgHeader, 0, sizeof(sMosiMsgHeader));
	dwMosiPdCnt = 0;
	dwMosiMdCnt = 0;
	wMosiMdSize = 0;

	dwLastMisoTimestamp = 0;

	eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
	dwMisoByteCnt    = 0;
	lMisoFrameData   = 0;

	eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
	dwMosiByteCnt    = 0;
	lMosiFrameData   = 0;

	bMosiByteCnt2 = 0;
	bMisoByteCnt2 = 0;
}

SpiAnalyzer::~SpiAnalyzer()
{
	KillThread();
}

void SpiAnalyzer::SetupResults()
{
	mResults.reset(new SpiAnalyzerResults(this, mSettings.get()));
	SetAnalyzerResults(mResults.get());

	if (mSettings->mMosiChannel != UNDEFINED_CHANNEL)
	{
		mResults->AddChannelBubblesWillAppearOn(mSettings->mMosiChannel);
	}
	if (mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mResults->AddChannelBubblesWillAppearOn(mSettings->mMisoChannel);
	}
}

void SpiAnalyzer::WorkerThread()
{
	U64 lMosiData;
	U64 lMisoData;
	U64 lFirstSample;
	tGetWordStatus eWordStatus;
	bool fReset;
	bool fError;
	bool fReady1 = true;
	bool fReady2 = true;
	Setup();

	/* Check that all required channels are valid */
	if ( (mMiso != NULL) && (mMosi != NULL) && (mClock != NULL) )
	{
		mMisoChecksum = AbccCrc();
		mMosiChecksum = AbccCrc();

		AdvanceToActiveEnableEdgeWithCorrectClockPolarity();

		/* Reset persistent state logic between captures */
		eMosiState = e_ABCC_MOSI_IDLE;
		eMisoState = e_ABCC_MISO_IDLE;
		bLastAnbSts = 0xFF;
		bLastApplSts = 0xFF;

		RunAbccMosiMsgSubStateMachine(true, NULL, NULL);
		RunAbccMisoMsgSubStateMachine(true, NULL, NULL);

		for (;;)
		{
			/* The SPI word length is 8-bits. Read 1 byte at a time and run the statemachines */
			eWordStatus = GetByte(&lMosiData, &lMisoData, &lFirstSample);
			switch (eWordStatus)
			{
			case e_GET_WORD_OK:
				fError = false;
				fReset = false;
				break;
			case e_GET_WORD_RESET:
				fError = false;
				fReset = true;
				break;
			default:
			case e_GET_WORD_ERROR:
				fError = true;
				fReset = true;
				break;
			}

			/* Run the ABCC MOSI state machine */
			fReady1 = RunAbccMosiStateMachine((fReset||fReady1), fError, lMosiData, lFirstSample);

			/* Run the ABCC MISO state machine */
			fReady2 = RunAbccMisoStateMachine((fReset||fReady2), fError, lMisoData, lFirstSample);

			if (mEnable == NULL)
			{
				if (!fReady1 && !fReady2)
				{
					if (Is3WireIdleCondition(MAX_CLOCK_IDLE_HI_TIME))
					{
						eMosiState = e_ABCC_MOSI_SPI_CTRL;
						eMisoState = e_ABCC_MISO_Reserved1;
						eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
						eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
						mMisoChecksum.Init();
						mMosiChecksum.Init();
						lMisoFrameData = 0;
						lMosiFrameData = 0;
						dwMisoByteCnt = 0;
						dwMosiByteCnt = 0;
						fError = true;
					}
				}
			}

			if (fError == true)
			{
				/* Signal error, do not commit packet */
				SignalReadyForNewPacket(false, e_PROTOCOL_ERROR_PACKET);

				/* Advance to the next enabled (or idle) state */
				if (mEnable != NULL)
				{
					AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				}
				else
				{
					AddFragFrame(false, lMisoFramesFirstSample, mClock->GetSampleOfNextEdge());
					AddFragFrame(true,  lMosiFramesFirstSample, mClock->GetSampleOfNextEdge());
					AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				}
			}

			mResults->CommitResults();
			ReportProgress(mClock->GetSampleNumber());

			CheckIfThreadShouldExit();
		}
	}
}

void SpiAnalyzer::AdvanceToActiveEnableEdgeWithCorrectClockPolarity()
{
	/* NOTE: 3-wire requires correct clock polarity (IDLE HI),
	** in 4-wire mode, the polarity does not matter. */

	AdvanceToActiveEnableEdge();

	if (mEnable == NULL)
	{
		/* With no enable line an idle gap of at least >=10us is required */
		for (;;)
		{
			/* First find idle gap */
			while (!Is3WireIdleCondition(MIN_IDLE_GAP_TIME))
			{
				mClock->AdvanceToNextEdge();
			}

			mCurrentSample = mClock->GetSampleNumber();
			/* If false, this function moves to the next enable-active edge. */
			if (IsInitialClockPolarityCorrect() == true)
			{
				break;
			}
			else
			{
				mClock->AdvanceToNextEdge();
			}
		}
	}
}

void SpiAnalyzer::Setup()
{
	if (mSettings->mMosiChannel != UNDEFINED_CHANNEL)
	{
		mMosi = GetAnalyzerChannelData(mSettings->mMosiChannel);
	}
	else
	{
		mMosi = NULL;
	}

	if (mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mMiso = GetAnalyzerChannelData(mSettings->mMisoChannel);
	}
	else
	{
		mMiso = NULL;
	}

	if (mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
	}
	else
	{
		mClock = NULL;
	}

	if (mSettings->mEnableChannel != UNDEFINED_CHANNEL)
	{
		mEnable = GetAnalyzerChannelData(mSettings->mEnableChannel);
	}
	else
	{
		mEnable = NULL;
	}

}

void SpiAnalyzer::AdvanceToActiveEnableEdge()
{
	if (mEnable != NULL)
	{
		if (mEnable->GetBitState() != BIT_LOW)
		{
			mEnable->AdvanceToNextEdge();
		}
		else
		{
			mEnable->AdvanceToNextEdge();
			mEnable->AdvanceToNextEdge();
		}
		mCurrentSample = mEnable->GetSampleNumber();
		mClock->AdvanceToAbsPosition(mCurrentSample);
	}
	else
	{
		mCurrentSample = mClock->GetSampleNumber();
	}
}

bool SpiAnalyzer::IsInitialClockPolarityCorrect()
{
	bool fCorrect = true;
	if (mEnable == NULL)
	{
		/* In 3-wire clock must idle HIGH */
		if (mClock->GetBitState() == BIT_LOW)
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::ErrorSquare, mSettings->mClockChannel);
			fCorrect = false;
		}
	}
	return fCorrect;
}

bool SpiAnalyzer::WouldAdvancingTheClockToggleEnable()
{
	if (mEnable == NULL)
	{
		return false;
	}

	U64 next_edge = mClock->GetSampleOfNextEdge();
	bool enable_will_toggle = mEnable->WouldAdvancingToAbsPositionCauseTransition(next_edge);

	if (enable_will_toggle == false)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool SpiAnalyzer::Is3WireIdleCondition(float rIdleTimeCondition)
{
	UINT64 lSampleDistance = mClock->GetSampleOfNextEdge() - mClock->GetSampleNumber();
	U32 dwSampleRate = GetSampleRate();
	float rIdleTime = (float)lSampleDistance / (float)dwSampleRate;
	return (rIdleTime >= rIdleTimeCondition);
}

tGetWordStatus SpiAnalyzer::GetByte(U64* plMosiData, U64* plMisoData, U64* plFirstSample)
{
	/* We're assuming we come into this function with the clock in the idle state */
	const U32 dwBitsPerTransfer = 8;
	const AnalyzerResults::MarkerType mArrowMarker = AnalyzerResults::UpArrow;
	DataBuilder mosi_result;
	DataBuilder miso_result;
	tGetWordStatus status = e_GET_WORD_OK;
	bool fClkIdleHigh = false;

	mosi_result.Reset(plMosiData, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);
	miso_result.Reset(plMisoData, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);
	mArrowLocations.clear();

	*plFirstSample = mClock->GetSampleNumber();

	if (mClock->GetBitState() == BIT_HIGH)
	{
		fClkIdleHigh = true;
	}

	for (U32 i = 0; i < dwBitsPerTransfer; i++)
	{
		/* On every single edge, we need to check that "enable" doesn't toggle. */
		/* Note that we can't just advance the enable line to the next edge, because there may not be another edge */

		if (WouldAdvancingTheClockToggleEnable() == true)
		{
			if (i == 0)
			{
				/* Simply advance forward to next transaction */
				AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
			}
			else
			{
				/* Reset everything and return. */
				status = e_GET_WORD_RESET;
				break;
			}
		}

		/* For CLOCK IDLE LOW configurations, skip advancing the clock when sampling the first bit. */
		if (mEnable == NULL)
		{
			/* In 3-wire mode, idle condition is >=5us (during a transaction).
			** On every advancement on clock, check for idle condition.
			** If detected, a reset of the statemachines are need to re-sync */
			if (Is3WireIdleCondition(MAX_CLOCK_IDLE_HI_TIME))
			{
				if (i == 0)
				{
					/* Simply advance forward to next transaction */
					AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				}
				else
				{
					/* Reset everything and return. */
					status = e_GET_WORD_ERROR;
					break;
				}
			}
		}

		/* Jump to the next clock phase */
		mClock->AdvanceToNextEdge();

		if(!fClkIdleHigh)
		{
			/* Sample on leading edge */
			mCurrentSample = mClock->GetSampleNumber();
			PROCESS_SAMPLE(mMosi, mosi_result, mMosiChannel);
			PROCESS_SAMPLE(mMiso, miso_result, mMisoChannel);
		}

		if (i == 0)
		{
			/* Latch the first sample point in the byte */
			*plFirstSample = mClock->GetSampleNumber();
		}

		if (WouldAdvancingTheClockToggleEnable() == true)
		{
			/* Error: reset everything and return. */
			status = e_GET_WORD_ERROR;
			break;
		}

		if (mEnable == NULL)
		{
			/* In 3-wire mode idle condition is >=5us (during a transaction).
			** On every advancement on clock, check for idle condition.
			** If detected, a reset of the statemachines are need to re-sync */
			if (Is3WireIdleCondition(MAX_CLOCK_IDLE_HI_TIME))
			{
				/* Error: reset everything and return. */
				status = e_GET_WORD_ERROR;
				break;
			}
		}

		/* Jump to the next clock phase */
		mClock->AdvanceToNextEdge();

		if(fClkIdleHigh)
		{
			/* Sample on tailing edge */
			mCurrentSample = mClock->GetSampleNumber();
			PROCESS_SAMPLE(mMosi, mosi_result, mMosiChannel);
			PROCESS_SAMPLE(mMiso, miso_result, mMisoChannel);
		}

		mArrowLocations.push_back(mCurrentSample);
	}

	if(status == e_GET_WORD_OK)
	{
		/* Add sample markers to the results */
		U32 count = (U32)mArrowLocations.size();
		for (U32 i = 0; i < count; i++)
		{
			mResults->AddMarker(mArrowLocations[i], mArrowMarker, mSettings->mClockChannel);
		}
	}

	return status;
}

bool SpiAnalyzer::NeedsRerun()
{
	if (bSettingsChangeID != mSettings->mChangeID)
	{
		bSettingsChangeID = mSettings->mChangeID;
		return true;
	}
	else
	{
		return false;
	}
}

U32 SpiAnalyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	if (mSimulationInitialized == false)
	{
		mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
		mSimulationInitialized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}


U32 SpiAnalyzer::GetMinimumSampleRateHz()
{
	/* Use Logic's lowest supported sample rate */
	return 10000;
}

const char* SpiAnalyzer::GetAnalyzerName() const
{
	return "ABCC SPI Protocol";
}

const char* GetAnalyzerName()
{
	return "ABCC SPI Protocol";
}

Analyzer* CreateAnalyzer()
{
	return new SpiAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer)
{
	delete analyzer;
}

bool SpiAnalyzer::IsEnableActive(void)
{
	if (mEnable != NULL)
	{
		return (mEnable->GetBitState() == BIT_LOW);
	}
	else
	{
		return true;
	}
}

void SpiAnalyzer::SignalReadyForNewPacket(bool fMosiChannel, tPacketType ePacketType)
{
	bool fStartNewPacket = false;
	if (fMosiChannel)
	{
		fMosiReadyForNewPacket = true;
		eMosiPacketType = ePacketType;
	}
	else
	{
		fMisoReadyForNewPacket = true;
		eMisoPacketType = ePacketType;
	}

	if (ePacketType == e_PROTOCOL_ERROR_PACKET)
	{
		fStartNewPacket = true;
		mResults->CancelPacketAndStartNewPacket();
		if (mEnable != NULL)
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::ErrorX, mSettings->mEnableChannel);
		}
	}
	else if (fMisoReadyForNewPacket && fMosiReadyForNewPacket)
	{
		U64 packet_id = mResults->CommitPacketAndStartNewPacket();
		fStartNewPacket = true;
		if (packet_id == INVALID_RESULT_INDEX)
		{
			if (mEnable != NULL)
			{
				mResults->AddMarker(mCurrentSample, AnalyzerResults::Zero, mSettings->mEnableChannel);
			}
		}
		else
		{
			if (mEnable != NULL)
			{
				AnalyzerResults::MarkerType eMarkerType = AnalyzerResults::One;
				/* If either MISO or MOSI contain an error report it,
				** Else if either MISO or MOSI contain an response report it,
				** Else report any command. */
				if ((eMosiPacketType == e_PROTOCOL_ERROR_PACKET) || (eMisoPacketType == e_PROTOCOL_ERROR_PACKET))
				{
					eMarkerType = AnalyzerResults::ErrorX;
				}
				else if ((eMosiPacketType == e_ERROR_RESPONSE_PACKET) || (eMisoPacketType == e_ERROR_RESPONSE_PACKET))
				{
					eMarkerType = AnalyzerResults::ErrorDot;
				}
				else if ((eMosiPacketType != e_NULL_PACKET) && (eMisoPacketType != e_NULL_PACKET))
				{
					eMarkerType = AnalyzerResults::Square;
				}
				else if ((eMosiPacketType == e_RESPONSE_PACKET) || (eMisoPacketType == e_RESPONSE_PACKET))
				{
					eMarkerType = AnalyzerResults::Stop;
				}
				else if ((eMosiPacketType == e_COMMAND_PACKET) || (eMisoPacketType == e_COMMAND_PACKET))
				{
					eMarkerType = AnalyzerResults::Start;
				}
				else if ((eMosiPacketType == e_MSG_FRAGMENT_PACKET) || (eMisoPacketType == e_MSG_FRAGMENT_PACKET))
				{
					eMarkerType = AnalyzerResults::Dot;
				}

				if (eMarkerType != AnalyzerResults::One)
				{
					mResults->AddMarker(mCurrentSample, eMarkerType, mSettings->mEnableChannel);
				}
			}
		}
		/* check if the source id is new */
		/* if new source id, allocate a new transaction id */
		/* if not a new source id, check that the header information matches the one in progress */
		/* if header information does not match, flag an error in the current frame */
	}
	if (fStartNewPacket)
	{
		fMosiReadyForNewPacket = false;
		fMisoReadyForNewPacket = false;

		/* Check if any additional clocks appear on SCLK before enable goes inactive */
		CheckForIdleAfterPacket();
	}
}

void SpiAnalyzer::CheckForIdleAfterPacket(void)
{
	Frame error_frame;
	U64 markerSample = 0;
	Channel mChannel;
	bool fAddError = false;

	if (mEnable != NULL)
	{
		if (mClock->WouldAdvancingToAbsPositionCauseTransition(mEnable->GetSampleOfNextEdge()))
		{
			mChannel = mSettings->mEnableChannel;
			error_frame.mStartingSampleInclusive = mClock->GetSampleOfNextEdge();
			error_frame.mEndingSampleInclusive = mEnable->GetSampleOfNextEdge();
			markerSample = error_frame.mEndingSampleInclusive;
			fAddError = true;
		}
		AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
	}
	else
	{
		if (!Is3WireIdleCondition(MIN_IDLE_GAP_TIME))
		{
			mChannel = mSettings->mClockChannel;
			error_frame.mStartingSampleInclusive = mClock->GetSampleOfNextEdge();
			AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
			error_frame.mEndingSampleInclusive = mClock->GetSampleOfNextEdge();
			markerSample = error_frame.mStartingSampleInclusive + (error_frame.mEndingSampleInclusive - error_frame.mStartingSampleInclusive) / 2;
			fAddError = true;
		}
	}

	if (fAddError)
	{
		error_frame.mFlags = (SPI_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG);
		error_frame.mType = e_ABCC_SPI_ERROR_END_OF_TRANSFER;
		mResults->AddFrame(error_frame);
		mResults->AddMarker(markerSample, AnalyzerResults::ErrorSquare, mChannel);
	}
}

void SpiAnalyzer::AddFragFrame(bool fMosi, U64 lFirstSample, U64 lLastSample)
{
	Frame error_frame;
	error_frame.mStartingSampleInclusive = lFirstSample;
	error_frame.mEndingSampleInclusive = lLastSample;
	error_frame.mData1 = 0;
	error_frame.mType = e_ABCC_SPI_ERROR_FRAGMENTATION;
	error_frame.mFlags = (SPI_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG);
	if (fMosi)
	{
		error_frame.mFlags |= SPI_MOSI_FLAG;

		/* Only apply marker from MOSI, this prevents multiple markers at the same spot
		** in such instances draw distance is reduced significantly. */
		if (mEnable != NULL)
		{
			mResults->AddMarker(lLastSample, AnalyzerResults::ErrorSquare, mSettings->mEnableChannel);
		}
		else
		{
			U64 markerSample = lFirstSample + (lLastSample - lFirstSample) / 2;
			mResults->AddMarker(markerSample, AnalyzerResults::ErrorSquare, mSettings->mClockChannel);
		}
	}
	mResults->AddFrame(error_frame);
}

void SpiAnalyzer::ProcessMisoFrame(tAbccMisoStates eState, U64 lFrameData, S64 lFramesFirstSample)
{
	Frame result_frame;
	result_frame.mFlags = 0x00;
	result_frame.mType = (U8)eState;
	result_frame.mStartingSampleInclusive = lFramesFirstSample;
	result_frame.mEndingSampleInclusive = (S64)mClock->GetSampleNumber();
	result_frame.mData1 = lFrameData;


	if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_obj)
	{
		sMisoMsgHeader.obj = (U8)lFrameData;
		dwMisoMdCnt = 0;
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_inst)
	{
		sMisoMsgHeader.inst = (U16)lFrameData;
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_cmd)
	{
		sMisoMsgHeader.cmd = (U8)lFrameData;

		/* Store the object code in frame data to handle object specific data */
		result_frame.mData2 = sMisoMsgHeader.obj;

		if ((lFrameData & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			fMisoErrorRsp = true;
		}
		else
		{
			fMisoErrorRsp = false;
		}
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt)
	{
		/* To better analyze the data in bubbletext
		** store the object code, instance, and command */
		memcpy(&result_frame.mData2, &sMisoMsgHeader, sizeof(sMisoMsgHeader));
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_size)
	{
		if ((U16)lFrameData > ABP_MAX_MSG_DATA_BYTES)
		{
			/* Max message data size exceeded */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			wMisoMdSize = 0;
			//TODO joca: the last state (where a valid packet was received) should be restored
			fMisoFirstFrag = false;
			fMisoLastFrag = false;
			fMisoFragmentation = false;
		}
		else
		{
			wMisoMdSize = (U16)lFrameData;
		}
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_data)
	{
		if (fMisoErrorRsp)
		{
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			/* Check if data is 0xFF, if so delay de-assertion of fMisoErrorRsp
			** so that the object specific error response can be detected */
			if ((((U8)lFrameData != (U8)0xFF) && (dwMisoMdCnt == 0)) ||
				(dwMisoMdCnt > 1))
			{
				fMisoErrorRsp = false;
			}
		}
		/* Add a byte counter that can be displayed
		** in the results for easy tracking of specific values */
		result_frame.mData2 = (U64)dwMisoMdCnt;
		dwMisoMdCnt++;
		/* Check if the message data counter has reached the end of valid data */
		if (dwMisoMdCnt > wMisoMdSize)
		{
			/* Override frame type */
			result_frame.mType = (U8)e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid;
		}
	}
	else if (eState == e_ABCC_MISO_ANB_STAT)
	{
		if (bLastAnbSts != (U8)lFrameData)
		{
			/* Anybus status change event */
			bLastAnbSts = (U8)lFrameData;
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG);
		}
	}
	else if (eState == e_ABCC_MISO_NET_TIME)
	{
		/* Compute delta from last timestamp and save it */
		((tNetworkTimeInfo*)&result_frame.mData2)->deltaTime = (U32)result_frame.mData1 - dwLastMisoTimestamp;
		((tNetworkTimeInfo*)&result_frame.mData2)->newRdPd = fMisoNewRdPd;
		((tNetworkTimeInfo*)&result_frame.mData2)->wrPdValid = fMosiWrPdValid;
		fMisoNewRdPd = false;
		fMosiWrPdValid = false;
		dwLastMisoTimestamp = (U32)result_frame.mData1;
	}
	else if (eState == e_ABCC_MISO_SPI_STAT)
	{
		if ((U8)(lFrameData & ABP_SPI_STATUS_WRMSG_FULL))
		{
			/* Write message buffer is full, possible overrun */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_WARNING_FLAG);
		}
	}
	else if (eState == e_ABCC_MISO_CRC32)
	{
		/* Save the computed CRC32 to the unused frameData */
		result_frame.mData2 = mMisoChecksum.Crc32();
		if (result_frame.mData2 != result_frame.mData1)
		{
			/* CRC Error */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			//TODO joca: the last state (where a valid packet was received) should be restored
			fMisoFirstFrag = false;
			fMisoLastFrag = false;
			fMisoFragmentation = false;
		}
	}

	/* Add a byte counter that can be displayed
	** in the results for easy tracking of specific values */
	if (eState == e_ABCC_MISO_RD_PD_FIELD)
	{
		result_frame.mData2 = (U64)dwMisoPdCnt;
		dwMisoPdCnt++;
	}
	else
	{
		dwMisoPdCnt = 0;
	}

	/* Handle indication of the SPI message fragmentation protocol */
	if (fMisoFragmentation)
	{
		result_frame.mFlags |= (SPI_MSG_FRAG_FLAG);
		if (fMisoFirstFrag)
		{
			result_frame.mFlags |= (SPI_MSG_FIRST_FRAG_FLAG);
		}
		if (eState == e_ABCC_MISO_CRC32)
		{
			fMisoFirstFrag = false;
			if (fMisoLastFrag)
			{
				fMisoLastFrag = false;
				fMisoFragmentation = false;
			}
		}
	}

	/* Commit the processed frame */
	mResults->AddFrame(result_frame);
	if (eState == e_ABCC_MISO_CRC32)
	{
		tPacketType ePacketType;
		if ((result_frame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
		{
			ePacketType = e_PROTOCOL_ERROR_PACKET;
		}
		else if (fMisoNewMsg)
		{
			if (((result_frame.mFlags & SPI_MSG_FRAG_FLAG) == SPI_MSG_FRAG_FLAG) &&
				!((result_frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG) == SPI_MSG_FIRST_FRAG_FLAG))
			{
				ePacketType = e_MSG_FRAGMENT_PACKET;
			}
			else
			{
				if (sMisoMsgHeader.cmd & ABP_MSG_HEADER_C_BIT)
				{
					ePacketType = e_COMMAND_PACKET;
				}
				else if (sMisoMsgHeader.cmd & ABP_MSG_HEADER_E_BIT)
				{
					ePacketType = e_ERROR_RESPONSE_PACKET;
				}
				else
				{
					ePacketType = e_RESPONSE_PACKET;
				}
			}
		}
		else
		{
			ePacketType = e_NULL_PACKET;
		}
		SignalReadyForNewPacket(false, ePacketType);
	}
}

void SpiAnalyzer::ProcessMosiFrame(tAbccMosiStates eState, U64 lFrameData, S64 lFramesFirstSample)
{
	Frame result_frame;
	result_frame.mFlags = SPI_MOSI_FLAG;
	result_frame.mType = (U8)eState;
	result_frame.mStartingSampleInclusive = lFramesFirstSample;
	result_frame.mEndingSampleInclusive = (S64)mClock->GetSampleNumber();
	result_frame.mData1 = lFrameData;


	if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_obj)
	{
		sMosiMsgHeader.obj = (U8)lFrameData;
		dwMosiMdCnt = 0;
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_inst)
	{
		sMosiMsgHeader.inst = (U16)lFrameData;
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd)
	{
		sMosiMsgHeader.cmd = (U8)lFrameData;

		/* Store the object code in frame data to handle object specific data */
		result_frame.mData2 = sMosiMsgHeader.obj;

		if ((lFrameData & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			fMosiErrorRsp = true;
		}
		else
		{
			fMosiErrorRsp = false;
		}
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt)
	{
		/* To better analyze the data in bubbletext
		** store the object code, instance, and command */
		memcpy(&result_frame.mData2, &sMosiMsgHeader, sizeof(sMosiMsgHeader));
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_size)
	{
		if ((U16)lFrameData > ABP_MAX_MSG_DATA_BYTES)
		{
			/* Max message data size exceeded */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			wMosiMdSize = 0;
			fMosiFirstFrag = false;
			fMosiLastFrag = false;
			fMosiFragmentation = false;
		}
		else
		{
			wMosiMdSize = (U16)lFrameData;
		}
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_data)
	{
		if (fMosiErrorRsp)
		{
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			/* Check if data is 0xFF, if so delay de-assertion of fMosiErrorRsp
			** so that the object specific error response can be detected */
			if ((((U8)lFrameData != (U8)0xFF) && (dwMosiMdCnt == 0)) ||
				(dwMosiMdCnt > 1))
			{
				fMosiErrorRsp = false;
			}
		}
		result_frame.mData2 = ((U64)sMosiMsgHeader.obj) << (8 * sizeof(U32));

		/* Add a byte counter that can be displayed
		** in the results for easy tracking of specific values */
		result_frame.mData2 |= (U64)dwMosiMdCnt;
		dwMosiMdCnt++;
		/* Check if the message data counter has reached the end of valid data */
		if (dwMosiMdCnt > wMosiMdSize)
		{
			/* Override frame type */
			result_frame.mType = (U8)e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid;
		}
	}
	else if (eState == e_ABCC_MOSI_APP_STAT)
	{
		if (bLastApplSts != (U8)lFrameData)
		{
			/* Application status change event */
			bLastApplSts = (U8)lFrameData;
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG);
		}
	}
	else if (eState == e_ABCC_MOSI_SPI_CTRL)
	{
		if (bMosiLastToggleState == (U8)(lFrameData & ABP_SPI_CTRL_T))
		{
			/* Retransmit event */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG);
		}
		else
		{
			bMosiLastToggleState = (U8)(lFrameData & ABP_SPI_CTRL_T);
		}
	}
	else if (eState == e_ABCC_MOSI_CRC32)
	{
		/* Save the computed CRC32 to the unused frameData */
		result_frame.mData2 = mMosiChecksum.Crc32();
		if (result_frame.mData2 != result_frame.mData1)
		{
			/* CRC Error */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			fMosiFirstFrag = false;
			fMosiLastFrag = false;
			fMosiFragmentation = false;
		}
	}

	/* Add a byte counter that can be displayed
	** in the results for easy tracking of specific values */
	if (eState == e_ABCC_MOSI_WR_PD_FIELD)
	{
		result_frame.mData2 = (U64)dwMosiPdCnt;
		dwMosiPdCnt++;
	}
	else
	{
		dwMosiPdCnt = 0;
	}

	/* Handle indication of the SPI message fragmentation protocol */
	if (fMosiFragmentation)
	{
		result_frame.mFlags |= (SPI_MSG_FRAG_FLAG);
		if (fMosiFirstFrag)
		{
			result_frame.mFlags |= (SPI_MSG_FIRST_FRAG_FLAG);
		}
		if (eState == e_ABCC_MOSI_PAD)
		{
			fMosiFirstFrag = false;
			if (fMosiLastFrag)
			{
				fMosiLastFrag = false;
				fMosiFragmentation = false;
			}
		}
	}

	/* Commit the processed frame */
	mResults->AddFrame(result_frame);
	if (eState == e_ABCC_MOSI_PAD)
	{
		tPacketType ePacketType;
		if ((result_frame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
		{
			ePacketType = e_PROTOCOL_ERROR_PACKET;
		}
		else if (fMosiNewMsg)
		{
			if (((result_frame.mFlags & SPI_MSG_FRAG_FLAG) == SPI_MSG_FRAG_FLAG) &&
				!((result_frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG) == SPI_MSG_FIRST_FRAG_FLAG))
			{
				ePacketType = e_MSG_FRAGMENT_PACKET;
			}
			else
			{
				if (sMosiMsgHeader.cmd & ABP_MSG_HEADER_C_BIT)
				{
					ePacketType = e_COMMAND_PACKET;
				}
				else if (sMosiMsgHeader.cmd & ABP_MSG_HEADER_E_BIT)
				{
					ePacketType = e_ERROR_RESPONSE_PACKET;
				}
				else
				{
					ePacketType = e_RESPONSE_PACKET;
				}
			}
		}
		else
		{
			ePacketType = e_NULL_PACKET;
		}
		SignalReadyForNewPacket(true, ePacketType);
	}
}

bool SpiAnalyzer::RunAbccMisoStateMachine(bool fReset, bool fError, U64 lMisoData, S64 lFirstSample)
{
	tAbccMisoStates eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
	tAbccMisoStates eMisoState_Current = e_ABCC_MISO_IDLE;
	bool fAddFrame = false;

	eMisoState_Current = eMisoState;

	/* If an error is signaled we jump into IDLE and wait to be reset.
	** A reset should be logically signaled when CS# is brought HIGH.
	** This would essentially indicate the begining of a new transaction. */
	if (fError || !IsEnableActive())// || WouldAdvancingTheClockToggleEnable())
	{
		eMisoState = e_ABCC_MISO_IDLE;
		if (dwMisoByteCnt == 0)
		{
			lMisoFramesFirstSample = lFirstSample;
		}
		if (mEnable != NULL)
		{
			AddFragFrame(false, lMisoFramesFirstSample, mEnable->GetSampleOfNextEdge());
		}
		else
		{
			/* 3-wire mode fragments exist only when idle gaps are detected too soon. */
			AddFragFrame(false, lMisoFramesFirstSample, mClock->GetSampleOfNextEdge());
		}
		mResults->CommitResults();
		return true;
	}

	if (eMisoState == e_ABCC_MISO_IDLE)
	{
		mMisoChecksum.Init();
		lMisoFrameData = 0;
		dwMisoByteCnt = 0;
		if (fReset)
		{
			eMisoState = e_ABCC_MISO_Reserved1;
			eMisoState_Current = eMisoState;
		}
	}

	if (dwMisoByteCnt == 0)
	{
		lMisoFramesFirstSample = lFirstSample;
	}

	lMisoFrameData |= (lMisoData << (8 * dwMisoByteCnt));
	dwMisoByteCnt++;

	if (eMisoState != e_ABCC_MISO_CRC32)
	{
		mMisoChecksum.Update((U8*)&lMisoData, 1);
	}

	switch (eMisoState)
	{
	case e_ABCC_MISO_IDLE:
		/* We wait here until a reset is signaled */
		break;
	case e_ABCC_MISO_Reserved1:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			fAddFrame = true;
			eMisoState = e_ABCC_MISO_Reserved2;
		}
		break;
	case e_ABCC_MISO_Reserved2:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			fAddFrame = true;
			eMisoState = e_ABCC_MISO_LED_STAT;
		}
		break;
	case e_ABCC_MISO_LED_STAT:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			fAddFrame = true;
			eMisoState = e_ABCC_MISO_ANB_STAT;
		}
		break;
	case e_ABCC_MISO_ANB_STAT:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			fAddFrame = true;
			eMisoState = e_ABCC_MISO_SPI_STAT;
		}
		break;
	case e_ABCC_MISO_SPI_STAT:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			if((lMisoFrameData & ABP_SPI_STATUS_NEW_PD) == ABP_SPI_STATUS_NEW_PD)
			{
				fMisoNewRdPd = true;
			}
			else
			{
				fMisoNewRdPd = false;
			}
			if ((lMisoFrameData & (ABP_SPI_STATUS_LAST_FRAG | ABP_SPI_STATUS_M)) == ABP_SPI_STATUS_M)
			{
				/* New message but not the last */
				fMisoNewMsg = true;
				if (!fMisoFragmentation)
				{
					/* Message fragmentation starts */
					fMisoFragmentation = true;
					fMisoFirstFrag = true;
					fMisoLastFrag = false;
				}
			}
			else if ((lMisoFrameData & (ABP_SPI_STATUS_LAST_FRAG | ABP_SPI_STATUS_M)) == (ABP_SPI_STATUS_LAST_FRAG | ABP_SPI_STATUS_M))
			{
				/* New message and last */
				fMisoNewMsg = true;
				if (fMisoFragmentation)
				{
					/* Message fragmentation ends */
					fMisoFirstFrag = false;
					fMisoLastFrag = true;
				}
			}
			else
			{
				/* No new message */
				fMisoNewMsg = false;
				eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid;
				dwMisoMdCnt = 0;
				wMisoMdSize = 0;
			}
			fAddFrame = true;
			eMisoState = e_ABCC_MISO_NET_TIME;
		}
		break;
	case e_ABCC_MISO_NET_TIME:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			fAddFrame = true;
			if (dwMisoMsgLen != 0)
			{
				eMisoState = e_ABCC_MISO_RD_MSG_FIELD;
				if (fMisoNewMsg)
				{
					RunAbccMisoMsgSubStateMachine(true, NULL, &eMsgSubState);
				}
			}
			else if (dwMisoPdLen != 0)
			{
				eMisoState = e_ABCC_MISO_RD_PD_FIELD;
			}
			else
			{
				eMisoState = e_ABCC_MISO_CRC32;
			}
		}
		break;
	case e_ABCC_MISO_RD_MSG_FIELD:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			if (fMisoFragmentation && !fMisoFirstFrag)
			{
				eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data;
				fAddFrame = true;
			}
			else
			{
				if (!RunAbccMisoMsgSubStateMachine(false, &fAddFrame, &eMsgSubState))
				{
					/* Error */
					eMisoState = e_ABCC_MISO_IDLE;
				}
			}
			if (dwMisoMsgLen == 1)
			{
				if (dwMisoPdLen != 0)
				{
					eMisoState = e_ABCC_MISO_RD_PD_FIELD;
				}
				else
				{
					eMisoState = e_ABCC_MISO_CRC32;
				}
			}
			//fAddFrame = true;
			dwMisoMsgLen--;
		}
		break;
	case e_ABCC_MISO_RD_PD_FIELD:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			if (dwMisoPdLen == 1)
			{
				eMisoState = e_ABCC_MISO_CRC32;
			}
			fAddFrame = true;
			dwMisoPdLen--;
		}
		break;
	case e_ABCC_MISO_CRC32:
		if (dwMisoByteCnt >= GET_MISO_FRAME_SIZE(eMisoState))
		{
			fAddFrame = true;
			eMisoState = e_ABCC_MISO_IDLE;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_res1:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_res2:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
	default:
		eMisoState = e_ABCC_MISO_IDLE;
		break;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		if (eMisoState != e_ABCC_MISO_IDLE)
		{
			/* We have a fragmented message */
			if (mEnable != NULL)
			{
				AddFragFrame(false, lMisoFramesFirstSample, mEnable->GetSampleOfNextEdge());
			}
			else
			{
				AddFragFrame(false, lMisoFramesFirstSample, mClock->GetSampleNumber());
			}
			eMisoState = e_ABCC_MISO_IDLE;
			lMisoFrameData = 0;
			dwMisoByteCnt = 0;
			return true;
		}
	}

	if (fAddFrame)
	{
		if (eMisoState_Current == e_ABCC_MISO_RD_MSG_FIELD)
		{
			ProcessMisoFrame(eMsgSubState, lMisoFrameData, lMisoFramesFirstSample);
		}
		else
		{
			ProcessMisoFrame(eMisoState_Current, lMisoFrameData, lMisoFramesFirstSample);
		}

		/* Reset the state variables */
		lMisoFrameData = 0;
		dwMisoByteCnt = 0;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		eMisoState = e_ABCC_MISO_IDLE;
		lMisoFrameData = 0;
		dwMisoByteCnt = 0;
	}

	if (eMisoState == e_ABCC_MISO_IDLE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SpiAnalyzer::RunAbccMosiStateMachine(bool fReset, bool fError, U64 lMosiData, S64 lFirstSample)
{
	tAbccMosiStates eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
	tAbccMosiStates eMosiState_Current = e_ABCC_MOSI_IDLE;
	bool fAddFrame = false;

	eMosiState_Current = eMosiState;

	/* If an error is signaled we jump into IDLE and wait to be reset.
	** A reset should be logically signaled when CS# is brought HIGH.
	** This would essentially indicate the begining of a new transaction. */
	if (fError || !IsEnableActive())// || WouldAdvancingTheClockToggleEnable())
	{
		if (dwMosiByteCnt == 0)
		{
			lMosiFramesFirstSample = lFirstSample;
		}
		eMosiState = e_ABCC_MOSI_IDLE;
		if (mEnable != NULL)
		{
			AddFragFrame(true, lMosiFramesFirstSample, mEnable->GetSampleOfNextEdge());
		}
		else
		{
			/* 3-wire mode fragments exist only when idle gaps are detected too soon. */
			AddFragFrame(true, lMosiFramesFirstSample, mClock->GetSampleOfNextEdge());
		}
		mResults->CommitResults();
		return true;
	}

	if (eMosiState == e_ABCC_MOSI_IDLE)
	{
		mMosiChecksum.Init();
		lMosiFrameData = 0;
		dwMosiByteCnt = 0;
		if (fReset)
		{
			eMosiState = e_ABCC_MOSI_SPI_CTRL;
			eMosiState_Current = eMosiState;
		}
	}

	if (dwMosiByteCnt == 0)
	{
		lMosiFramesFirstSample = lFirstSample;
	}

	lMosiFrameData |= (lMosiData << (8 * dwMosiByteCnt));
	dwMosiByteCnt++;

	if (eMosiState != e_ABCC_MOSI_CRC32)
	{
		mMosiChecksum.Update((U8*)&lMosiData, 1);
	}

	switch (eMosiState)
	{
	case e_ABCC_MOSI_IDLE:
		/* We wait here until a reset is signaled */
		break;
	case e_ABCC_MOSI_SPI_CTRL:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			if((lMosiFrameData & ABP_SPI_CTRL_WRPD_VALID) == ABP_SPI_CTRL_WRPD_VALID)
			{
				fMosiWrPdValid = true;
			}
			else
			{
				fMosiWrPdValid = false;
			}

			if ((lMosiFrameData & (ABP_SPI_CTRL_LAST_FRAG | ABP_SPI_CTRL_M)) == ABP_SPI_CTRL_M)
			{
				/* New message but not the last */
				fMosiNewMsg = true;
				if (!fMosiFragmentation)
				{
					/* Message fragmentation starts */
					fMosiFragmentation = true;
					fMosiFirstFrag = true;
					fMosiLastFrag = false;
				}
			}
			else if ((lMosiFrameData & (ABP_SPI_CTRL_LAST_FRAG | ABP_SPI_CTRL_M)) == (ABP_SPI_CTRL_LAST_FRAG | ABP_SPI_CTRL_M))
			{
				/* New message and last */
				fMosiNewMsg = true;
				if (fMosiFragmentation)
				{
					/* Message fragmentation ends */
					fMosiFirstFrag = false;
					fMosiLastFrag = true;
				}
			}
			else
			{
				/* No new message */
				fMosiNewMsg = false;
				eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid;
				dwMosiMdCnt = 0;
				wMosiMdSize = 0;
			}
			fAddFrame = true;
			eMosiState = e_ABCC_MOSI_RESERVED1;
		}
		break;
	case e_ABCC_MOSI_RESERVED1:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			eMosiState = e_ABCC_MOSI_MSG_LEN;
		}
		break;
	case e_ABCC_MOSI_MSG_LEN:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			dwMosiMsgLen = (U32)lMosiFrameData * 2;
			dwMisoMsgLen = dwMosiMsgLen;
			eMosiState = e_ABCC_MOSI_PD_LEN;
		}
		break;
	case e_ABCC_MOSI_PD_LEN:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			dwMosiPdLen = (U32)lMosiFrameData * 2;
			dwMisoPdLen = dwMosiPdLen;
			eMosiState = e_ABCC_MOSI_APP_STAT;
		}
		break;
	case e_ABCC_MOSI_APP_STAT:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			eMosiState = e_ABCC_MOSI_INT_MASK;
		}
		break;
	case e_ABCC_MOSI_INT_MASK:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			if (dwMosiMsgLen != 0)
			{
				eMosiState = e_ABCC_MOSI_WR_MSG_FIELD;
				if (fMosiNewMsg)
				{
					RunAbccMosiMsgSubStateMachine(true, NULL, &eMsgSubState);
				}
			}
			else if (dwMosiPdLen != 0)
			{
				eMosiState = e_ABCC_MOSI_WR_PD_FIELD;
			}
			else
			{
				eMosiState = e_ABCC_MOSI_CRC32;
			}
		}
		break;
	case e_ABCC_MOSI_WR_MSG_FIELD:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			if (fMosiFragmentation && !fMosiFirstFrag)
			{
				eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data;
				fAddFrame = true;
			}
			else
			{
				if (!RunAbccMosiMsgSubStateMachine(false, &fAddFrame, &eMsgSubState))
				{
					/* Error */
					eMosiState = e_ABCC_MOSI_IDLE;
				}
			}
			if (dwMosiMsgLen == 1)
			{
				if (dwMosiPdLen != 0)
				{
					eMosiState = e_ABCC_MOSI_WR_PD_FIELD;
				}
				else
				{
					eMosiState = e_ABCC_MOSI_CRC32;
				}
			}
			dwMosiMsgLen--;
		}
		break;
	case e_ABCC_MOSI_WR_PD_FIELD:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			if (dwMosiPdLen == 1)
			{
				eMosiState = e_ABCC_MOSI_CRC32;
			}
			fAddFrame = true;
			dwMosiPdLen--;
		}
		break;
	case e_ABCC_MOSI_CRC32:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			eMosiState = e_ABCC_MOSI_PAD;
		}
		break;
	case e_ABCC_MOSI_PAD:
		if (dwMosiByteCnt >= GET_MOSI_FRAME_SIZE(eMosiState))
		{
			fAddFrame = true;
			eMosiState = e_ABCC_MOSI_IDLE;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_res1:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_obj:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_res2:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
	default:
		eMosiState = e_ABCC_MOSI_IDLE;
		break;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		if (eMosiState != e_ABCC_MOSI_IDLE)
		{
			/* We have a fragmented message */
			if (mEnable != NULL)
			{
				AddFragFrame(true, lMosiFramesFirstSample, mEnable->GetSampleOfNextEdge());
			}
			else
			{
				AddFragFrame(true, lMosiFramesFirstSample, mClock->GetSampleOfNextEdge());
			}
			eMosiState = e_ABCC_MOSI_IDLE;
			lMosiFrameData = 0;
			dwMosiByteCnt = 0;
			return true;
		}
	}

	if (fAddFrame)
	{
		if (eMosiState_Current == e_ABCC_MOSI_WR_MSG_FIELD)
		{
			ProcessMosiFrame(eMsgSubState, lMosiFrameData, lMosiFramesFirstSample);
		}
		else
		{
			ProcessMosiFrame(eMosiState_Current, lMosiFrameData, lMosiFramesFirstSample);
		}

		/* Reset the state variables */
		lMosiFrameData = 0;
		dwMosiByteCnt = 0;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		eMosiState = e_ABCC_MOSI_IDLE;
		lMosiFrameData = 0;
		dwMosiByteCnt = 0;
	}

	if (eMosiState == e_ABCC_MOSI_IDLE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SpiAnalyzer::RunAbccMisoMsgSubStateMachine(bool fReset, bool* pfAddFrame, tAbccMisoStates *peMisoMsgSubState)
{
	if (fReset)
	{
		/* Perform checks here that we were in the last state and that the
		** number of bytes seen in this state matched the header's msg len specifier
		** In such cases a "framing error" should be signaled */
		eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
		bMisoByteCnt2 = 0;
		return true;
	}

	if ((pfAddFrame == NULL) || (peMisoMsgSubState == NULL))
	{
		return false;
	}

	*peMisoMsgSubState = eMisoMsgSubState;
	bMisoByteCnt2++;

	switch (eMisoMsgSubState)
	{
	case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_res1;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_res1:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_srcId;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_obj;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_inst;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_cmd;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_res2;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_res2:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
			eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid:
		if (bMisoByteCnt2 >= GET_MISO_FRAME_SIZE(eMisoMsgSubState))
		{
			*pfAddFrame = true;
		}
		break;
	case e_ABCC_MISO_IDLE:
	case e_ABCC_MISO_Reserved1:
	case e_ABCC_MISO_Reserved2:
	case e_ABCC_MISO_LED_STAT:
	case e_ABCC_MISO_ANB_STAT:
	case e_ABCC_MISO_SPI_STAT:
	case e_ABCC_MISO_NET_TIME:
	case e_ABCC_MISO_RD_MSG_FIELD:
	case e_ABCC_MISO_RD_PD_FIELD:
	case e_ABCC_MISO_CRC32:
	default:
		eMisoMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data;
		return false;
	}

	if (*pfAddFrame == true)
	{
		bMisoByteCnt2 = 0;
	}

	return true;
}

bool SpiAnalyzer::RunAbccMosiMsgSubStateMachine(bool fReset, bool* pfAddFrame, tAbccMosiStates* peMosiMsgSubState)
{
	if (fReset)
	{
		/* Perform checks here that we were in the last state and that the
		** number of bytes seen in this state matched the header's msg len specifier
		** In such cases a "framing error" should be signaled */
		eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
		bMosiByteCnt2 = 0;
		return true;
	}

	if ((pfAddFrame == NULL) || (peMosiMsgSubState == NULL))
	{
		return false;
	}

	*peMosiMsgSubState = eMosiMsgSubState;
	bMosiByteCnt2++;

	switch (eMosiMsgSubState)
	{
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_res1;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_res1:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_obj;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_inst;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_res2;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_res2:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
			eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid:
		if (bMosiByteCnt2 >= GET_MOSI_FRAME_SIZE(eMosiMsgSubState))
		{
			*pfAddFrame = true;
		}
		break;
	case e_ABCC_MOSI_PAD:
	case e_ABCC_MOSI_CRC32:
	case e_ABCC_MOSI_WR_PD_FIELD:
	case e_ABCC_MOSI_WR_MSG_FIELD:
	case e_ABCC_MOSI_INT_MASK:
	case e_ABCC_MOSI_APP_STAT:
	case e_ABCC_MOSI_PD_LEN:
	case e_ABCC_MOSI_MSG_LEN:
	case e_ABCC_MOSI_RESERVED1:
	case e_ABCC_MOSI_SPI_CTRL:
	case e_ABCC_MOSI_IDLE:
	default:
		eMosiMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data;
		return false;
	}

	if (*pfAddFrame == true)
	{
		bMosiByteCnt2 = 0;
	}

	return true;
}

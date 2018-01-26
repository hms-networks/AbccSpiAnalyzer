/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
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

#define IS_3WIRE_MODE() (((mEnable == NULL) && (mSettings->m4WireOn3Channels == false)) || (mSettings->m3WireOn4Channels == true))
#define IS_PURE_4WIRE_MODE() ((mEnable != NULL) && (mSettings->m3WireOn4Channels == false))

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

	mSettingsChangeID   = mSettings->mChangeID;

	mMosiVars.eState              = e_ABCC_MOSI_IDLE;
	mMisoVars.eState              = e_ABCC_MISO_IDLE;
	mMisoVars.bLastAnbSts         = 0xFF;
	mMosiVars.bLastApplSts        = 0xFF;
	mMosiVars.bLastToggleState    = 0xFF;
	mMosiVars.dwMsgLen            = 0;
	mMosiVars.dwPdLen             = 0;
	mMisoVars.dwMsgLen            = 0;
	mMisoVars.dwPdLen             = 0;

	mMisoVars.fNewMsg             = false;
	mMisoVars.fErrorRsp           = true;
	mMisoVars.fFragmentation      = false;
	mMisoVars.fFirstFrag          = false;
	mMisoVars.fLastFrag           = false;
	mMisoVars.fNewRdPd            = false;
	mMosiVars.fNewMsg             = false;
	mMosiVars.fErrorRsp           = true;
	mMosiVars.fFragmentation      = false;
	mMosiVars.fFirstFrag          = false;
	mMosiVars.fLastFrag           = false;
	mMosiVars.fWrPdValid          = false;

	mMisoVars.fReadyForNewPacket  = false;
	mMosiVars.fReadyForNewPacket  = false;

	memset(&mMisoVars.sMsgHeader, 0, sizeof(mMisoVars.sMsgHeader));
	mMisoVars.dwPdCnt = 0;
	mMisoVars.dwMdCnt = 0;
	mMisoVars.wMdSize = 0;

	memset(&mMosiVars.sMsgHeader, 0, sizeof(mMosiVars.sMsgHeader));
	mMosiVars.dwPdCnt = 0;
	mMosiVars.dwMdCnt = 0;
	mMosiVars.wMdSize = 0;

	mMisoVars.dwLastTimestamp = 0;

	mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
	mMisoVars.dwByteCnt    = 0;
	mMisoVars.lFrameData   = 0;

	mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
	mMosiVars.dwByteCnt    = 0;
	mMosiVars.lFrameData   = 0;

	mMosiVars.bByteCnt2 = 0;
	mMisoVars.bByteCnt2 = 0;
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
		mMisoVars.oChecksum = AbccCrc();
		mMosiVars.oChecksum = AbccCrc();

		AdvanceToActiveEnableEdgeWithCorrectClockPolarity();

		/* Reset persistent state logic between captures */
		mMosiVars.eState = e_ABCC_MOSI_IDLE;
		mMisoVars.eState = e_ABCC_MISO_IDLE;
		mMisoVars.bLastAnbSts = 0xFF;
		mMosiVars.bLastApplSts = 0xFF;

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

			if (IS_3WIRE_MODE())
			{
				if (!fReady1 && !fReady2)
				{
					if (Is3WireIdleCondition(MAX_CLOCK_IDLE_HI_TIME))
					{
						mMosiVars.eState = e_ABCC_MOSI_SPI_CTRL;
						mMisoVars.eState = e_ABCC_MISO_Reserved1;
						mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
						mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
						mMisoVars.oChecksum.Init();
						mMosiVars.oChecksum.Init();
						mMisoVars.lFrameData = 0;
						mMosiVars.lFrameData = 0;
						mMisoVars.dwByteCnt = 0;
						mMosiVars.dwByteCnt = 0;
						fError = true;
					}
				}
			}

			if (fError == true)
			{
				/* Signal error, do not commit packet */
				SetMosiPacketType(e_CANCEL_PACKET);
				SignalReadyForNewPacket(true);

				/* Advance to the next enabled (or idle) state */
				if (mEnable != NULL)
				{
					AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				}
				else
				{
					AddFragFrame(false, mMisoVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
					AddFragFrame(true, mMosiVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
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

	if (IS_3WIRE_MODE())
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
	if (IS_PURE_4WIRE_MODE())
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
	if (IS_3WIRE_MODE())
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
	if (IS_3WIRE_MODE())
	{
		return false;
	}

	if (mEnable != NULL)
	{
		if( mClock->DoMoreTransitionsExistInCurrentData() )
		{
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
		else if( mEnable->DoMoreTransitionsExistInCurrentData() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool SpiAnalyzer::Is3WireIdleCondition(float rIdleTimeCondition)
{
	if ( mSettings->m4WireOn3Channels )
	{
		return false;
	}

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
				/* The enable state changed in the middle of aquiring a byte.
				** Suggests we are not byte-synchronized. */
				status = e_GET_WORD_RESET;
				break;
			}
		}

		if (i == 0)
		{
			if (mClock->GetBitState() == BIT_HIGH)
			{
				fClkIdleHigh = true;
			}
		}

		/* For CLOCK IDLE LOW configurations, skip advancing the clock when sampling the first bit. */
		if (IS_3WIRE_MODE())
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

		if (IS_3WIRE_MODE())
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

	mResults->CommitResults();

	return status;
}

bool SpiAnalyzer::NeedsRerun()
{
	if (mSettingsChangeID != mSettings->mChangeID)
	{
		mSettingsChangeID = mSettings->mChangeID;
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
	if (IS_PURE_4WIRE_MODE())
	{
		return (mEnable->GetBitState() == BIT_LOW);
	}
	else
	{
		return true;
	}
}

static bool IsErrorPacketType(tPacketType ePacketType)
{
	switch (ePacketType)
	{
	case e_PROTOCOL_ERROR_PACKET:
	case e_CHECKSUM_ERROR_PACKET:
	case e_ERROR_RESPONSE_PACKET:
	case e_MULTI_ERROR_PACKET:
	case e_CANCEL_PACKET:
		return true;
	}
	return false;
}

AnalyzerResults::MarkerType SpiAnalyzer::GetPacketMarkerType(void)
{
	AnalyzerResults::MarkerType eMarkerType;

	/* Determine marker type based on packet types:
	** Dot - Message Fragment
	** Start - Message Command
	** Stop - Message Response
	** Square - Multiple Events (no errors)
	** ErrorDot - Error Response
	** ErrorX - Checksum Error
	** ErrorSquare - Protocol Error or Multiple Events with at least one error
	*/
	if ((mMosiVars.ePacketType != e_NULL_PACKET) && (mMisoVars.ePacketType != e_NULL_PACKET))
	{
		/* Multiple events (at least one on each channel, or multiple events on one channel) */
		if (IsErrorPacketType(mMosiVars.ePacketType) || IsErrorPacketType(mMisoVars.ePacketType))
		{
			eMarkerType = AnalyzerResults::ErrorSquare;
		}
		else
		{
			eMarkerType = AnalyzerResults::Square;
		}
	}
	else
	{
		/* Only one event */
		if ((mMosiVars.ePacketType == e_MULTI_ERROR_PACKET) || (mMisoVars.ePacketType == e_MULTI_ERROR_PACKET))
		{
			eMarkerType = AnalyzerResults::ErrorSquare;
		}
		else if ((mMosiVars.ePacketType == e_PROTOCOL_ERROR_PACKET) || (mMisoVars.ePacketType == e_PROTOCOL_ERROR_PACKET))
		{
			eMarkerType = AnalyzerResults::ErrorSquare;
		}
		else if ((mMosiVars.ePacketType == e_CHECKSUM_ERROR_PACKET) || (mMisoVars.ePacketType == e_CHECKSUM_ERROR_PACKET))
		{
			eMarkerType = AnalyzerResults::ErrorX;
		}
		else if ((mMosiVars.ePacketType == e_ERROR_RESPONSE_PACKET) || (mMisoVars.ePacketType == e_ERROR_RESPONSE_PACKET))
		{
			eMarkerType = AnalyzerResults::ErrorDot;
		}
		else if ((mMosiVars.ePacketType == e_MULTI_PACKET) || (mMisoVars.ePacketType == e_MULTI_PACKET))
		{
			eMarkerType = AnalyzerResults::Square;
		}
		else if ((mMosiVars.ePacketType == e_RESPONSE_PACKET) || (mMisoVars.ePacketType == e_RESPONSE_PACKET))
		{
			eMarkerType = AnalyzerResults::Stop;
		}
		else if ((mMosiVars.ePacketType == e_COMMAND_PACKET) || (mMisoVars.ePacketType == e_COMMAND_PACKET))
		{
			eMarkerType = AnalyzerResults::Start;
		}
		else if ((mMosiVars.ePacketType == e_MSG_FRAGMENT_PACKET) || (mMisoVars.ePacketType == e_MSG_FRAGMENT_PACKET))
		{
			eMarkerType = AnalyzerResults::Dot;
		}
		else if ((mMosiVars.ePacketType == e_NULL_PACKET) || (mMisoVars.ePacketType == e_NULL_PACKET))
		{
			eMarkerType = AnalyzerResults::One;
		}
		else
		{
			eMarkerType = AnalyzerResults::UpArrow;
		}
	}

	return eMarkerType;
}

void SpiAnalyzer::SignalReadyForNewPacket(bool fMosiChannel)
{
	bool fStartNewPacket = false;
	if (fMosiChannel)
	{
		mMosiVars.fReadyForNewPacket = true;
	}
	else
	{
		mMisoVars.fReadyForNewPacket = true;
	}

	if (mMosiVars.ePacketType == e_CANCEL_PACKET)
	{
		fStartNewPacket = true;
		mResults->CancelPacketAndStartNewPacket();
		if (mEnable != NULL)
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::ErrorX, mSettings->mEnableChannel);
		}
	}
	else if (mMisoVars.fReadyForNewPacket && mMosiVars.fReadyForNewPacket)
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
				AnalyzerResults::MarkerType eMarkerType = GetPacketMarkerType();

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
		mMosiVars.fReadyForNewPacket = false;
		mMisoVars.fReadyForNewPacket = false;
		mMosiVars.ePacketType = e_NULL_PACKET;
		mMisoVars.ePacketType = e_NULL_PACKET;

		/* Check if any additional clocks appear on SCLK before enable goes inactive */
		CheckForIdleAfterPacket();
	}
}

void SpiAnalyzer::CheckForIdleAfterPacket(void)
{
	Frame error_frame;
	U64 markerSample = 0;
	Channel chn;
	bool fAddError = false;

	if (IS_PURE_4WIRE_MODE())
	{
		U64 lNextSample = mEnable->GetSampleOfNextEdge();
		if (lNextSample <= mClock->GetSampleNumber())
		{
			mEnable->AdvanceToAbsPosition(mClock->GetSampleNumber()); //TODO joca: this fixes the crashing...other changes dont do much
			lNextSample = mEnable->GetSampleOfNextEdge();
		}

		if (mClock->WouldAdvancingToAbsPositionCauseTransition(lNextSample))
		{
			chn = mSettings->mEnableChannel;
			error_frame.mStartingSampleInclusive = mClock->GetSampleOfNextEdge();
			error_frame.mEndingSampleInclusive = mEnable->GetSampleOfNextEdge();
			markerSample = error_frame.mEndingSampleInclusive;
			fAddError = true;
		}
		AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
	}
	else
	{
		/* Skip idle check when m4WireOn3Channels is being used, since it is
		** impossible to infer if the enable line had toggled or not */
		if( mSettings->m4WireOn3Channels == false )
		{
			if (!Is3WireIdleCondition(MIN_IDLE_GAP_TIME))
			{
				chn = mSettings->mClockChannel;
				error_frame.mStartingSampleInclusive = mClock->GetSampleOfNextEdge();
				AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				error_frame.mEndingSampleInclusive = mClock->GetSampleOfNextEdge();
				markerSample = error_frame.mStartingSampleInclusive + (error_frame.mEndingSampleInclusive - error_frame.mStartingSampleInclusive) / 2;
				fAddError = true;
			}
		}
	}

	if (fAddError)
	{
		error_frame.mFlags = (SPI_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG);
		error_frame.mType = e_ABCC_SPI_ERROR_END_OF_TRANSFER;
		mResults->AddFrame(error_frame);
		mResults->AddMarker(markerSample, AnalyzerResults::ErrorSquare, chn);
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
		mMisoVars.sMsgHeader.obj = (U8)lFrameData;
		mMisoVars.dwMdCnt = 0;
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_inst)
	{
		mMisoVars.sMsgHeader.inst = (U16)lFrameData;

		/* Store the object code in frame data to handle object specific data */
		result_frame.mData2 = mMisoVars.sMsgHeader.obj;
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_cmd)
	{
		mMisoVars.sMsgHeader.cmd = (U8)lFrameData;

		/* Store the object code in frame data to handle object specific data */
		result_frame.mData2 = mMisoVars.sMsgHeader.obj;

		if ((lFrameData & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			mMisoVars.fErrorRsp = true;
		}
		else
		{
			mMisoVars.fErrorRsp = false;
		}
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt)
	{
		/* To better analyze the data in bubbletext
		** store the object code, instance, and command */
		memcpy(&result_frame.mData2, &mMisoVars.sMsgHeader, sizeof(mMisoVars.sMsgHeader));
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_size)
	{
		if ((U16)lFrameData > ABP_MAX_MSG_DATA_BYTES)
		{
			/* Max message data size exceeded */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			mMisoVars.wMdSize = 0;
			//TODO joca: the last state (where a valid packet was received) should be restored
			mMisoVars.fFirstFrag = false;
			mMisoVars.fLastFrag = false;
			mMisoVars.fFragmentation = false;
		}
		else
		{
			mMisoVars.wMdSize = (U16)lFrameData;
		}
	}
	else if (eState == e_ABCC_MISO_RD_MSG_SUBFIELD_data)
	{
		if (mMisoVars.fErrorRsp)
		{
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			/* Check if data is 0xFF, if so delay de-assertion of fMisoErrorRsp
			** so that the object specific error response can be detected */
			if ((((U8)lFrameData != (U8)0xFF) && (mMisoVars.dwMdCnt == 0)) ||
				(mMisoVars.dwMdCnt > 1))
			{
				mMisoVars.fErrorRsp = false;
			}
		}
		/* Add a byte counter that can be displayed
		** in the results for easy tracking of specific values */
		result_frame.mData2 = (U64)mMisoVars.dwMdCnt;
		mMisoVars.dwMdCnt++;
		/* Check if the message data counter has reached the end of valid data */
		if (mMisoVars.dwMdCnt > mMisoVars.wMdSize)
		{
			/* Override frame type */
			result_frame.mType = (U8)e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid;
		}
	}
	else if (eState == e_ABCC_MISO_ANB_STAT)
	{
		if (mMisoVars.bLastAnbSts != (U8)lFrameData)
		{
			/* Anybus status change event */
			mMisoVars.bLastAnbSts = (U8)lFrameData;
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG);
		}
	}
	else if (eState == e_ABCC_MISO_NET_TIME)
	{
		/* Compute delta from last timestamp and save it */
		((tNetworkTimeInfo*)&result_frame.mData2)->deltaTime = (U32)result_frame.mData1 - mMisoVars.dwLastTimestamp;
		((tNetworkTimeInfo*)&result_frame.mData2)->newRdPd = mMisoVars.fNewRdPd;
		((tNetworkTimeInfo*)&result_frame.mData2)->wrPdValid = mMosiVars.fWrPdValid;
		mMisoVars.fNewRdPd = false;
		mMosiVars.fWrPdValid = false;
		mMisoVars.dwLastTimestamp = (U32)result_frame.mData1;
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
		result_frame.mData2 = mMisoVars.oChecksum.Crc32();
		if (result_frame.mData2 != result_frame.mData1)
		{
			/* CRC Error */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			//TODO joca: the last state (where a valid packet was received) should be restored
			mMisoVars.fFirstFrag = false;
			mMisoVars.fLastFrag = false;
			mMisoVars.fFragmentation = false;
		}
	}

	/* Add a byte counter that can be displayed
	** in the results for easy tracking of specific values */
	if (eState == e_ABCC_MISO_RD_PD_FIELD)
	{
		result_frame.mData2 = (U64)mMisoVars.dwPdCnt;
		mMisoVars.dwPdCnt++;
	}
	else
	{
		mMisoVars.dwPdCnt = 0;
	}

	/* Handle indication of the SPI message fragmentation protocol */
	if (mMisoVars.fFragmentation)
	{
		result_frame.mFlags |= (SPI_MSG_FRAG_FLAG);
		if (mMisoVars.fFirstFrag)
		{
			result_frame.mFlags |= (SPI_MSG_FIRST_FRAG_FLAG);
		}
		if (eState == e_ABCC_MISO_CRC32)
		{
			mMisoVars.fFirstFrag = false;
			if (mMisoVars.fLastFrag)
			{
				mMisoVars.fLastFrag = false;
				mMisoVars.fFragmentation = false;
			}
		}
	}

	/* Commit the processed frame */
	mResults->AddFrame(result_frame);
	mResults->CommitResults();

	if (eState == e_ABCC_MISO_CRC32)
	{
		tPacketType ePacketType = e_NULL_PACKET;
		if ((result_frame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
		{
			if (eState == e_ABCC_MISO_CRC32)
			{
				SetMisoPacketType(e_CHECKSUM_ERROR_PACKET);
			}
			else
			{
				SetMisoPacketType(e_PROTOCOL_ERROR_PACKET);
			}
		}

		if (mMisoVars.fNewMsg)
		{
			if (((result_frame.mFlags & SPI_MSG_FRAG_FLAG) == SPI_MSG_FRAG_FLAG) &&
				!((result_frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG) == SPI_MSG_FIRST_FRAG_FLAG))
			{
				SetMisoPacketType(e_MSG_FRAGMENT_PACKET);
			}
			else
			{
				if (mMisoVars.sMsgHeader.cmd & ABP_MSG_HEADER_C_BIT)
				{
					SetMisoPacketType(e_COMMAND_PACKET);
				}
				else if (mMisoVars.sMsgHeader.cmd & ABP_MSG_HEADER_E_BIT)
				{
					SetMisoPacketType(e_ERROR_RESPONSE_PACKET);
				}
				else
				{
					SetMisoPacketType(e_RESPONSE_PACKET);
				}
			}
		}
		SignalReadyForNewPacket(false);
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
		mMosiVars.sMsgHeader.obj = (U8)lFrameData;
		mMosiVars.dwMdCnt = 0;
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_inst)
	{
		mMosiVars.sMsgHeader.inst = (U16)lFrameData;

		/* Store the object code in frame data to handle object specific data */
		result_frame.mData2 = mMosiVars.sMsgHeader.obj;
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd)
	{
		mMosiVars.sMsgHeader.cmd = (U8)lFrameData;

		/* Store the object code in frame data to handle object specific data */
		result_frame.mData2 = mMosiVars.sMsgHeader.obj;

		if ((lFrameData & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			mMosiVars.fErrorRsp = true;
		}
		else
		{
			mMosiVars.fErrorRsp = false;
		}
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt)
	{
		/* To better analyze the data in bubbletext
		** store the object code, instance, and command */
		memcpy(&result_frame.mData2, &mMosiVars.sMsgHeader, sizeof(mMosiVars.sMsgHeader));
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_size)
	{
		if ((U16)lFrameData > ABP_MAX_MSG_DATA_BYTES)
		{
			/* Max message data size exceeded */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			mMosiVars.wMdSize = 0;
			mMosiVars.fFirstFrag = false;
			mMosiVars.fLastFrag = false;
			mMosiVars.fFragmentation = false;
		}
		else
		{
			mMosiVars.wMdSize = (U16)lFrameData;
		}
	}
	else if (eState == e_ABCC_MOSI_WR_MSG_SUBFIELD_data)
	{
		if (mMosiVars.fErrorRsp)
		{
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			/* Check if data is 0xFF, if so delay de-assertion of fMosiErrorRsp
			** so that the object specific error response can be detected */
			if ((((U8)lFrameData != (U8)0xFF) && (mMosiVars.dwMdCnt == 0)) ||
				(mMosiVars.dwMdCnt > 1))
			{
				mMosiVars.fErrorRsp = false;
			}
		}
		result_frame.mData2 = ((U64)mMosiVars.sMsgHeader.obj) << (8 * sizeof(U32));

		/* Add a byte counter that can be displayed
		** in the results for easy tracking of specific values */
		result_frame.mData2 |= (U64)mMosiVars.dwMdCnt;
		mMosiVars.dwMdCnt++;
		/* Check if the message data counter has reached the end of valid data */
		if (mMosiVars.dwMdCnt > mMosiVars.wMdSize)
		{
			/* Override frame type */
			result_frame.mType = (U8)e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid;
		}
	}
	else if (eState == e_ABCC_MOSI_APP_STAT)
	{
		if (mMosiVars.bLastApplSts != (U8)lFrameData)
		{
			/* Application status change event */
			mMosiVars.bLastApplSts = (U8)lFrameData;
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG);
		}
	}
	else if (eState == e_ABCC_MOSI_SPI_CTRL)
	{
		if (mMosiVars.bLastToggleState == (U8)(lFrameData & ABP_SPI_CTRL_T))
		{
			/* Retransmit event */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG);
		}
		else
		{
			mMosiVars.bLastToggleState = (U8)(lFrameData & ABP_SPI_CTRL_T);
		}
	}
	else if (eState == e_ABCC_MOSI_CRC32)
	{
		/* Save the computed CRC32 to the unused frameData */
		result_frame.mData2 = mMosiVars.oChecksum.Crc32();
		if (result_frame.mData2 != result_frame.mData1)
		{
			/* CRC Error */
			result_frame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			mMosiVars.fFirstFrag = false;
			mMosiVars.fLastFrag = false;
			mMosiVars.fFragmentation = false;
		}
	}

	/* Add a byte counter that can be displayed
	** in the results for easy tracking of specific values */
	if (eState == e_ABCC_MOSI_WR_PD_FIELD)
	{
		result_frame.mData2 = (U64)mMosiVars.dwPdCnt;
		mMosiVars.dwPdCnt++;
	}
	else
	{
		mMosiVars.dwPdCnt = 0;
	}

	/* Handle indication of the SPI message fragmentation protocol */
	if (mMosiVars.fFragmentation)
	{
		result_frame.mFlags |= (SPI_MSG_FRAG_FLAG);
		if (mMosiVars.fFirstFrag)
		{
			result_frame.mFlags |= (SPI_MSG_FIRST_FRAG_FLAG);
		}
		if (eState == e_ABCC_MOSI_PAD)
		{
			mMosiVars.fFirstFrag = false;
			if (mMosiVars.fLastFrag)
			{
				mMosiVars.fLastFrag = false;
				mMosiVars.fFragmentation = false;
			}
		}
	}

	/* Commit the processed frame */
	mResults->AddFrame(result_frame);
	mResults->CommitResults();

	if ((result_frame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
	{
		if (eState == e_ABCC_MOSI_CRC32)
		{
			SetMosiPacketType(e_CHECKSUM_ERROR_PACKET);
		}
		/*else
		{
			SetMosiPacketType(e_PROTOCOL_ERROR_PACKET);
		}*/
	}

	if (eState == e_ABCC_MOSI_PAD)
	{
		if (mMosiVars.fNewMsg)
		{
			if (((result_frame.mFlags & SPI_MSG_FRAG_FLAG) == SPI_MSG_FRAG_FLAG) &&
				!((result_frame.mFlags & SPI_MSG_FIRST_FRAG_FLAG) == SPI_MSG_FIRST_FRAG_FLAG))
			{
				SetMosiPacketType(e_MSG_FRAGMENT_PACKET);
			}
			else
			{
				if (mMosiVars.sMsgHeader.cmd & ABP_MSG_HEADER_C_BIT)
				{
					SetMosiPacketType(e_COMMAND_PACKET);
				}
				else if (mMosiVars.sMsgHeader.cmd & ABP_MSG_HEADER_E_BIT)
				{
					SetMosiPacketType(e_ERROR_RESPONSE_PACKET);
				}
				else
				{
					SetMosiPacketType(e_RESPONSE_PACKET);
				}
			}
		}
		SignalReadyForNewPacket(true);
	}
}

void SpiAnalyzer::SetMosiPacketType(tPacketType ePacketType)
{
	switch (ePacketType)
	{
	case e_NULL_PACKET:
	case e_CANCEL_PACKET:
		mMosiVars.ePacketType = ePacketType;
		break;
	case e_PROTOCOL_ERROR_PACKET:
	case e_CHECKSUM_ERROR_PACKET:
	case e_ERROR_RESPONSE_PACKET:
		if (mMosiVars.ePacketType == e_NULL_PACKET)
		{
			mMosiVars.ePacketType = ePacketType;
		}
		else if (mMosiVars.ePacketType != ePacketType)
		{
			mMosiVars.ePacketType = e_MULTI_ERROR_PACKET;
		}
		break;
	case e_RESPONSE_PACKET:
	case e_COMMAND_PACKET:
	case e_MSG_FRAGMENT_PACKET:
		if (mMosiVars.ePacketType == e_NULL_PACKET)
		{
			mMosiVars.ePacketType = ePacketType;
		}
		else if (IsErrorPacketType(mMosiVars.ePacketType))
		{
			mMosiVars.ePacketType = e_MULTI_ERROR_PACKET;
		}
		else if (mMosiVars.ePacketType != ePacketType)
		{
			mMosiVars.ePacketType = e_MULTI_PACKET;
		}
		break;
	default:
	case e_MULTI_PACKET:
	case e_MULTI_ERROR_PACKET:
		break;
	}
}

void SpiAnalyzer::SetMisoPacketType(tPacketType ePacketType)
{
	switch (ePacketType)
	{
	case e_NULL_PACKET:
	case e_CANCEL_PACKET:
		mMisoVars.ePacketType = ePacketType;
		break;
	case e_PROTOCOL_ERROR_PACKET:
	case e_CHECKSUM_ERROR_PACKET:
	case e_ERROR_RESPONSE_PACKET:
		if (mMisoVars.ePacketType == e_NULL_PACKET)
		{
			mMisoVars.ePacketType = ePacketType;
		}
		else if (mMisoVars.ePacketType != ePacketType)
		{
			mMisoVars.ePacketType = e_MULTI_ERROR_PACKET;
		}
		break;
	case e_RESPONSE_PACKET:
	case e_COMMAND_PACKET:
	case e_MSG_FRAGMENT_PACKET:
		if (mMisoVars.ePacketType == e_NULL_PACKET)
		{
			mMisoVars.ePacketType = ePacketType;
		}
		else if (IsErrorPacketType(mMisoVars.ePacketType))
		{
			mMisoVars.ePacketType = e_MULTI_ERROR_PACKET;
		}
		else if (mMisoVars.ePacketType != ePacketType)
		{
			mMisoVars.ePacketType = e_MULTI_PACKET;
		}
		break;
	default:
	case e_MULTI_PACKET:
	case e_MULTI_ERROR_PACKET:
		break;
	}
}

bool SpiAnalyzer::RunAbccMisoStateMachine(bool fReset, bool fError, U64 lMisoData, S64 lFirstSample)
{
	tAbccMisoStates eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
	tAbccMisoStates eMisoState_Current = e_ABCC_MISO_IDLE;
	bool fAddFrame = false;

	eMisoState_Current = mMisoVars.eState;

	/* If an error is signaled we jump into IDLE and wait to be reset.
	** A reset should be logically signaled when CS# is brought HIGH.
	** This would essentially indicate the begining of a new transaction. */
	if (fError || !IsEnableActive())// || WouldAdvancingTheClockToggleEnable())
	{
		mMisoVars.eState = e_ABCC_MISO_IDLE;
		if (mMisoVars.dwByteCnt == 0)
		{
			mMisoVars.lFramesFirstSample = lFirstSample;
		}
		if (mEnable != NULL)
		{
			AddFragFrame(false, mMisoVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
		}
		else
		{
			/* 3-wire mode fragments exist only when idle gaps are detected too soon. */
			AddFragFrame(false, mMisoVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
		}
		mResults->CommitResults();
		return true;
	}

	if (mMisoVars.eState == e_ABCC_MISO_IDLE)
	{
		mMisoVars.oChecksum.Init();
		mMisoVars.lFrameData = 0;
		mMisoVars.dwByteCnt = 0;
		if (fReset)
		{
			mMisoVars.eState = e_ABCC_MISO_Reserved1;
			eMisoState_Current = mMisoVars.eState;
		}
	}

	if (mMisoVars.dwByteCnt == 0)
	{
		mMisoVars.lFramesFirstSample = lFirstSample;
	}

	mMisoVars.lFrameData |= (lMisoData << (8 * mMisoVars.dwByteCnt));
	mMisoVars.dwByteCnt++;

	if (mMisoVars.eState != e_ABCC_MISO_CRC32)
	{
		mMisoVars.oChecksum.Update((U8*)&lMisoData, 1);
	}

	switch (mMisoVars.eState)
	{
	case e_ABCC_MISO_IDLE:
		/* We wait here until a reset is signaled */
		break;
	case e_ABCC_MISO_Reserved1:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = e_ABCC_MISO_Reserved2;
		}
		break;
	case e_ABCC_MISO_Reserved2:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = e_ABCC_MISO_LED_STAT;
		}
		break;
	case e_ABCC_MISO_LED_STAT:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = e_ABCC_MISO_ANB_STAT;
		}
		break;
	case e_ABCC_MISO_ANB_STAT:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = e_ABCC_MISO_SPI_STAT;
		}
		break;
	case e_ABCC_MISO_SPI_STAT:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			if((mMisoVars.lFrameData & ABP_SPI_STATUS_NEW_PD) == ABP_SPI_STATUS_NEW_PD)
			{
				mMisoVars.fNewRdPd = true;
			}
			else
			{
				mMisoVars.fNewRdPd = false;
			}
			if ((mMisoVars.lFrameData & (ABP_SPI_STATUS_LAST_FRAG | ABP_SPI_STATUS_M)) == ABP_SPI_STATUS_M)
			{
				/* New message but not the last */
				mMisoVars.fNewMsg = true;
				if (!mMisoVars.fFragmentation)
				{
					/* Message fragmentation starts */
					mMisoVars.fFragmentation = true;
					mMisoVars.fFirstFrag = true;
					mMisoVars.fLastFrag = false;
				}
			}
			else if ((mMisoVars.lFrameData & (ABP_SPI_STATUS_LAST_FRAG | ABP_SPI_STATUS_M)) == (ABP_SPI_STATUS_LAST_FRAG | ABP_SPI_STATUS_M))
			{
				/* New message and last */
				mMisoVars.fNewMsg = true;
				if (mMisoVars.fFragmentation)
				{
					/* Message fragmentation ends */
					mMisoVars.fFirstFrag = false;
					mMisoVars.fLastFrag = true;
				}
			}
			else
			{
				/* No new message */
				mMisoVars.fNewMsg = false;
				mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid;
				mMisoVars.dwMdCnt = 0;
				mMisoVars.wMdSize = 0;
			}
			fAddFrame = true;
			mMisoVars.eState = e_ABCC_MISO_NET_TIME;
		}
		break;
	case e_ABCC_MISO_NET_TIME:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			if (mMisoVars.dwMsgLen != 0)
			{
				mMisoVars.eState = e_ABCC_MISO_RD_MSG_FIELD;
				if (mMisoVars.fNewMsg)
				{
					RunAbccMisoMsgSubStateMachine(true, NULL, &eMsgSubState);
				}
			}
			else if (mMisoVars.dwPdLen != 0)
			{
				mMisoVars.eState = e_ABCC_MISO_RD_PD_FIELD;
			}
			else
			{
				mMisoVars.eState = e_ABCC_MISO_CRC32;
			}
		}
		break;
	case e_ABCC_MISO_RD_MSG_FIELD:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			if (mMisoVars.fFragmentation && !mMisoVars.fFirstFrag)
			{
				eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data;
				fAddFrame = true;
			}
			else
			{
				if (!RunAbccMisoMsgSubStateMachine(false, &fAddFrame, &eMsgSubState))
				{
					/* Error */
					mMisoVars.eState = e_ABCC_MISO_IDLE;
				}
			}
			if (mMisoVars.dwMsgLen == 1)
			{
				if (mMisoVars.dwPdLen != 0)
				{
					mMisoVars.eState = e_ABCC_MISO_RD_PD_FIELD;
				}
				else
				{
					mMisoVars.eState = e_ABCC_MISO_CRC32;
				}
			}
			//fAddFrame = true;
			mMisoVars.dwMsgLen--;
		}
		break;
	case e_ABCC_MISO_RD_PD_FIELD:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			if (mMisoVars.dwPdLen == 1)
			{
				mMisoVars.eState = e_ABCC_MISO_CRC32;
			}
			fAddFrame = true;
			mMisoVars.dwPdLen--;
		}
		break;
	case e_ABCC_MISO_CRC32:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = e_ABCC_MISO_IDLE;
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
		mMisoVars.eState = e_ABCC_MISO_IDLE;
		break;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		if (mMisoVars.eState != e_ABCC_MISO_IDLE)
		{
			/* We have a fragmented message */
			if (mEnable != NULL)
			{
				AddFragFrame(false, mMisoVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
			}
			else
			{
				AddFragFrame(false, mMisoVars.lFramesFirstSample, mClock->GetSampleNumber());
			}
			mMisoVars.eState = e_ABCC_MISO_IDLE;
			mMisoVars.lFrameData = 0;
			mMisoVars.dwByteCnt = 0;
			return true;
		}
	}

	if (fAddFrame)
	{
		if (eMisoState_Current == e_ABCC_MISO_RD_MSG_FIELD)
		{
			ProcessMisoFrame(eMsgSubState, mMisoVars.lFrameData, mMisoVars.lFramesFirstSample);
		}
		else
		{
			ProcessMisoFrame(eMisoState_Current, mMisoVars.lFrameData, mMisoVars.lFramesFirstSample);
		}

		/* Reset the state variables */
		mMisoVars.lFrameData = 0;
		mMisoVars.dwByteCnt = 0;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		mMisoVars.eState = e_ABCC_MISO_IDLE;
		mMisoVars.lFrameData = 0;
		mMisoVars.dwByteCnt = 0;
	}

	if (mMisoVars.eState == e_ABCC_MISO_IDLE)
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

	eMosiState_Current = mMosiVars.eState;

	/* If an error is signaled we jump into IDLE and wait to be reset.
	** A reset should be logically signaled when CS# is brought HIGH.
	** This would essentially indicate the begining of a new transaction. */
	if (fError || !IsEnableActive())// || WouldAdvancingTheClockToggleEnable())
	{
		if (mMosiVars.dwByteCnt == 0)
		{
			mMosiVars.lFramesFirstSample = lFirstSample;
		}
		mMosiVars.eState = e_ABCC_MOSI_IDLE;
		if (mEnable != NULL)
		{
			AddFragFrame(true, mMosiVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
		}
		else
		{
			/* 3-wire mode fragments exist only when idle gaps are detected too soon. */
			AddFragFrame(true, mMosiVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
		}
		mResults->CommitResults();
		return true;
	}

	if (mMosiVars.eState == e_ABCC_MOSI_IDLE)
	{
		mMosiVars.oChecksum.Init();
		mMosiVars.lFrameData = 0;
		mMosiVars.dwByteCnt = 0;
		if (fReset)
		{
			mMosiVars.eState = e_ABCC_MOSI_SPI_CTRL;
			eMosiState_Current = mMosiVars.eState;
		}
	}

	if (mMosiVars.dwByteCnt == 0)
	{
		mMosiVars.lFramesFirstSample = lFirstSample;
	}

	mMosiVars.lFrameData |= (lMosiData << (8 * mMosiVars.dwByteCnt));
	mMosiVars.dwByteCnt++;

	if (mMosiVars.eState != e_ABCC_MOSI_CRC32)
	{
		mMosiVars.oChecksum.Update((U8*)&lMosiData, 1);
	}

	switch (mMosiVars.eState)
	{
	case e_ABCC_MOSI_IDLE:
		/* We wait here until a reset is signaled */
		break;
	case e_ABCC_MOSI_SPI_CTRL:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			if((mMosiVars.lFrameData & ABP_SPI_CTRL_WRPD_VALID) == ABP_SPI_CTRL_WRPD_VALID)
			{
				mMosiVars.fWrPdValid = true;
			}
			else
			{
				mMosiVars.fWrPdValid = false;
			}

			if ((mMosiVars.lFrameData & (ABP_SPI_CTRL_LAST_FRAG | ABP_SPI_CTRL_M)) == ABP_SPI_CTRL_M)
			{
				/* New message but not the last */
				mMosiVars.fNewMsg = true;
				if (!mMosiVars.fFragmentation)
				{
					/* Message fragmentation starts */
					mMosiVars.fFragmentation = true;
					mMosiVars.fFirstFrag = true;
					mMosiVars.fLastFrag = false;
				}
			}
			else if ((mMosiVars.lFrameData & (ABP_SPI_CTRL_LAST_FRAG | ABP_SPI_CTRL_M)) == (ABP_SPI_CTRL_LAST_FRAG | ABP_SPI_CTRL_M))
			{
				/* New message and last */
				mMosiVars.fNewMsg = true;
				if (mMosiVars.fFragmentation)
				{
					/* Message fragmentation ends */
					mMosiVars.fFirstFrag = false;
					mMosiVars.fLastFrag = true;
				}
			}
			else
			{
				/* No new message */
				mMosiVars.fNewMsg = false;
				mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid;
				mMosiVars.dwMdCnt = 0;
				mMosiVars.wMdSize = 0;
			}
			fAddFrame = true;
			mMosiVars.eState = e_ABCC_MOSI_RESERVED1;
		}
		break;
	case e_ABCC_MOSI_RESERVED1:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = e_ABCC_MOSI_MSG_LEN;
		}
		break;
	case e_ABCC_MOSI_MSG_LEN:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.dwMsgLen = (U32)mMosiVars.lFrameData * 2;
			mMisoVars.dwMsgLen = mMosiVars.dwMsgLen;
			mMosiVars.eState = e_ABCC_MOSI_PD_LEN;
		}
		break;
	case e_ABCC_MOSI_PD_LEN:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.dwPdLen = (U32)mMosiVars.lFrameData * 2;
			mMisoVars.dwPdLen = mMosiVars.dwPdLen;
			mMosiVars.eState = e_ABCC_MOSI_APP_STAT;
		}
		break;
	case e_ABCC_MOSI_APP_STAT:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = e_ABCC_MOSI_INT_MASK;
		}
		break;
	case e_ABCC_MOSI_INT_MASK:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			if (mMosiVars.dwMsgLen != 0)
			{
				mMosiVars.eState = e_ABCC_MOSI_WR_MSG_FIELD;
				if (mMosiVars.fNewMsg)
				{
					RunAbccMosiMsgSubStateMachine(true, NULL, &eMsgSubState);
				}
			}
			else if (mMosiVars.dwPdLen != 0)
			{
				mMosiVars.eState = e_ABCC_MOSI_WR_PD_FIELD;
			}
			else
			{
				mMosiVars.eState = e_ABCC_MOSI_CRC32;
			}
		}
		break;
	case e_ABCC_MOSI_WR_MSG_FIELD:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			if (mMosiVars.fFragmentation && !mMosiVars.fFirstFrag)
			{
				eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data;
				fAddFrame = true;
			}
			else
			{
				if (!RunAbccMosiMsgSubStateMachine(false, &fAddFrame, &eMsgSubState))
				{
					/* Error */
					mMosiVars.eState = e_ABCC_MOSI_IDLE;
				}
			}
			if (mMosiVars.dwMsgLen == 1)
			{
				if (mMosiVars.dwPdLen != 0)
				{
					mMosiVars.eState = e_ABCC_MOSI_WR_PD_FIELD;
				}
				else
				{
					mMosiVars.eState = e_ABCC_MOSI_CRC32;
				}
			}
			mMosiVars.dwMsgLen--;
		}
		break;
	case e_ABCC_MOSI_WR_PD_FIELD:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			if (mMosiVars.dwPdLen == 1)
			{
				mMosiVars.eState = e_ABCC_MOSI_CRC32;
			}
			fAddFrame = true;
			mMosiVars.dwPdLen--;
		}
		break;
	case e_ABCC_MOSI_CRC32:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = e_ABCC_MOSI_PAD;
		}
		break;
	case e_ABCC_MOSI_PAD:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = e_ABCC_MOSI_IDLE;
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
		mMosiVars.eState = e_ABCC_MOSI_IDLE;
		break;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		if (mMosiVars.eState != e_ABCC_MOSI_IDLE)
		{
			/* We have a fragmented message */
			if (mEnable != NULL)
			{
				AddFragFrame(true, mMosiVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
			}
			else
			{
				AddFragFrame(true, mMosiVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
			}
			mMosiVars.eState = e_ABCC_MOSI_IDLE;
			mMosiVars.lFrameData = 0;
			mMosiVars.dwByteCnt = 0;
			return true;
		}
	}

	if (fAddFrame)
	{
		if (eMosiState_Current == e_ABCC_MOSI_WR_MSG_FIELD)
		{
			ProcessMosiFrame(eMsgSubState, mMosiVars.lFrameData, mMosiVars.lFramesFirstSample);
		}
		else
		{
			ProcessMosiFrame(eMosiState_Current, mMosiVars.lFrameData, mMosiVars.lFramesFirstSample);
		}

		/* Reset the state variables */
		mMosiVars.lFrameData = 0;
		mMosiVars.dwByteCnt = 0;
	}

	if (WouldAdvancingTheClockToggleEnable() == true)
	{
		mMosiVars.eState = e_ABCC_MOSI_IDLE;
		mMosiVars.lFrameData = 0;
		mMosiVars.dwByteCnt = 0;
	}

	if (mMosiVars.eState == e_ABCC_MOSI_IDLE)
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
		mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_size;
		mMisoVars.bByteCnt2 = 0;
		return true;
	}

	if ((pfAddFrame == NULL) || (peMisoMsgSubState == NULL))
	{
		return false;
	}

	*peMisoMsgSubState = mMisoVars.eMsgSubState;
	mMisoVars.bByteCnt2++;

	switch (mMisoVars.eMsgSubState)
	{
	case e_ABCC_MISO_RD_MSG_SUBFIELD_size:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_res1;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_res1:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_srcId;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_srcId:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_obj;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_inst;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_inst:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_cmd;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_cmd:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_res2;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_res2:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_cmdExt:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_data:
	case e_ABCC_MISO_RD_MSG_SUBFIELD_data_not_valid:
		if (mMisoVars.bByteCnt2 >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
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
		mMisoVars.eMsgSubState = e_ABCC_MISO_RD_MSG_SUBFIELD_data;
		return false;
	}

	if (*pfAddFrame == true)
	{
		mMisoVars.bByteCnt2 = 0;
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
		mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_size;
		mMosiVars.bByteCnt2 = 0;
		return true;
	}

	if ((pfAddFrame == NULL) || (peMosiMsgSubState == NULL))
	{
		return false;
	}

	*peMosiMsgSubState = mMosiVars.eMsgSubState;
	mMosiVars.bByteCnt2++;

	switch (mMosiVars.eMsgSubState)
	{
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_size:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_res1;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_res1:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_srcId:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_obj;
		}
		break;
	case e_ABCC_MISO_RD_MSG_SUBFIELD_obj:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_inst;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_inst:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmd:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_res2;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_res2:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_cmdExt:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*pfAddFrame = true;
			mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data;
		}
		break;
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_data:
	case e_ABCC_MOSI_WR_MSG_SUBFIELD_data_not_valid:
		if (mMosiVars.bByteCnt2 >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
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
		mMosiVars.eMsgSubState = e_ABCC_MOSI_WR_MSG_SUBFIELD_data;
		return false;
	}

	if (*pfAddFrame == true)
	{
		mMosiVars.bByteCnt2 = 0;
	}

	return true;
}

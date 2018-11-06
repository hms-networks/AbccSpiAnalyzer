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

#include <cstring>

#include "AbccSpiAnalyzer.h"
#include "AbccSpiAnalyzerSettings.h"
#include "AnalyzerChannelData.h"
#include "AbccCrc.h"

#include "abcc_td.h"
#include "abcc_abp/abp.h"

#define IS_3WIRE_MODE() (((mEnable == nullptr) && (mSettings->m4WireOn3Channels == false)) || (mSettings->m3WireOn4Channels == true))
#define IS_PURE_4WIRE_MODE() ((mEnable != nullptr) && (mSettings->m3WireOn4Channels == false))

inline void SpiAnalyzer::ProcessSample(AnalyzerChannelData* chn_data, DataBuilder& data, Channel& chn)
{
	if (chn_data != nullptr)
	{
		chn_data->AdvanceToAbsPosition(mCurrentSample);
		data.AddBit(chn_data->GetBitState());

		if (chn_data->GetBitState() == BitState::BIT_HIGH)
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::One, chn);
		}
		else
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::Zero, chn);
		}
	}
}

SpiAnalyzer::SpiAnalyzer()
	: Analyzer2(),
	mSettings(new SpiAnalyzerSettings()),
	mSimulationInitialized(false),
	mMosi(nullptr),
	mMiso(nullptr),
	mClock(nullptr),
	mEnable(nullptr)
{
	SetAnalyzerSettings(mSettings.get());

	mSettingsChangeID = mSettings->mChangeID;
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
	GetByteStatus byteStatus;
	StateOperation mosiOperation;
	StateOperation misoOperation;
	AcquisitionStatus acquisitionStatus;
	bool fMosiReady = true;
	bool fMisoReady = true;

	Setup();

	/* Check that all required channels are valid */
	if ( (mMiso != nullptr) && (mMosi != nullptr) && (mClock != nullptr) )
	{
		mMisoVars.oChecksum = AbccCrc();
		mMosiVars.oChecksum = AbccCrc();

		AdvanceToActiveEnableEdgeWithCorrectClockPolarity();

		/* Reset persistent state logic between captures */
		mMosiVars.eState = AbccMosiStates::Idle;
		mMisoVars.eState = AbccMisoStates::Idle;
		mMisoVars.bLastAnbSts = 0xFF;
		mMosiVars.bLastApplSts = 0xFF;

		RunAbccMosiMsgSubStateMachine(StateOperation::Reset, nullptr, nullptr);
		RunAbccMisoMsgSubStateMachine(StateOperation::Reset, nullptr, nullptr);

		for (;;)
		{
			/* The SPI word length is 8-bits. Read 1 byte at a time and run the statemachines */
			byteStatus = GetByte(&lMosiData, &lMisoData, &lFirstSample);
			switch (byteStatus)
			{
			case GetByteStatus::OK:
			case GetByteStatus::Skip:
				acquisitionStatus = AcquisitionStatus::OK;
				mosiOperation = StateOperation::Run;
				misoOperation = StateOperation::Run;
				break;
			case GetByteStatus::Reset:
				acquisitionStatus = AcquisitionStatus::Reset;
				mosiOperation = StateOperation::Reset;
				misoOperation = StateOperation::Reset;
				break;
			default:
			case GetByteStatus::Error:
				acquisitionStatus = AcquisitionStatus::Error;
				mosiOperation = StateOperation::Reset;
				misoOperation = StateOperation::Reset;
				break;
			}

			if (byteStatus != GetByteStatus::Skip)
			{
				if (fMosiReady)
				{
					mosiOperation = StateOperation::Reset;
				}

				if (fMisoReady)
				{
					misoOperation = StateOperation::Reset;
				}

				fMosiReady = RunAbccMosiStateMachine(mosiOperation, acquisitionStatus, lMosiData, lFirstSample);
				fMisoReady = RunAbccMisoStateMachine(misoOperation, acquisitionStatus, lMisoData, lFirstSample);

				if (IS_3WIRE_MODE())
				{
					if (!fMosiReady && !fMisoReady)
					{
						if (Is3WireIdleCondition(MAX_CLOCK_IDLE_HI_TIME))
						{
							mMosiVars.eState = AbccMosiStates::SpiControl;
							mMisoVars.eState = AbccMisoStates::Reserved1;
							mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Size;
							mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Size;
							mMisoVars.oChecksum.Init();
							mMosiVars.oChecksum.Init();
							mMisoVars.lFrameData = 0;
							mMosiVars.lFrameData = 0;
							mMisoVars.dwByteCnt = 0;
							mMosiVars.dwByteCnt = 0;
							acquisitionStatus = AcquisitionStatus::Error;
						}
					}
				}

				if (acquisitionStatus == AcquisitionStatus::Error)
				{
					/* Signal error, do not commit packet */
					SetMosiPacketType(PacketType::Cancel);
					SignalReadyForNewPacket(SpiChannel::MOSI);

				}

				mResults->CommitResults();
			}

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

			/* If false, this function moves to the next enable-active edge. */
			if (IsInitialClockPolarityCorrect())
			{
				break;
			}
			else
			{
				mClock->AdvanceToNextEdge();
			}
		}

		mCurrentSample = mClock->GetSampleNumber();
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
		mMosi = nullptr;
	}

	if (mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mMiso = GetAnalyzerChannelData(mSettings->mMisoChannel);
	}
	else
	{
		mMiso = nullptr;
	}

	if (mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
	}
	else
	{
		mClock = nullptr;
	}

	if (mSettings->mEnableChannel != UNDEFINED_CHANNEL)
	{
		mEnable = GetAnalyzerChannelData(mSettings->mEnableChannel);
	}
	else
	{
		mEnable = nullptr;
	}

	mMosiVars.eState              = AbccMosiStates::Idle;
	mMisoVars.eState              = AbccMisoStates::Idle;
	mMisoVars.bLastAnbSts         = 0xFF;
	mMosiVars.bLastApplSts        = 0xFF;
	mMosiVars.bLastToggleState    = 0xFF;
	mMosiVars.dwMsgLen            = 0;
	mMosiVars.dwMsgLenCnt         = 0;
	mMosiVars.dwPdLen             = 0;
	mMisoVars.dwMsgLen            = 0;
	mMisoVars.dwMsgLenCnt         = 0;
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
	mMisoVars.wMdCnt = 0;
	mMisoVars.wMdSize = 0;

	memset(&mMosiVars.sMsgHeader, 0, sizeof(mMosiVars.sMsgHeader));
	mMosiVars.dwPdCnt = 0;
	mMosiVars.wMdCnt = 0;
	mMosiVars.wMdSize = 0;

	mMisoVars.dwLastTimestamp = 0;

	mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Size;
	mMisoVars.dwByteCnt    = 0;
	mMisoVars.lFrameData   = 0;

	mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Size;
	mMosiVars.dwByteCnt    = 0;
	mMosiVars.lFrameData   = 0;

	mMosiVars.bFrameSizeCnt = 0;
	mMisoVars.bFrameSizeCnt = 0;

	mClockingErrorCount = 0;
}

void SpiAnalyzer::AdvanceToActiveEnableEdge()
{
	if (IS_PURE_4WIRE_MODE())
	{
		if (mEnable->GetBitState() == BitState::BIT_HIGH)
		{
			mEnable->AdvanceToNextEdge();
		}
		else
		{
			mEnable->AdvanceToNextEdge();
			mEnable->AdvanceToNextEdge();
		}

		mClock->AdvanceToAbsPosition(mEnable->GetSampleNumber());
	}

	mCurrentSample = mClock->GetSampleNumber();
}

bool SpiAnalyzer::IsInitialClockPolarityCorrect()
{
	bool correctPolarity = true;

	if (IS_3WIRE_MODE())
	{
		/* In 3-wire clock must idle HIGH */
		if (mClock->GetBitState() == BitState::BIT_LOW)
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::ErrorSquare, mSettings->mClockChannel);
			correctPolarity = false;
		}
	}

	return correctPolarity;
}

bool SpiAnalyzer::WouldAdvancingTheClockToggleEnable()
{
	if (IS_3WIRE_MODE())
	{
		return false;
	}

	if (mEnable != nullptr)
	{
		if (mClock->DoMoreTransitionsExistInCurrentData())
		{
			U64 nextEdge = mClock->GetSampleOfNextEdge();

			return mEnable->WouldAdvancingToAbsPositionCauseTransition(nextEdge);
		}
		else
		{
			return mEnable->DoMoreTransitionsExistInCurrentData();
		}
	}
	else
	{
		return false;
	}
}

bool SpiAnalyzer::Is3WireIdleCondition(float idle_time_condition)
{
	if (mSettings->m4WireOn3Channels)
	{
		return false;
	}

	UINT64 lSampleDistance = mClock->GetSampleOfNextEdge() - mClock->GetSampleNumber();
	U32 dwSampleRate = GetSampleRate();
	float rIdleTime = (float)lSampleDistance / (float)dwSampleRate;
	return (rIdleTime >= idle_time_condition);
}

GetByteStatus SpiAnalyzer::GetByte(U64* mosi_data_ptr, U64* miso_data_ptr, U64* first_sample_ptr)
{
	/* We're assuming we come into this function with the clock in the idle state */
	const U32 dwBitsPerTransfer = 8;
	DataBuilder mosiResult;
	DataBuilder misoResult;
	GetByteStatus byteStatus = GetByteStatus::OK;
	bool fClkIdleHigh = false;

	mosiResult.Reset(mosi_data_ptr, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);
	misoResult.Reset(miso_data_ptr, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);
	mArrowLocations.clear();

	*first_sample_ptr = mClock->GetSampleNumber();

	for (auto bitIndex = 0; bitIndex < dwBitsPerTransfer; bitIndex++)
	{
		/* On every single edge, we need to check that "enable" doesn't toggle. */
		/* Note that we can't just advance the enable line to the next edge, because there may not be another edge */

		if (WouldAdvancingTheClockToggleEnable())
		{
			if (bitIndex == 0)
			{
				/* Simply advance forward to next transaction */
				AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				byteStatus = GetByteStatus::Skip;
			}
			else
			{
				/* The enable state changed in the middle of acquiring a byte.
				** Suggests we are not byte-synchronized. */
				byteStatus = GetByteStatus::Reset;
			}

			break;
		}

		if (bitIndex == 0)
		{
			if (mClock->GetBitState() == BitState::BIT_HIGH)
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
				if (bitIndex == 0)
				{
					/* Simply advance forward to next transaction */
					AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				}
				else
				{
					/* Reset everything and return. */
					byteStatus = GetByteStatus::Error;
					break;
				}
			}
		}

		/* Jump to the next clock phase */
		mClock->AdvanceToNextEdge();

		if (!fClkIdleHigh)
		{
			/* Sample on leading edge */
			mCurrentSample = mClock->GetSampleNumber();
			ProcessSample(mMosi, mosiResult, mSettings->mMosiChannel);
			ProcessSample(mMiso, misoResult, mSettings->mMisoChannel);
		}

		if (bitIndex == 0)
		{
			/* Latch the first sample point in the byte */
			*first_sample_ptr = mClock->GetSampleNumber();
		}

		if (IS_3WIRE_MODE())
		{
			/* In 3-wire mode idle condition is >=5us (during a transaction).
			** On every advancement on clock, check for idle condition.
			** If detected, a reset of the statemachines are need to re-sync */
			if (Is3WireIdleCondition(MAX_CLOCK_IDLE_HI_TIME))
			{
				/* Error: reset everything and return. */
				byteStatus = GetByteStatus::Error;
				break;
			}
		}
		else if (WouldAdvancingTheClockToggleEnable())
		{
			if (clkIdleHigh && (bitIndex == 0))
			{
				/* Simply advance forward to next transaction */
				AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				byteStatus = GetByteStatus::Skip;
			}
			else
			{
				/* Error: reset everything and return. */
				byteStatus = GetByteStatus::Error;
			}

			break;
		}

		/* Jump to the next clock phase */
		mClock->AdvanceToNextEdge();

		if (fClkIdleHigh)
		{
			/* Sample on tailing edge */
			mCurrentSample = mClock->GetSampleNumber();
			ProcessSample(mMosi, mosiResult, mSettings->mMosiChannel);
			ProcessSample(mMiso, misoResult, mSettings->mMisoChannel);
		}

		mArrowLocations.push_back(mCurrentSample);
	}

	if (byteStatus == GetByteStatus::OK)
	{
		/* Add sample markers to the results */
		const AnalyzerResults::MarkerType mArrowMarker = AnalyzerResults::UpArrow;
		U32 count = (U32)mArrowLocations.size();
		for (U32 bitIndex = 0; bitIndex < count; bitIndex++)
		{
			mResults->AddMarker(mArrowLocations[bitIndex], mArrowMarker, mSettings->mClockChannel);
		}
	}

	mResults->CommitResults();

	return byteStatus;
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
	if (IS_3WIRE_MODE())
	{
		/* In 3-wire mode, there is a requirement for the maximum time the
		** clock can idle high during a transfer of 5us. This means the
		** minimum SPI clock frequency supported in 3-wire mode is 100Khz.
		** Nyquist rate means sampling above 200kHz is required,
		** use 3x as the minimum supported option. */
		return 300000;
	}
	else
	{
		/* In 4-wire mode, use Logic's lowest supported sample rate. */
		return 10000;
	}
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
		return (mEnable->GetBitState() == BitState::BIT_LOW);
	}
	else
	{
		return true;
	}
}

static bool IsErrorPacketType(PacketType packet_type)
{
	switch (packet_type)
	{
	case PacketType::ProtocolError:
	case PacketType::ChecksumError:
	case PacketType::ErrorResponse:
	case PacketType::MultiEventWithError:
	case PacketType::Cancel:
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
	if ((mMosiVars.ePacketType != PacketType::Empty) && (mMisoVars.ePacketType != PacketType::Empty))
	{
		/* Multiple events (at least one on each channel, or multiple events on one channel) */
		if (IsErrorPacketType(mMosiVars.ePacketType) ||
			IsErrorPacketType(mMisoVars.ePacketType))
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
		/* Only one channel contains an event(s) */
		if ((mMosiVars.ePacketType == PacketType::MultiEventWithError) ||
			(mMisoVars.ePacketType == PacketType::MultiEventWithError))
		{
			eMarkerType = AnalyzerResults::ErrorSquare;
		}
		else if ((mMosiVars.ePacketType == PacketType::ProtocolError) ||
				 (mMisoVars.ePacketType == PacketType::ProtocolError))
		{
			eMarkerType = AnalyzerResults::ErrorSquare;
		}
		else if ((mMosiVars.ePacketType == PacketType::ChecksumError) ||
				 (mMisoVars.ePacketType == PacketType::ChecksumError))
		{
			eMarkerType = AnalyzerResults::ErrorX;
		}
		else if ((mMosiVars.ePacketType == PacketType::ErrorResponse) ||
				 (mMisoVars.ePacketType == PacketType::ErrorResponse))
		{
			eMarkerType = AnalyzerResults::ErrorDot;
		}
		else if ((mMosiVars.ePacketType == PacketType::MultiEvent) ||
				 (mMisoVars.ePacketType == PacketType::MultiEvent))
		{
			eMarkerType = AnalyzerResults::Square;
		}
		else if ((mMosiVars.ePacketType == PacketType::Response) ||
				 (mMisoVars.ePacketType == PacketType::Response))
		{
			eMarkerType = AnalyzerResults::Stop;
		}
		else if ((mMosiVars.ePacketType == PacketType::Command) ||
				 (mMisoVars.ePacketType == PacketType::Command))
		{
			eMarkerType = AnalyzerResults::Start;
		}
		else if ((mMosiVars.ePacketType == PacketType::MessageFragment) ||
				 (mMisoVars.ePacketType == PacketType::MessageFragment))
		{
			eMarkerType = AnalyzerResults::Dot;
		}
		else if ((mMosiVars.ePacketType == PacketType::Empty) ||
				 (mMisoVars.ePacketType == PacketType::Empty))
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

void SpiAnalyzer::SignalReadyForNewPacket(SpiChannel_t e_channel)
{
	bool fStartNewPacket = false;
	if (e_channel == SpiChannel::MOSI)
	{
		mMosiVars.fReadyForNewPacket = true;
	}
	else
	{
		mMisoVars.fReadyForNewPacket = true;
	}

	if (mMosiVars.ePacketType == PacketType::Cancel)
	{
		fStartNewPacket = true;
		mResults->CancelPacketAndStartNewPacket();
		if (mEnable != nullptr)
		{
			mResults->AddMarker(mCurrentSample, AnalyzerResults::ErrorX, mSettings->mEnableChannel);
		}
	}
	else if (mMisoVars.fReadyForNewPacket && mMosiVars.fReadyForNewPacket)
	{
		U64 packetId = mResults->CommitPacketAndStartNewPacket();
		fStartNewPacket = true;
		if (packetId == INVALID_RESULT_INDEX)
		{
			if (mEnable != nullptr)
			{
				mResults->AddMarker(mCurrentSample, AnalyzerResults::Zero, mSettings->mEnableChannel);
			}
		}
		else
		{
			if (mEnable != nullptr)
			{
				AnalyzerResults::MarkerType eMarkerType = GetPacketMarkerType();

				if (eMarkerType != AnalyzerResults::One)
				{
					mResults->AddMarker(mCurrentSample, eMarkerType, mSettings->mEnableChannel);
				}
			}
		}

		mResults->CommitResults();
		/* check if the source id is new */
		/* if new source id, allocate a new transaction id */
		/* if not a new source id, check that the header information matches the one in progress */
		/* if header information does not match, flag an error in the current frame */
	}
	if (fStartNewPacket)
	{
		if (mMosiVars.ePacketType != PacketType::Cancel)
		{
			// Check if any additional clocks appear on SCLK before enable goes inactive
			CheckForIdleAfterPacket();
		}

		mMosiVars.fReadyForNewPacket = false;
		mMisoVars.fReadyForNewPacket = false;
		mMosiVars.ePacketType = PacketType::Empty;
		mMisoVars.ePacketType = PacketType::Empty;
	}
}

void SpiAnalyzer::CheckForIdleAfterPacket(void)
{
	Frame errorFrame;
	U64 markerSample = 0;
	Channel chn;
	bool fAddError = false;

	if (IS_PURE_4WIRE_MODE())
	{
		U64 lNextSample = mEnable->GetSampleOfNextEdge();
		if (lNextSample <= mClock->GetSampleNumber())
		{
			mEnable->AdvanceToAbsPosition(mClock->GetSampleNumber());
			lNextSample = mEnable->GetSampleOfNextEdge();
		}

		if (mClock->WouldAdvancingToAbsPositionCauseTransition(lNextSample))
		{
			U32 maxAllowedTransitions = 0;
			U32 transitionCount;

			if(mClock->GetBitState() == BitState::BIT_HIGH)
			{
				maxAllowedTransitions = 1;
			}

			errorFrame.mStartingSampleInclusive = mClock->GetSampleOfNextEdge();
			transitionCount = mClock->AdvanceToAbsPosition(lNextSample);

			if (transitionCount > maxAllowedTransitions)
			{
				chn = mSettings->mEnableChannel;
				errorFrame.mEndingSampleInclusive = mEnable->GetSampleOfNextEdge();
				markerSample = errorFrame.mEndingSampleInclusive;
				fAddError = true;
			}
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
				errorFrame.mStartingSampleInclusive = mClock->GetSampleOfNextEdge();
				AdvanceToActiveEnableEdgeWithCorrectClockPolarity();
				errorFrame.mEndingSampleInclusive = mClock->GetSampleOfNextEdge();
				markerSample = errorFrame.mStartingSampleInclusive + (errorFrame.mEndingSampleInclusive - errorFrame.mStartingSampleInclusive) / 2;
				fAddError = true;
			}
		}
	}

	if (fAddError)
	{
		if ((mSettings->mClockingAlertLimit < 0) ||
			(mClockingErrorCount < mSettings->mClockingAlertLimit))
		{
			mClockingErrorCount++;
			errorFrame.mFlags = (SPI_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG);
			errorFrame.mType = AbccSpiError::EndOfTransfer;
			mResults->AddFrame(errorFrame);
			mResults->AddMarker(markerSample, AnalyzerResults::ErrorSquare, chn);
		}
	}
}

void SpiAnalyzer::AddFragFrame(SpiChannel_t e_channel, U64 first_sample, U64 last_sample)
{
	Frame errorFrame;
	errorFrame.mStartingSampleInclusive = first_sample;
	errorFrame.mEndingSampleInclusive = last_sample;
	errorFrame.mData1 = 0;
	errorFrame.mType = AbccSpiError::Fragmentation;
	errorFrame.mFlags = (SPI_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG);

	if (e_channel == SpiChannel::MOSI)
	{
		errorFrame.mFlags |= SPI_MOSI_FLAG;

		/* Only apply marker from MOSI, this prevents multiple markers at the same spot
		** in such instances draw distance is reduced significantly. */
		if (mEnable != nullptr)
		{
			mResults->AddMarker(last_sample, AnalyzerResults::ErrorSquare, mSettings->mEnableChannel);
		}
		else
		{
			U64 markerSample = first_sample + (last_sample - first_sample) / 2;
			mResults->AddMarker(markerSample, AnalyzerResults::ErrorSquare, mSettings->mClockChannel);
		}
	}

	mResults->AddFrame(errorFrame);

	SignalReadyForNewPacket(e_channel);
	RestorePreviousStateVars();
}

void SpiAnalyzer::ProcessMisoFrame(AbccMisoStates::Enum e_state, U64 frame_data, S64 frames_first_sample)
{
	Frame resultFrame;
	resultFrame.mFlags = 0x00;
	resultFrame.mType = (U8)e_state;
	resultFrame.mStartingSampleInclusive = frames_first_sample;
	resultFrame.mEndingSampleInclusive = (S64)mClock->GetSampleNumber();
	resultFrame.mData1 = frame_data;


	if (e_state == AbccMisoStates::MessageField_Object)
	{
		mMisoVars.sMsgHeader.obj = (U8)frame_data;
		mMisoVars.wMdCnt = 0;
	}
	else if (e_state == AbccMisoStates::MessageField_Instance)
	{
		mMisoVars.sMsgHeader.inst = (U16)frame_data;

		/* Store the object code in frame data to handle object specific data */
		resultFrame.mData2 = mMisoVars.sMsgHeader.obj;
	}
	else if (e_state == AbccMisoStates::MessageField_Command)
	{
		mMisoVars.sMsgHeader.cmd = (U8)frame_data;

		/* Store the object code in frame data to handle object specific data */
		resultFrame.mData2 = mMisoVars.sMsgHeader.obj;

		if ((frame_data & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			mMisoVars.fErrorRsp = true;
		}
		else
		{
			mMisoVars.fErrorRsp = false;
		}
	}
	else if (e_state == AbccMisoStates::MessageField_CommandExtension)
	{
		/* To better analyze the data in bubbletext
		** store the object code, instance, and command */
		memcpy(&resultFrame.mData2, &mMisoVars.sMsgHeader, sizeof(mMisoVars.sMsgHeader));
		mMisoVars.sMsgHeader.cmdExt = (U16)frame_data;
	}
	else if (e_state == AbccMisoStates::MessageField_Size)
	{
		if ((U16)frame_data > ABP_MAX_MSG_DATA_BYTES)
		{
			/* Max message data size exceeded */
			resultFrame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			mMisoVars.wMdSize = 0;
			mMisoVars.fFirstFrag = false;
			mMisoVars.fLastFrag = false;
			mMisoVars.fFragmentation = false;
		}
		else
		{
			mMisoVars.wMdSize = (U16)frame_data;
		}
	}
	else if (e_state == AbccMisoStates::MessageField_Data)
	{
		MsgDataFrameData2_t* psFrameData2 = (MsgDataFrameData2_t*)&resultFrame.mData2;

		if (mMisoVars.fErrorRsp)
		{
			resultFrame.mFlags |= SPI_PROTO_EVENT_FLAG;
			/* Check if data is 0xFF, if so delay de-assertion of fErrorRsp
			** so that the object specific error response can be detected */
			if ((((U8)frame_data != (U8)0xFF) && (mMisoVars.wMdCnt == 0)) ||
				(mMisoVars.wMdCnt > 1))
			{
				mMisoVars.fErrorRsp = false;
			}
		}

		/* Copy message header info to frame data so that the display of the
		** data can be adapted based on the provided information. */
		memcpy(&psFrameData2->msgHeader,
			&mMisoVars.sMsgHeader, sizeof(mMisoVars.sMsgHeader));

		/* Add a byte counter that can be displayed
		** in the results for easy tracking of specific values */
		psFrameData2->msgDataCnt = mMisoVars.wMdCnt;
		mMisoVars.wMdCnt++;
		/* Check if the message data counter has reached the end of valid data */
		if (mMisoVars.wMdCnt > mMisoVars.wMdSize)
		{
			/* Override frame type */
			resultFrame.mType = (U8)AbccMisoStates::MessageField_DataNotValid;
		}
	}
	else if (e_state == AbccMisoStates::AnybusStatus)
	{
		if (mMisoVars.bLastAnbSts != (U8)frame_data)
		{
			/* Anybus status change event */
			mMisoVars.bLastAnbSts = (U8)frame_data;
			resultFrame.mFlags |= SPI_PROTO_EVENT_FLAG;
		}
	}
	else if (e_state == AbccMisoStates::NetworkTime)
	{
		/* Compute delta from last timestamp and save it */
		((NetworkTimeInfo_t*)&resultFrame.mData2)->deltaTime = (U32)resultFrame.mData1 - mMisoVars.dwLastTimestamp;
		((NetworkTimeInfo_t*)&resultFrame.mData2)->newRdPd = mMisoVars.fNewRdPd;
		((NetworkTimeInfo_t*)&resultFrame.mData2)->wrPdValid = mMosiVars.fWrPdValid;
		mMisoVars.fNewRdPd = false;
		mMosiVars.fWrPdValid = false;
		mMisoVars.dwLastTimestamp = (U32)resultFrame.mData1;
	}
	else if (e_state == AbccMisoStates::SpiStatus)
	{
		if ((U8)(frame_data & ABP_SPI_STATUS_WRMSG_FULL))
		{
			/* Write message buffer is full, possible overrun */
			resultFrame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_WARNING_FLAG);
		}
	}
	else if (e_state == AbccMisoStates::Crc32)
	{
		/* Save the computed CRC32 to the unused frame data */
		resultFrame.mData2 = mMisoVars.oChecksum.Crc32();
		if (resultFrame.mData2 != resultFrame.mData1)
		{
			/* CRC Error */
			resultFrame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
		}
	}

	/* Add a byte counter that can be displayed
	** in the results for easy tracking of specific values */
	if (e_state == AbccMisoStates::ReadProcessData)
	{
		resultFrame.mData2 = (U64)mMisoVars.dwPdCnt;
		mMisoVars.dwPdCnt++;
	}
	else
	{
		mMisoVars.dwPdCnt = 0;
	}

	/* Handle indication of the SPI message fragmentation protocol */
	if (mMisoVars.fFragmentation)
	{
		resultFrame.mFlags |= (SPI_MSG_FRAG_FLAG);
		if (mMisoVars.fFirstFrag)
		{
			resultFrame.mFlags |= (SPI_MSG_FIRST_FRAG_FLAG);
		}
		if (e_state == AbccMisoStates::Crc32)
		{
			mMisoVars.fFirstFrag = false;
			if (mMisoVars.fLastFrag)
			{
				mMisoVars.fLastFrag = false;
				mMisoVars.fFragmentation = false;
			}
		}
	}

	if (e_state == AbccMisoStates::Crc32)
	{
		if ((resultFrame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
		{
			SetMisoPacketType(PacketType::ChecksumError);
			RestorePreviousStateVars();
		}
		else
		{
			/* Backup state variables for both MOSI and MISO */
			memcpy(&mPreviousMisoVars, &mMisoVars, sizeof(MisoVars_t));
			memcpy(&mPreviousMosiVars, &mMosiVars, sizeof(MosiVars_t));
		}

		if (mMisoVars.fNewMsg)
		{
			if (((resultFrame.mFlags & SPI_MSG_FRAG_FLAG) == SPI_MSG_FRAG_FLAG) &&
				!((resultFrame.mFlags & SPI_MSG_FIRST_FRAG_FLAG) == SPI_MSG_FIRST_FRAG_FLAG))
			{
				SetMisoPacketType(PacketType::MessageFragment);
			}
			else
			{
				if (mMisoVars.sMsgHeader.cmd & ABP_MSG_HEADER_C_BIT)
				{
					SetMisoPacketType(PacketType::Command);
				}
				else if (mMisoVars.sMsgHeader.cmd & ABP_MSG_HEADER_E_BIT)
				{
					SetMisoPacketType(PacketType::ErrorResponse);
				}
				else
				{
					SetMisoPacketType(PacketType::Response);
				}
			}
		}
	}
	else if ((resultFrame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
	{
		SetMisoPacketType(PacketType::ProtocolError);
	}

	/* Commit the processed frame */
	mResults->AddFrame(resultFrame);
	mResults->CommitResults();

	if (e_state == AbccMisoStates::Crc32)
	{
		SignalReadyForNewPacket(SpiChannel::MISO);
	}
}

void SpiAnalyzer::ProcessMosiFrame(AbccMosiStates::Enum e_state, U64 frame_data, S64 frames_first_sample)
{
	Frame resultFrame;
	resultFrame.mFlags = SPI_MOSI_FLAG;
	resultFrame.mType = (U8)e_state;
	resultFrame.mStartingSampleInclusive = frames_first_sample;
	resultFrame.mEndingSampleInclusive = (S64)mClock->GetSampleNumber();
	resultFrame.mData1 = frame_data;


	if (e_state == AbccMosiStates::MessageField_Object)
	{
		mMosiVars.sMsgHeader.obj = (U8)frame_data;
		mMosiVars.wMdCnt = 0;
	}
	else if (e_state == AbccMosiStates::MessageField_Instance)
	{
		mMosiVars.sMsgHeader.inst = (U16)frame_data;

		/* Store the object code in frame data to handle object specific data */
		resultFrame.mData2 = mMosiVars.sMsgHeader.obj;
	}
	else if (e_state == AbccMosiStates::MessageField_Command)
	{
		mMosiVars.sMsgHeader.cmd = (U8)frame_data;

		/* Store the object code in frame data to handle object specific data */
		resultFrame.mData2 = mMosiVars.sMsgHeader.obj;

		if ((frame_data & ABP_MSG_HEADER_E_BIT) == ABP_MSG_HEADER_E_BIT)
		{
			mMosiVars.fErrorRsp = true;
		}
		else
		{
			mMosiVars.fErrorRsp = false;
		}
	}
	else if (e_state == AbccMosiStates::MessageField_CommandExtension)
	{
		/* To better analyze the data in bubbletext
		** store the object code, instance, and command */
		memcpy(&resultFrame.mData2, &mMosiVars.sMsgHeader, sizeof(mMosiVars.sMsgHeader));
		mMosiVars.sMsgHeader.cmdExt = (U16)frame_data;
	}
	else if (e_state == AbccMosiStates::MessageField_Size)
	{
		if ((U16)frame_data > ABP_MAX_MSG_DATA_BYTES)
		{
			/* Max message data size exceeded */
			resultFrame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
			mMosiVars.wMdSize = 0;
			mMosiVars.fFirstFrag = false;
			mMosiVars.fLastFrag = false;
			mMosiVars.fFragmentation = false;
		}
		else
		{
			mMosiVars.wMdSize = (U16)frame_data;
		}
	}
	else if (e_state == AbccMosiStates::MessageField_Data)
	{
		MsgDataFrameData2_t* psFrameData2 = (MsgDataFrameData2_t*)&resultFrame.mData2;

		if (mMosiVars.fErrorRsp)
		{
			resultFrame.mFlags |= SPI_PROTO_EVENT_FLAG;
			/* Check if data is 0xFF, if so delay de-assertion of fErrorRsp
			** so that the object specific error response can be detected */
			if ((((U8)frame_data != (U8)0xFF) && (mMosiVars.wMdCnt == 0)) ||
				(mMosiVars.wMdCnt > 1))
			{
				mMosiVars.fErrorRsp = false;
			}
		}

		/* Copy message header info to frame data so that the display of the
		** data can be adapted based on the provided information. */
		memcpy(&psFrameData2->msgHeader,
			&mMosiVars.sMsgHeader, sizeof(mMosiVars.sMsgHeader));

		/* Add a byte counter that can be displayed
		** in the results for easy tracking of specific values */
		psFrameData2->msgDataCnt = mMosiVars.wMdCnt;
		mMosiVars.wMdCnt++;
		/* Check if the message data counter has reached the end of valid data */
		if (mMosiVars.wMdCnt > mMosiVars.wMdSize)
		{
			/* Override frame type */
			resultFrame.mType = (U8)AbccMosiStates::MessageField_DataNotValid;
		}
	}
	else if (e_state == AbccMosiStates::ApplicationStatus)
	{
		if (mMosiVars.bLastApplSts != (U8)frame_data)
		{
			/* Application status change event */
			mMosiVars.bLastApplSts = (U8)frame_data;
			resultFrame.mFlags |= SPI_PROTO_EVENT_FLAG;
		}
	}
	else if (e_state == AbccMosiStates::SpiControl)
	{
		if (mMosiVars.bLastToggleState == (U8)(frame_data & ABP_SPI_CTRL_T))
		{
			/* Retransmit event */
			resultFrame.mFlags |= SPI_PROTO_EVENT_FLAG;
		}
		else
		{
			mMosiVars.bLastToggleState = (U8)(frame_data & ABP_SPI_CTRL_T);
		}
	}
	else if (e_state == AbccMosiStates::Crc32)
	{
		/* Save the computed CRC32 to the unused frame data */
		resultFrame.mData2 = mMosiVars.oChecksum.Crc32();
		if (resultFrame.mData2 != resultFrame.mData1)
		{
			/* CRC Error */
			resultFrame.mFlags |= (SPI_PROTO_EVENT_FLAG | DISPLAY_AS_ERROR_FLAG);
		}
	}

	/* Add a byte counter that can be displayed
	** in the results for easy tracking of specific values */
	if (e_state == AbccMosiStates::WriteProcessData)
	{
		resultFrame.mData2 = (U64)mMosiVars.dwPdCnt;
		mMosiVars.dwPdCnt++;
	}
	else
	{
		mMosiVars.dwPdCnt = 0;
	}

	/* Handle indication of the SPI message fragmentation protocol */
	if (mMosiVars.fFragmentation)
	{
		resultFrame.mFlags |= (SPI_MSG_FRAG_FLAG);
		if (mMosiVars.fFirstFrag)
		{
			resultFrame.mFlags |= SPI_MSG_FIRST_FRAG_FLAG;
		}
		if (e_state == AbccMosiStates::Pad)
		{
			mMosiVars.fFirstFrag = false;
			if (mMosiVars.fLastFrag)
			{
				mMosiVars.fLastFrag = false;
				mMosiVars.fFragmentation = false;
			}
		}
	}

	if ((resultFrame.mFlags & DISPLAY_AS_ERROR_FLAG) == DISPLAY_AS_ERROR_FLAG)
	{
		if (e_state == AbccMosiStates::Crc32)
		{
			SetMosiPacketType(PacketType::ChecksumError);
		}
		}

	if (e_state == AbccMosiStates::Pad)
	{
		if (mMosiVars.fNewMsg)
		{
			if (((resultFrame.mFlags & SPI_MSG_FRAG_FLAG) == SPI_MSG_FRAG_FLAG) &&
				!((resultFrame.mFlags & SPI_MSG_FIRST_FRAG_FLAG) == SPI_MSG_FIRST_FRAG_FLAG))
			{
				SetMosiPacketType(PacketType::MessageFragment);
			}
			else
			{
				if (mMosiVars.sMsgHeader.cmd & ABP_MSG_HEADER_C_BIT)
				{
					SetMosiPacketType(PacketType::Command);
				}
				else if (mMosiVars.sMsgHeader.cmd & ABP_MSG_HEADER_E_BIT)
				{
					SetMosiPacketType(PacketType::ErrorResponse);
				}
				else
				{
					SetMosiPacketType(PacketType::Response);
				}
			}
		}
	}

	/* Commit the processed frame */
	mResults->AddFrame(resultFrame);
	mResults->CommitResults();

	if (e_state == AbccMosiStates::Pad)
	{
		SignalReadyForNewPacket(SpiChannel::MOSI);
	}
}

void SpiAnalyzer::SetMosiPacketType(PacketType packet_type)
{
	switch (packet_type)
	{
	case PacketType::Empty:
	case PacketType::Cancel:
		mMosiVars.ePacketType = packet_type;
		break;
	case PacketType::ProtocolError:
	case PacketType::ChecksumError:
	case PacketType::ErrorResponse:
		if (mMosiVars.ePacketType == PacketType::Empty)
		{
			mMosiVars.ePacketType = packet_type;
		}
		else if (mMosiVars.ePacketType != packet_type)
		{
			mMosiVars.ePacketType = PacketType::MultiEventWithError;
		}
		break;
	case PacketType::Response:
	case PacketType::Command:
	case PacketType::MessageFragment:
		if (mMosiVars.ePacketType == PacketType::Empty)
		{
			mMosiVars.ePacketType = packet_type;
		}
		else if (IsErrorPacketType(mMosiVars.ePacketType))
		{
			mMosiVars.ePacketType = PacketType::MultiEventWithError;
		}
		else if (mMosiVars.ePacketType != packet_type)
		{
			mMosiVars.ePacketType = PacketType::MultiEvent;
		}
		break;
	default:
	case PacketType::MultiEvent:
	case PacketType::MultiEventWithError:
		break;
	}
}

void SpiAnalyzer::SetMisoPacketType(PacketType packet_type)
{
	switch (packet_type)
	{
	case PacketType::Empty:
	case PacketType::Cancel:
		mMisoVars.ePacketType = packet_type;
		break;
	case PacketType::ProtocolError:
	case PacketType::ChecksumError:
	case PacketType::ErrorResponse:
		if (mMisoVars.ePacketType == PacketType::Empty)
		{
			mMisoVars.ePacketType = packet_type;
		}
		else if (mMisoVars.ePacketType != packet_type)
		{
			mMisoVars.ePacketType = PacketType::MultiEventWithError;
		}
		break;
	case PacketType::Response:
	case PacketType::Command:
	case PacketType::MessageFragment:
		if (mMisoVars.ePacketType == PacketType::Empty)
		{
			mMisoVars.ePacketType = packet_type;
		}
		else if (IsErrorPacketType(mMisoVars.ePacketType))
		{
			mMisoVars.ePacketType = PacketType::MultiEventWithError;
		}
		else if (mMisoVars.ePacketType != packet_type)
		{
			mMisoVars.ePacketType = PacketType::MultiEvent;
		}
		break;
	default:
	case PacketType::MultiEvent:
	case PacketType::MultiEventWithError:
		break;
	}
}

bool SpiAnalyzer::RunAbccMisoStateMachine(StateOperation operation, AcquisitionStatus acquisition_status, U64 miso_data, S64 first_sample)
{
	AbccMisoStates::Enum eMsgSubState = AbccMisoStates::MessageField_Size;
	AbccMisoStates::Enum eMisoState_Current = AbccMisoStates::Idle;
	bool fAddFrame = false;

	eMisoState_Current = mMisoVars.eState;

	/* If an error is signaled we jump into IDLE and wait to be reset.
	** A reset should be logically signaled when CS# is brought HIGH.
	** This would essentially indicate the begining of a new transaction. */
	if ((acquisition_status == AcquisitionStatus::Error) || !IsEnableActive())// || WouldAdvancingTheClockToggleEnable())
	{
		mMisoVars.eState = AbccMisoStates::Idle;
		if (mMisoVars.dwByteCnt == 0)
		{
			mMisoVars.lFramesFirstSample = first_sample;
		}
		if (mEnable != nullptr)
		{
			AddFragFrame(SpiChannel::MISO, mMisoVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
		}
		else
		{
			/* 3-wire mode fragments exist only when idle gaps are detected too soon. */
			AddFragFrame(SpiChannel::MISO, mMisoVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
		}
		mResults->CommitResults();
		return true;
	}

	if (mMisoVars.eState == AbccMisoStates::Idle)
	{
		mMisoVars.oChecksum.Init();
		mMisoVars.lFrameData = 0;
		mMisoVars.dwByteCnt = 0;
		if (operation == StateOperation::Reset)
		{
			mMisoVars.eState = AbccMisoStates::Reserved1;
			eMisoState_Current = mMisoVars.eState;
		}
	}

	if (mMisoVars.dwByteCnt == 0)
	{
		mMisoVars.lFramesFirstSample = first_sample;
	}

	mMisoVars.lFrameData |= (miso_data << (8 * mMisoVars.dwByteCnt));
	mMisoVars.dwByteCnt++;

	if (mMisoVars.eState != AbccMisoStates::Crc32)
	{
		mMisoVars.oChecksum.Update((U8*)&miso_data, 1);
	}

	switch (mMisoVars.eState)
	{
	case AbccMisoStates::Idle:
		/* We wait here until a reset is signaled */
		break;
	case AbccMisoStates::Reserved1:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = AbccMisoStates::Reserved2;
		}
		break;
	case AbccMisoStates::Reserved2:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = AbccMisoStates::LedStatus;
		}
		break;
	case AbccMisoStates::LedStatus:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = AbccMisoStates::AnybusStatus;
		}
		break;
	case AbccMisoStates::AnybusStatus:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = AbccMisoStates::SpiStatus;
		}
		break;
	case AbccMisoStates::SpiStatus:
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
				mMisoVars.eMsgSubState = AbccMisoStates::MessageField_DataNotValid;
				mMisoVars.wMdCnt = 0;
				mMisoVars.wMdSize = 0;
			}
			fAddFrame = true;
			mMisoVars.eState = AbccMisoStates::NetworkTime;
		}
		break;
	case AbccMisoStates::NetworkTime:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			if (mMisoVars.dwMsgLenCnt != 0)
			{
				mMisoVars.eState = AbccMisoStates::MessageField;
				if (mMisoVars.fNewMsg)
				{
					RunAbccMisoMsgSubStateMachine(StateOperation::Reset, nullptr, &eMsgSubState);
				}
			}
			else if (mMisoVars.dwPdLen != 0)
			{
				mMisoVars.eState = AbccMisoStates::ReadProcessData;
			}
			else
			{
				mMisoVars.eState = AbccMisoStates::Crc32;
			}
		}
		break;
	case AbccMisoStates::MessageField:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			if (mMisoVars.fFragmentation && !mMisoVars.fFirstFrag)
			{
				eMsgSubState = AbccMisoStates::MessageField_Data;
				fAddFrame = true;
			}
			else
			{
				if (!RunAbccMisoMsgSubStateMachine(StateOperation::Run, &fAddFrame, &eMsgSubState))
				{
					/* Error */
					mMisoVars.eState = AbccMisoStates::Idle;
				}
			}
			if (mMisoVars.dwMsgLenCnt == 1)
			{
				if (mMisoVars.dwPdLen != 0)
				{
					mMisoVars.eState = AbccMisoStates::ReadProcessData;
				}
				else
				{
					mMisoVars.eState = AbccMisoStates::Crc32;
				}
			}
			//fAddFrame = true;
			mMisoVars.dwMsgLenCnt--;
		}
		break;
	case AbccMisoStates::ReadProcessData:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			if (mMisoVars.dwPdLen == 1)
			{
				mMisoVars.eState = AbccMisoStates::Crc32;
			}
			fAddFrame = true;
			mMisoVars.dwPdLen--;
		}
		break;
	case AbccMisoStates::Crc32:
		if (mMisoVars.dwByteCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eState))
		{
			fAddFrame = true;
			mMisoVars.eState = AbccMisoStates::Idle;
		}
		break;
	case AbccMisoStates::MessageField_Size:
	case AbccMisoStates::MessageField_Reserved1:
	case AbccMisoStates::MessageField_SourceId:
	case AbccMisoStates::MessageField_Object:
	case AbccMisoStates::MessageField_Instance:
	case AbccMisoStates::MessageField_Command:
	case AbccMisoStates::MessageField_Reserved2:
	case AbccMisoStates::MessageField_CommandExtension:
	case AbccMisoStates::MessageField_Data:
	default:
		mMisoVars.eState = AbccMisoStates::Idle;
		break;
	}

	if (WouldAdvancingTheClockToggleEnable())
	{
		if (mMisoVars.eState != AbccMisoStates::Idle)
		{
			/* We have a fragmented message */
			if (mEnable != nullptr)
			{
				AddFragFrame(SpiChannel::MISO, mMisoVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
			}
			else
			{
				AddFragFrame(SpiChannel::MISO, mMisoVars.lFramesFirstSample, mClock->GetSampleNumber());
			}
			mMisoVars.eState = AbccMisoStates::Idle;
			mMisoVars.lFrameData = 0;
			mMisoVars.dwByteCnt = 0;
			return true;
		}
	}

	if (fAddFrame)
	{
		if (eMisoState_Current == AbccMisoStates::MessageField)
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

	if (WouldAdvancingTheClockToggleEnable())
	{
		mMisoVars.eState = AbccMisoStates::Idle;
		mMisoVars.lFrameData = 0;
		mMisoVars.dwByteCnt = 0;
	}

	if (mMisoVars.eState == AbccMisoStates::Idle)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SpiAnalyzer::RunAbccMosiStateMachine(StateOperation operation, AcquisitionStatus acquisition_status, U64 mosi_data, S64 first_sample)
{
	AbccMosiStates::Enum eMsgSubState = AbccMosiStates::MessageField_Size;
	AbccMosiStates::Enum eMosiState_Current;
	bool fAddFrame = false;

	eMosiState_Current = mMosiVars.eState;

	/* If an error is signaled we jump into IDLE and wait to be reset.
	** A reset should be logically signaled when CS# is brought HIGH.
	** This would essentially indicate the begining of a new transaction. */
	if ((acquisition_status == AcquisitionStatus::Error) || !IsEnableActive())// || WouldAdvancingTheClockToggleEnable())
	{
		if (mMosiVars.dwByteCnt == 0)
		{
			mMosiVars.lFramesFirstSample = first_sample;
		}
		mMosiVars.eState = AbccMosiStates::Idle;
		if (mEnable != nullptr)
		{
			AddFragFrame(SpiChannel::MOSI, mMosiVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
		}
		else
		{
			/* 3-wire mode fragments exist only when idle gaps are detected too soon. */
			AddFragFrame(SpiChannel::MOSI, mMosiVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
		}
		mResults->CommitResults();
		return true;
	}

	if (mMosiVars.eState == AbccMosiStates::Idle)
	{
		mMosiVars.oChecksum.Init();
		mMosiVars.lFrameData = 0;
		mMosiVars.dwByteCnt = 0;
		if (operation == StateOperation::Reset)
		{
			mMosiVars.eState = AbccMosiStates::SpiControl;
			eMosiState_Current = mMosiVars.eState;
		}
	}

	if (mMosiVars.dwByteCnt == 0)
	{
		mMosiVars.lFramesFirstSample = first_sample;
	}

	mMosiVars.lFrameData |= (mosi_data << (8 * mMosiVars.dwByteCnt));
	mMosiVars.dwByteCnt++;

	if (mMosiVars.eState != AbccMosiStates::Crc32)
	{
		mMosiVars.oChecksum.Update((U8*)&mosi_data, 1);
	}

	switch (mMosiVars.eState)
	{
	case AbccMosiStates::Idle:
		/* We wait here until a reset is signaled */
		break;
	case AbccMosiStates::SpiControl:
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
				mMosiVars.eMsgSubState = AbccMosiStates::MessageField_DataNotValid;
				mMosiVars.wMdCnt = 0;
				mMosiVars.wMdSize = 0;
			}
			fAddFrame = true;
			mMosiVars.eState = AbccMosiStates::Reserved1;
		}
		break;
	case AbccMosiStates::Reserved1:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = AbccMosiStates::MessageLength;
		}
		break;
	case AbccMosiStates::MessageLength:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.dwMsgLen = (U32)mMosiVars.lFrameData * 2;
			mMosiVars.dwMsgLenCnt = mMosiVars.dwMsgLen;
			mMisoVars.dwMsgLen = mMosiVars.dwMsgLen;
			mMisoVars.dwMsgLenCnt = mMosiVars.dwMsgLen;
			mMosiVars.eState = AbccMosiStates::ProcessDataLength;
		}
		break;
	case AbccMosiStates::ProcessDataLength:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.dwPdLen = (U32)mMosiVars.lFrameData * 2;
			mMisoVars.dwPdLen = mMosiVars.dwPdLen;
			mMosiVars.eState = AbccMosiStates::ApplicationStatus;
		}
		break;
	case AbccMosiStates::ApplicationStatus:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = AbccMosiStates::InterruptMask;
		}
		break;
	case AbccMosiStates::InterruptMask:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			if (mMosiVars.dwMsgLenCnt != 0)
			{
				mMosiVars.eState = AbccMosiStates::MessageField;
				if (mMosiVars.fNewMsg)
				{
					RunAbccMosiMsgSubStateMachine(StateOperation::Reset, nullptr, &eMsgSubState);
				}
			}
			else if (mMosiVars.dwPdLen != 0)
			{
				mMosiVars.eState = AbccMosiStates::WriteProcessData;
			}
			else
			{
				mMosiVars.eState = AbccMosiStates::Crc32;
			}
		}
		break;
	case AbccMosiStates::MessageField:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			if (mMosiVars.fFragmentation && !mMosiVars.fFirstFrag)
			{
				eMsgSubState = AbccMosiStates::MessageField_Data;
				fAddFrame = true;
			}
			else
			{
				if (!RunAbccMosiMsgSubStateMachine(StateOperation::Run, &fAddFrame, &eMsgSubState))
				{
					/* Error */
					mMosiVars.eState = AbccMosiStates::Idle;
				}
			}
			if (mMosiVars.dwMsgLenCnt == 1)
			{
				if (mMosiVars.dwPdLen != 0)
				{
					mMosiVars.eState = AbccMosiStates::WriteProcessData;
				}
				else
				{
					mMosiVars.eState = AbccMosiStates::Crc32;
				}
			}
			mMosiVars.dwMsgLenCnt--;
		}
		break;
	case AbccMosiStates::WriteProcessData:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			if (mMosiVars.dwPdLen == 1)
			{
				mMosiVars.eState = AbccMosiStates::Crc32;
			}
			fAddFrame = true;
			mMosiVars.dwPdLen--;
		}
		break;
	case AbccMosiStates::Crc32:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = AbccMosiStates::Pad;
		}
		break;
	case AbccMosiStates::Pad:
		if (mMosiVars.dwByteCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eState))
		{
			fAddFrame = true;
			mMosiVars.eState = AbccMosiStates::Idle;
		}
		break;
	case AbccMosiStates::MessageField_Size:
	case AbccMosiStates::MessageField_Reserved1:
	case AbccMosiStates::MessageField_SourceId:
	case AbccMosiStates::MessageField_Object:
	case AbccMosiStates::MessageField_Instance:
	case AbccMosiStates::MessageField_Command:
	case AbccMosiStates::MessageField_Reserved2:
	case AbccMosiStates::MessageField_CommandExtension:
	case AbccMosiStates::MessageField_Data:
	default:
		mMosiVars.eState = AbccMosiStates::Idle;
		break;
	}

	if (WouldAdvancingTheClockToggleEnable())
	{
		if (mMosiVars.eState != AbccMosiStates::Idle)
		{
			/* We have a fragmented message */
			if (mEnable != nullptr)
			{
				AddFragFrame(SpiChannel::MOSI, mMosiVars.lFramesFirstSample, mEnable->GetSampleOfNextEdge());
			}
			else
			{
				AddFragFrame(SpiChannel::MOSI, mMosiVars.lFramesFirstSample, mClock->GetSampleOfNextEdge());
			}
			mMosiVars.eState = AbccMosiStates::Idle;
			mMosiVars.lFrameData = 0;
			mMosiVars.dwByteCnt = 0;
			return true;
		}
	}

	if (fAddFrame)
	{
		if (eMosiState_Current == AbccMosiStates::MessageField)
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

	if (WouldAdvancingTheClockToggleEnable())
	{
		mMosiVars.eState = AbccMosiStates::Idle;
		mMosiVars.lFrameData = 0;
		mMosiVars.dwByteCnt = 0;
	}

	if (mMosiVars.eState == AbccMosiStates::Idle)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SpiAnalyzer::RunAbccMisoMsgSubStateMachine(StateOperation operation, bool* add_frame_ptr, AbccMisoStates::Enum* e_substate_ptr)
{
	if (operation == StateOperation::Reset)
	{
		/* Perform checks here that we were in the last state and that the
		** number of bytes seen in this state matched the header's msg len specifier
		** In such cases a "framing error" should be signaled */
		mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Size;
		mMisoVars.bFrameSizeCnt = 0;
		return true;
	}

	if ((add_frame_ptr == nullptr) || (e_substate_ptr == nullptr))
	{
		return false;
	}

	*e_substate_ptr = mMisoVars.eMsgSubState;
	mMisoVars.bFrameSizeCnt++;

	switch (mMisoVars.eMsgSubState)
	{
	case AbccMisoStates::MessageField_Size:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Reserved1;
		}
		break;
	case AbccMisoStates::MessageField_Reserved1:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_SourceId;
		}
		break;
	case AbccMisoStates::MessageField_SourceId:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Object;
		}
		break;
	case AbccMisoStates::MessageField_Object:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Instance;
		}
		break;
	case AbccMisoStates::MessageField_Instance:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Command;
		}
		break;
	case AbccMisoStates::MessageField_Command:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Reserved2;
		}
		break;
	case AbccMisoStates::MessageField_Reserved2:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_CommandExtension;
		}
		break;
	case AbccMisoStates::MessageField_CommandExtension:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Data;
		}
		break;
	case AbccMisoStates::MessageField_Data:
	case AbccMisoStates::MessageField_DataNotValid:
		if (mMisoVars.bFrameSizeCnt >= GET_MISO_FRAME_SIZE(mMisoVars.eMsgSubState))
		{
			*add_frame_ptr = true;
		}
		break;
	case AbccMisoStates::Idle:
	case AbccMisoStates::Reserved1:
	case AbccMisoStates::Reserved2:
	case AbccMisoStates::LedStatus:
	case AbccMisoStates::AnybusStatus:
	case AbccMisoStates::SpiStatus:
	case AbccMisoStates::NetworkTime:
	case AbccMisoStates::MessageField:
	case AbccMisoStates::ReadProcessData:
	case AbccMisoStates::Crc32:
	default:
		mMisoVars.eMsgSubState = AbccMisoStates::MessageField_Data;
		return false;
	}

	if (*add_frame_ptr == true)
	{
		mMisoVars.bFrameSizeCnt = 0;
	}

	return true;
}

bool SpiAnalyzer::RunAbccMosiMsgSubStateMachine(StateOperation operation, bool* add_frame_ptr, AbccMosiStates::Enum* e_substate_ptr)
{
	if (operation == StateOperation::Reset)
	{
		/* Perform checks here that we were in the last state and that the
		** number of bytes seen in this state matched the header's msg len specifier
		** In such cases a "framing error" should be signaled */
		mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Size;
		mMosiVars.bFrameSizeCnt = 0;
		return true;
	}

	if ((add_frame_ptr == nullptr) || (e_substate_ptr == nullptr))
	{
		return false;
	}

	*e_substate_ptr = mMosiVars.eMsgSubState;
	mMosiVars.bFrameSizeCnt++;

	switch (mMosiVars.eMsgSubState)
	{
	case AbccMosiStates::MessageField_Size:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Reserved1;
		}
		break;
	case AbccMosiStates::MessageField_Reserved1:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_SourceId;
		}
		break;
	case AbccMosiStates::MessageField_SourceId:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Object;
		}
		break;
	case AbccMisoStates::MessageField_Object:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Instance;
		}
		break;
	case AbccMosiStates::MessageField_Instance:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Command;
		}
		break;
	case AbccMosiStates::MessageField_Command:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Reserved2;
		}
		break;
	case AbccMosiStates::MessageField_Reserved2:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_CommandExtension;
		}
		break;
	case AbccMosiStates::MessageField_CommandExtension:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
			mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Data;
		}
		break;
	case AbccMosiStates::MessageField_Data:
	case AbccMosiStates::MessageField_DataNotValid:
		if (mMosiVars.bFrameSizeCnt >= GET_MOSI_FRAME_SIZE(mMosiVars.eMsgSubState))
		{
			*add_frame_ptr = true;
		}
		break;
	case AbccMosiStates::Pad:
	case AbccMosiStates::Crc32:
	case AbccMosiStates::WriteProcessData:
	case AbccMosiStates::MessageField:
	case AbccMosiStates::InterruptMask:
	case AbccMosiStates::ApplicationStatus:
	case AbccMosiStates::ProcessDataLength:
	case AbccMosiStates::MessageLength:
	case AbccMosiStates::Reserved1:
	case AbccMosiStates::SpiControl:
	case AbccMosiStates::Idle:
	default:
		mMosiVars.eMsgSubState = AbccMosiStates::MessageField_Data;
		return false;
	}

	if (*add_frame_ptr == true)
	{
		mMosiVars.bFrameSizeCnt = 0;
	}

	return true;
}

void SpiAnalyzer::RestorePreviousStateVars()
{
	/* In the event of an error packet that would otherwise result in a
	** 'retransmit' event, this routine must be called to put the revelant
	** state variable back to the last known 'good state' */
	mMisoVars.wMdCnt = mPreviousMisoVars.wMdCnt;
	mMisoVars.fFirstFrag = mPreviousMisoVars.fFirstFrag;
	mMisoVars.fLastFrag = mPreviousMisoVars.fLastFrag;
	mMisoVars.fFragmentation = mPreviousMisoVars.fFragmentation;

	mMosiVars.wMdCnt = mPreviousMosiVars.wMdCnt;
	mMosiVars.fFirstFrag = mPreviousMosiVars.fFirstFrag;
	mMosiVars.fLastFrag = mPreviousMosiVars.fLastFrag;
	mMosiVars.fFragmentation = mPreviousMosiVars.fFragmentation;
}

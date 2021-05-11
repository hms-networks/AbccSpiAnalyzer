/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerDataGenerator.cpp
**    Summary: DLL-DataGenerator source
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include <cstring>
#include <random>
#include <math.h>

#include "AbccSpiSimulationDataGenerator.h"
#include "AbccSpiAnalyzerSettings.h"
#include "AbccSpiAnalyzer.h"
#include "AbccCrc.h"
#include "abcc_td.h"
#include "abcc_abp/abp.h"
#include "abcc_abp/abp_fsi.h"

#include "AbccSpiMetadata.h"

#define _USE_MATH_DEFINES

#define FILE_INSTANCE_TO_USE	0x01
#define FILE_READ_CHUNK_SIZE	511

#define MSG_HEADER_SIZE			12

/*------------------------------------------------------------------------
** Enums, Types, and Classes
**------------------------------------------------------------------------
*/

enum class SimulationState : U16
{
	CreateInstanceCommand,
	CreateInstanceResponse,
	FileOpenCommand,
	FileOpenResponse,
	GetFileSizeCommand,
	GetFileSizeResponse,
	FileReadCommand,
	FileReadResponse,
	FileCloseCommand,
	FileCloseResponse,
	DeleteInstanceCommand,
	DeleteInstanceResponse,
	SizeOfEnum
};

/*------------------------------------------------------------------------
** Globals
**------------------------------------------------------------------------
*/

static const CHAR filename[] = "metadata.json";
static const CHAR fileData[] = "{\n"
								"\t" METADATA_FILEVERSION_KEY "=\"" ABCC_SPI_METADATA_FILEVERSION "\"\n"
								"\t" METADATA_COMPANYNAME_KEY "=\"" ABCC_SPI_METADATA_COMPANYNAME "\"\n"
								"\t" METADATA_FILEDESCRIPTION_KEY "=\"" ABCC_SPI_METADATA_FILEDESCRIPTION "\"\n"
								"\t" METADATA_INTERNALNAME_KEY "=\"" ABCC_SPI_METADATA_INTERNALNAME "\"\n"
								"\t" METADATA_LEGALCOPYRIGHT_KEY "=\"" ABCC_SPI_METADATA_LEGALCOPYRIGHT "\"\n"
								"\t" METADATA_ORIGINALFILENAME_KEY "=\"" ABCC_SPI_METADATA_ORIGINALFILENAME "\"\n"
								"\t" METADATA_PRODUCTNAME_KEY "=\"" ABCC_SPI_METADATA_PRODUCTNAME "\"\n"
								"\t" METADATA_PRODUCTVERSION_KEY "=\"" ABCC_SPI_METADATA_PRODUCTVERSION "\"\n"
								"}";

/*------------------------------------------------------------------------
** Methods/Routines
**------------------------------------------------------------------------
*/

inline void SpiSimulationDataGenerator::SetMosiObjectSpecificError(U8 error_code)
{
	mMosiMsgData.sHeader.iDataSize = 2;
	mMosiMsgData.abData[0] = 0xFF;
	mMosiMsgData.abData[1] = error_code;
}

SpiSimulationDataGenerator::SpiSimulationDataGenerator()
{
	mMsgCmdRespState = (U16)SimulationState::SizeOfEnum;
	mMessageDataOffset = 0;
	mMessageCount = 0;
	mLogFileSimulation = false;
	mClockIdleMode = ClockIdleMode::Auto;
	mNextClockIdleMode = ClockIdleMode::High;
	m3WireMode = false;

	mToggleBit = 0;
	mSourceId = 0;
	mNetTime = 0x00000001;

	memset(&mMisoMsgData, 0, sizeof(mMisoMsgData));
	memset(&mMosiMsgData, 0, sizeof(mMosiMsgData));

	mAbortTransfer = false;
}

SpiSimulationDataGenerator::~SpiSimulationDataGenerator()
{
}

void SpiSimulationDataGenerator::Initialize(U32 simulation_sample_rate, SpiAnalyzerSettings* settings)
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	InitializeSpiChannels();

	if (!mSettings->mSimulateLogFilePath.empty())
	{
		// Try to load the log file for simulation
		mLogFileParser = new AbccLogFileParser(
			mSettings->mSimulateLogFilePath,
			static_cast<ABP_AnbStateType>(mSettings->mSimulateLogFileDefaultState));

		if (mLogFileParser->IsOpen())
		{
			mLogFileSimulation = true;
		}
	}

	InitializeSpiClockIdleMode();
	InitializeSpiTimingCharacteristics();
	mClockGenerator.Init(mTargetClockFrequencyHz, mSimulationSampleRateHz);

	// Insert inter-packet gap idle time
	mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * mInterPacketGapTime));

	mIncrementingValue = 0;

	mDynamicMsgFragmentationLength = (mSettings->mSimulateMsgDataLength < 0);
	mMaxMsgFragmentationLength = static_cast<U16>(std::abs(mSettings->mSimulateMsgDataLength)) << 1;
	UpdatePacketDynamicFormat(mMaxMsgFragmentationLength, ABCC_CFG_MAX_PROCESS_DATA_SIZE);
}

U32 SpiSimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	U64 adjustedLargestSampleRequested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

	while (mClock->GetCurrentSampleNumber() < adjustedLargestSampleRequested)
	{
		if (!CreateSpiTransaction())
		{
			break;
		}

		// Insert inter-packet gap idle time
		mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * mInterPacketGapTime));
	}

	*simulation_channels = mSpiSimulationChannels.GetArray();
	return mSpiSimulationChannels.GetCount();
}

void SpiSimulationDataGenerator::InitializeSpiClockIdleMode()
{
	if (mLogFileSimulation)
	{
		if (m3WireMode || (mSettings->mSimulateClockIdleHigh == 1))
		{
			mClockIdleMode = ClockIdleMode::High;
		}
		else
		{
			mClockIdleMode = ClockIdleMode::Low;
		}
	}
	else
	{
		// "clock idle low" and "auto" mode are only available in 4 wire mode.
		if (m3WireMode)
		{
			mClockIdleMode = ClockIdleMode::High;
		}
		else
		{
			switch (mSettings->mSimulateClockIdleHigh)
			{
			case 0:
				mClockIdleMode = ClockIdleMode::Low;
				break;
			case 1:
				mClockIdleMode = ClockIdleMode::High;
				break;
			default:
				mClockIdleMode = ClockIdleMode::Auto;
				break;
			}
		}
	}
}

void SpiSimulationDataGenerator::InitializeSpiTimingCharacteristics()
{
	const double maxAbccSpiClockFrequencyHz = 20000000.0;
	const double min3WireAbccSpiClockFrequencyHz = 100000.0;

	mInterPacketGapTime = mSettings->mSimulatePacketGapNs * 1e-9;
	mInterByteGapTime = mSettings->mSimulateByteGapNs * 1e-9;

	if (mInterPacketGapTime <= 0)
	{
		mInterPacketGapTime = (1.5 * MIN_IDLE_GAP_TIME);
	}

	mChipSelectDelay = mSettings->mSimulateChipSelectNs * 1e-9;

	if (mChipSelectDelay <= 0)
	{
		mChipSelectDelay = 1e-6;
	}

	// Use a 1/10th rule for clock frequency versus sample rate to provide good sample characteristics
	mTargetClockFrequencyHz = mSimulationSampleRateHz / 10;

	if (mLogFileSimulation && (mSettings->mSimulateClockFrequency > 0))
	{
		mTargetClockFrequencyHz = static_cast<double>(mSettings->mSimulateClockFrequency);
	}

	if (mTargetClockFrequencyHz > maxAbccSpiClockFrequencyHz)
	{
		mTargetClockFrequencyHz = maxAbccSpiClockFrequencyHz;
	}

	if (m3WireMode)
	{
		if (mTargetClockFrequencyHz < min3WireAbccSpiClockFrequencyHz)
		{
			mTargetClockFrequencyHz = min3WireAbccSpiClockFrequencyHz;
		}

		// Verify that the packet gap time does not exceed 3-wire limitations
		if (mInterPacketGapTime < MIN_IDLE_GAP_TIME)
		{
			mInterPacketGapTime = MIN_IDLE_GAP_TIME;
		}
	}
}

void SpiSimulationDataGenerator::InitializeSpiChannels()
{
	if (mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mMiso = mSpiSimulationChannels.Add(mSettings->mMisoChannel, mSimulationSampleRateHz, BitState::BIT_LOW);
	}
	else
	{
		mMiso = nullptr;
	}

	if (mSettings->mMosiChannel != UNDEFINED_CHANNEL)
	{
		mMosi = mSpiSimulationChannels.Add(mSettings->mMosiChannel, mSimulationSampleRateHz, BitState::BIT_LOW);
	}
	else
	{
		mMosi = nullptr;
	}

	BitState initialClockState = (mClockIdleMode == ClockIdleMode::Low) ?
		BitState::BIT_LOW :
		BitState::BIT_HIGH;

	mClock = mSpiSimulationChannels.Add(mSettings->mClockChannel, mSimulationSampleRateHz, initialClockState);

	if (mSettings->mEnableChannel != UNDEFINED_CHANNEL)
	{
		BitState enableInitState = BitState::BIT_HIGH;

		if (mSettings->m3WireOn4Channels)
		{
			enableInitState = BitState::BIT_LOW;
		}

		mEnable = mSpiSimulationChannels.Add(mSettings->mEnableChannel, mSimulationSampleRateHz, enableInitState);
	}
	else
	{
		mEnable = nullptr;
	}

	m3WireMode = (((mEnable == nullptr) && (mSettings->m4WireOn3Channels == false)) || (mSettings->m3WireOn4Channels == true));
}

void SpiSimulationDataGenerator::UpdatePacketDynamicFormat(U16 message_data_field_length, U16 process_data_field_length)
{
	const U16 mosiHeaderBytes = 8;
	const U16 mosiTrailingBytes = 6;
	const U16 misoHeaderBytes = 10;

	if (message_data_field_length > mMaxMsgFragmentationLength)
	{
		mMsgFragmentationLength = mMaxMsgFragmentationLength;
	}
	else
	{
		mMsgFragmentationLength = message_data_field_length;
	}

	mMosiProcessDataPtr = mMosiPacket.msgData + mMsgFragmentationLength;
	mMosiCrc32Ptr = mMosiProcessDataPtr + process_data_field_length;
	mMosiCrcPacketLength = mosiHeaderBytes + mMsgFragmentationLength + process_data_field_length;

	mMisoProcessDataPtr = mMisoPacket.msgData + mMsgFragmentationLength;
	mMisoCrc32Ptr = mMisoProcessDataPtr + process_data_field_length;
	mMisoCrcPacketLength = misoHeaderBytes + mMsgFragmentationLength + process_data_field_length;

	mNumBytesInSpiPacket = mMosiCrcPacketLength + mosiTrailingBytes;
}

void SpiSimulationDataGenerator::UpdateProcessData()
{
	const double freq = 100.0;
	double t = 0.0;
	double x;

	if (mSimulationSampleRateHz != 0)
	{
		t = (double)mClock->GetCurrentSampleNumber() / (double)mSimulationSampleRateHz;
	}

	// Simulate a 100Hz sinusoids on MOSI and MISO process data.
	// This sinusoid will swing between -65535 and 65535
	x = 2.0 * 3.14159265359 * freq * t;
	S32 mosiProcessData = (S32)(65535 * sin(x));
	S32 misoProcessData = (S32)(65535 * cos(x));

	memcpy(mMosiProcessDataPtr, &mosiProcessData, sizeof(mosiProcessData));
	memcpy(mMisoProcessDataPtr, &misoProcessData, sizeof(misoProcessData));

	mMosiPacket.spiCtrl |= ABP_SPI_CTRL_WRPD_VALID;
	mMisoPacket.spiStat |= ABP_SPI_STATUS_NEW_PD;
}

bool SpiSimulationDataGenerator::UpdateMessageData(U8* mosi_msg_data_source, U8* miso_msg_data_source)
{
	const U32 msgHeaderSize = static_cast<U32>(sizeof(ABP_MsgHeaderType));
	bool lastFragment = (mMessageDataOffset + mMsgFragmentationLength) >= (mTotalMsgDataBytesToSend + msgHeaderSize);

	mMosiPacket.msgLen = mMsgFragmentationLength >> 1;

	// If a valid message is available, copy message data to SPI buffer.
	if (mMisoPacket.spiStat & ABP_SPI_STATUS_M)
	{
		memcpy(mMisoPacket.msgData, miso_msg_data_source, mMsgFragmentationLength);
	}

	if (mMosiPacket.spiCtrl & ABP_SPI_CTRL_M)
	{
		memcpy(mMosiPacket.msgData, mosi_msg_data_source, mMsgFragmentationLength);
	}

	// Update the LAST_FRAG flag to indicate if more fragments follow or not.
	if (lastFragment)
	{
		if (mMisoPacket.spiStat & ABP_SPI_STATUS_M)
		{
			mMisoPacket.spiStat |= ABP_SPI_STATUS_LAST_FRAG;
		}

		if (mMosiPacket.spiCtrl & ABP_SPI_CTRL_M)
		{
			mMosiPacket.spiCtrl |= ABP_SPI_CTRL_LAST_FRAG;
		}
	}
	else
	{
		if (mMisoPacket.spiStat & ABP_SPI_STATUS_M)
		{
			mMisoPacket.spiStat &= ~ABP_SPI_STATUS_LAST_FRAG;
		}

		if (mMosiPacket.spiCtrl & ABP_SPI_CTRL_M)
		{
			mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_LAST_FRAG;
		}
	}

	return lastFragment;
}

void SpiSimulationDataGenerator::UpdateCrc32(bool generate_mosi_crc_error, bool generate_miso_crc_error)
{
	AbccCrc mosiChecksum = AbccCrc();
	AbccCrc misoChecksum = AbccCrc();
	U32 crc32;

	mosiChecksum.Init();
	misoChecksum.Init();

	mosiChecksum.Update(((U8*)&mMosiPacket), mMosiCrcPacketLength);
	misoChecksum.Update(((U8*)&mMisoPacket), mMisoCrcPacketLength);

	crc32 = mosiChecksum.Crc32();

	if (generate_mosi_crc_error)
	{
		crc32++;
	}

	memcpy(mMosiCrc32Ptr, (U8*)&crc32, sizeof(crc32));

	crc32 = misoChecksum.Crc32();

	if (generate_miso_crc_error)
	{
		crc32++;
	}

	memcpy(mMisoCrc32Ptr, (U8*)&crc32, sizeof(crc32));
}

bool SpiSimulationDataGenerator::CreateSpiTransaction()
{
	ClockIdleMode currentClockIdleMode;

	std::random_device rd;
	std::mt19937 prng(rd());
	std::uniform_int_distribution<> fragmentSize(1, mNumBytesInSpiPacket - 1);

	bool mosiCrcError;
	bool misoCrcError;
	bool fragmentError;
	bool errorResponse;
	bool clockingError;
	bool outOfBandClocking;
	bool errorPresent = false;
	bool oneByteFragmentError = false;

	bool continueSimulation = true;

	U8* pMosiData = (U8*)&mMosiMsgData;
	U8* pMisoData = (U8*)&mMisoMsgData;

	// Reinitialize SPI packets each cycle
	memset(&mMisoPacket, 0, sizeof(mMisoPacket));
	memset(&mMosiPacket, 0, sizeof(mMosiPacket));

	mMosiPacket.pdLen = ABCC_CFG_MAX_PROCESS_DATA_SIZE >> 1;
	mMosiPacket.spiCtrl |= mToggleBit | ABP_SPI_CTRL_CMDCNT;
	mMisoPacket.spiStat |= ABP_SPI_STATUS_CMDCNT;

	if (mLogFileSimulation)
	{
		mosiCrcError = false;
		misoCrcError = false;
		fragmentError = false;
		clockingError = false;
		outOfBandClocking = false;
		mNextClockIdleMode = mClockIdleMode;

		// When getting a message, the MOSI buffer is always used,
		// Depending on the actual message type determined, the pointers
		// may be swapped to produce the message data on the proper channel.
		// In this simulation, only one channel will have a valid message
		// at a time.

		if (mMessageDataOffset == 0)
		{
			mLogFileMessageType = mLogFileParser->GetNextMessage(mMosiMsgData);

			// Use the 4 bytes of process data to indicate the message count.
			if ((mLogFileMessageType == MessageReturnType::Tx) ||
				(mLogFileMessageType == MessageReturnType::TxError))
			{
				memcpy(mMosiProcessDataPtr, &mMessageCount, sizeof(mMessageCount));
				mMessageCount++;
			}
			else if ((mLogFileMessageType == MessageReturnType::Rx) ||
					 (mLogFileMessageType == MessageReturnType::RxError))
			{
				memcpy(mMisoProcessDataPtr, &mMessageCount, sizeof(mMessageCount));
				mMessageCount++;
			}
		}

		switch (mLogFileMessageType)
		{
		case MessageReturnType::StateChange:
			mTotalMsgDataBytesToSend = 0;
			break;

		case MessageReturnType::Tx:
			mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
			mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
			break;

		case MessageReturnType::Rx:
			// Swap buffers
			pMosiData = (U8*)&mMisoMsgData;
			pMisoData = (U8*)&mMosiMsgData;
			mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
			mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
			break;

		case MessageReturnType::TxError:
			// Parsing error occurred, produce a CRC error to help indicate this to the user.
			mTotalMsgDataBytesToSend = 0;
			mosiCrcError = true;
			break;

		case MessageReturnType::RxError:
			// Parsing error occurred, produce a CRC error to help indicate this to the user.
			mTotalMsgDataBytesToSend = 0;
			misoCrcError = true;
			break;

		case MessageReturnType::IoError:
		case MessageReturnType::EndOfFile:
			continueSimulation = false;
			break;

		default:
			break;
		}

		mMisoPacket.anbStat = static_cast<U8>(mLogFileParser->GetAnbStatus());
	}
	else
	{
		// Create a set of bernoulli random sequences to generate
		// random events in the simulation
		std::bernoulli_distribution generateClockIdleStateToggle(0.10);
		std::bernoulli_distribution generateOutOfBandClocking(0.005);
		std::bernoulli_distribution generateFragmentError(0.002);
		std::bernoulli_distribution generateMisoCrcError(0.002);
		std::bernoulli_distribution generateMosiCrcError(0.001);
		std::bernoulli_distribution generateMosiErrorRespMsg(0.01);
		std::bernoulli_distribution generateClockingError(0.001);
		std::bernoulli_distribution generate1ByteFragError(0.001);

		// In this simulation, a MOSI CRC error implies a
		// MISO CRC error as well which simulates the error
		// detection/reporting mechanism of the ABCC.
		mosiCrcError = generateMosiCrcError(prng);
		misoCrcError = generateMisoCrcError(prng) || mosiCrcError;

		// Determine if clock idle mode should change
		if ((mClockIdleMode == ClockIdleMode::Auto) && generateClockIdleStateToggle(prng))
		{
			if (mNextClockIdleMode == ClockIdleMode::Low)
			{
				mNextClockIdleMode = ClockIdleMode::High;
			}
			else
			{
				mNextClockIdleMode = ClockIdleMode::Low;
			}
		}

		fragmentError = generateFragmentError(prng);
		errorResponse = generateMosiErrorRespMsg(prng);
		clockingError = generateClockingError(prng);
		outOfBandClocking = generateOutOfBandClocking(prng);

		if (fragmentError || misoCrcError || mosiCrcError)
		{
			errorPresent = true;
		}

		if (m3WireMode)
		{
			oneByteFragmentError = generate1ByteFragError(prng);
			errorPresent = errorPresent || oneByteFragmentError;
		}

		// Obtain time information from the analyzer's current sample and sample frequency
		mNetTime = (U32)(mClock->GetCurrentSampleNumber() / (double)mSimulationSampleRateHz * (double)1e9) + 1;

		// Update the network time
		mMisoPacket.netTime_lo = mNetTime & 0xFFFF;
		mMisoPacket.netTime_hi = (mNetTime >> 16) & 0xFFFF;

		UpdateProcessData();
		mMisoPacket.anbStat = ABP_ANB_STATE_PROCESS_ACTIVE;

		// Generate message data and perform message fragmention the packet as needed
		if (errorResponse)
		{
			RunFileTransferStateMachine(MessageResponseType::Error);
		}
		else
		{
			RunFileTransferStateMachine(MessageResponseType::Normal);
		}
	}

	if (continueSimulation)
	{
		// ACTUAL FRAGMENTATION LENGTH IS THE "MAX" of MOSI and MISO requested fragmentation sizes
		// Adapt fragmentation length for MOSI messages such that:
		// -- MAX FRAG LENGTH IS NOT EXCEEDED
		// -- LAST FRAGMENT HAS NO PADDED MSG DATA BYTES
		// -- DEFAULT FRAGMENTATION IS USED IF IN IDLE or PROCESS ACTIVE
		// Adapt fragmentation length for MISO messages such that:
		// -- FIRST FRAGMENT USES DEFAULT SIZE (in real world application, fragmentation length cannot be determined until the MISO message header has been received)
		// -- NEXT FRAGMENTS DO NOT EXCEEDED MAX FRAG LENGTH
		// -- LAST FRAGMENT HAS NO PADDED MSG DATA BYTES
		// -- DEFAULT FRAGMENTATION IS USED IF IN IDLE or PROCESS ACTIVE
		//if (mDynamicMsgFragmentationLength)
		//{
		//	UpdatePacketDynamicFormat(mTotalMsgDataBytesToSend, ABCC_CFG_MAX_PROCESS_DATA_SIZE);
		//}

		bool lastFragment = UpdateMessageData(&pMosiData[mMessageDataOffset], &pMisoData[mMessageDataOffset]);
		UpdateCrc32(mosiCrcError, misoCrcError);

		currentClockIdleMode = (mClock->GetCurrentBitState() == BitState::BIT_HIGH) ?
			ClockIdleMode::High :
			ClockIdleMode::Low;

		if (!m3WireMode)
		{
			// Assert SPI Enable and move forward in time
			mEnable->Transition();
			mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(mChipSelectDelay));

			if (fragmentError)
			{
				// Create a fragmented SPI packet which will be short by 1 or more bytes
				SendPacketData(currentClockIdleMode, fragmentSize(prng));
			}
			else
			{
				// Produce the SPI packet
				SendPacketData(currentClockIdleMode, mNumBytesInSpiPacket);

				if (clockingError)
				{
					// Send an additional SPI packet before enable goes high (causes clocking errors)
					SendPacketData(currentClockIdleMode, mNumBytesInSpiPacket);
				}
			}

			// Deassert SPI Enable
			mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(mChipSelectDelay));
			mEnable->Transition();

			if (outOfBandClocking)
			{
				// Send an out-of-band SPI packet, this communication is ignored by the analyzer
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
				OutputByte_CPOL1_CPHA1(mIncrementingValue, mIncrementingValue + 1);
				mIncrementingValue++;
			}

			mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));

			// Select between "Clock Idle Low" and "Clock Idle High" SPI configurations
			if (mNextClockIdleMode == ClockIdleMode::Low)
			{
				mClock->TransitionIfNeeded(BitState::BIT_LOW);
			}
			else
			{
				mClock->TransitionIfNeeded(BitState::BIT_HIGH);
			}
		}
		else
		{
			if (fragmentError)
			{
				// Create a fragmented SPI packet which will be short by 1 or more bytes
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
				SendPacketData(currentClockIdleMode, fragmentSize(prng));
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			}
			else if (oneByteFragmentError)
			{
				// Create a fragmented SPI packet (1 byte)
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
				OutputByte_CPOL1_CPHA1(mIncrementingValue, mIncrementingValue + 1);
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
				mIncrementingValue++;
			}
			else
			{
				// Produce the SPI packet
				SendPacketData(currentClockIdleMode, mNumBytesInSpiPacket);

				if (clockingError)
				{
					// Send an additional SPI byte before enable goes high (causes clocking errors)
					mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
					OutputByte_CPOL1_CPHA1(mIncrementingValue, mIncrementingValue + 1);
					mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
					mIncrementingValue++;
				}
			}
		}

		// Update toggle bit only when no error in communication was
		// generated. The toggle bit should be left as-is in case of
		// errors to indicate the need for a retransmission.
		if (!errorPresent)
		{
			// Update toggle bit
			mToggleBit ^= ABP_SPI_CTRL_T;
			mMessageDataOffset += mMsgFragmentationLength;

			// Check if full message has been sent, if so update the
			// state of the file transfer state machine, and reset
			// the fragmentation packet offset
			if (lastFragment || mAbortTransfer)
			{
				mAbortTransfer = false;
				mMessageDataOffset = 0;

				if (!mLogFileSimulation)
				{
					UpdateFileTransferStateMachine();
				}
			}
		}
	}

	return continueSimulation;
}

void SpiSimulationDataGenerator::SendPacketData(ClockIdleMode clock_idle_mode, U32 length)
{
	if (length > sizeof(AbccMosiPacket_t))
	{
		length = sizeof(AbccMosiPacket_t);
	}

	for (U16 i = 0; i < length; i++)
	{
		U64 misoData;
		U64 mosiData;
		bool lastTransfer;

		if (mSettings->mSimulateWordMode)
		{
			U16 wordIndex = i >> 1;
			misoData = bswap_16(((U16*)&mMisoPacket)[wordIndex]);
			mosiData = bswap_16(((U16*)&mMosiPacket)[wordIndex]);
			i++;
			lastTransfer = i < (length - 2);
		}
		else
		{
			misoData = ((U8*)&mMisoPacket)[i];
			mosiData = ((U8*)&mMosiPacket)[i];
			lastTransfer = i < (length - 1);
		}

		if (clock_idle_mode == ClockIdleMode::High)
		{
			OutputByte_CPOL1_CPHA1(mosiData, misoData, mSettings->mSimulateWordMode);
		}
		else
		{
			OutputByte_CPOL0_CPHA0(mosiData, misoData, mSettings->mSimulateWordMode);
		}

		if (lastTransfer)
		{
			U32 minSamplesToAdvance = mClockGenerator.AdvanceByHalfPeriod(0.5);
			U32 samplesToAdvance = mClockGenerator.AdvanceByTimeS(mInterByteGapTime);

			if (samplesToAdvance > minSamplesToAdvance)
			{
				// During the last bit transfer, the signals were already advanced by "minSamplesToAdvance"
				// deduct this from the requested number of samples to advance.
				samplesToAdvance -= minSamplesToAdvance;
				mSpiSimulationChannels.AdvanceAll(samplesToAdvance);
			}
		}
	}

	mMosi->TransitionIfNeeded(BitState::BIT_LOW);
	mMiso->TransitionIfNeeded(BitState::BIT_LOW);
}

void SpiSimulationDataGenerator::OutputByte_CPOL0_CPHA0(U64 mosi_data, U64 miso_data, bool word_mode)
{
	U32 bitsPerTransfer = word_mode ? 16U : 8U;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);

	// First ensure clock is low
	if (mClock->GetCurrentBitState() == BitState::BIT_HIGH)
	{
		// Wrong beginning polarity, don't bother sending anything
		return;
	}

	for (U32 i = 0U; i < bitsPerTransfer; i++)
	{
		mMosi->TransitionIfNeeded(mosi_bits.GetNextBit());
		mMiso->TransitionIfNeeded(miso_bits.GetNextBit());

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();
	}
}

void SpiSimulationDataGenerator::OutputByte_CPOL1_CPHA1(U64 mosi_data, U64 miso_data, bool word_mode)
{
	U32 bitsPerTransfer = word_mode ? 16U : 8U;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);

	// First ensure clock is high
	if (mClock->GetCurrentBitState() == BitState::BIT_LOW)
	{
		// Wrong beginning polarity, don't bother sending anything
		return;
	}

	for (U32 i = 0U; i < bitsPerTransfer; i++)
	{
		mClock->Transition();
		mMosi->TransitionIfNeeded(mosi_bits.GetNextBit());
		mMiso->TransitionIfNeeded(miso_bits.GetNextBit());

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
	}
}

void SpiSimulationDataGenerator::CreateFileInstance(ABP_MsgType* msg_ptr, MessageType message_type)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = 0x0000;
	msg_ptr->sHeader.bCmd = ABP_CMD_CREATE;
	msg_ptr->sHeader.bCmdExt0 = 0x00;
	msg_ptr->sHeader.bCmdExt1 = 0x00;

	switch (message_type)
	{
	case MessageType::Command:
		msg_ptr->sHeader.iDataSize = 0;
		msg_ptr->sHeader.bCmd |= ABP_MSG_HEADER_C_BIT;
		break;
	case MessageType::Response:
		msg_ptr->sHeader.iDataSize = 0x0002;
		msg_ptr->abData[0] = (U8)FILE_INSTANCE_TO_USE;
		msg_ptr->abData[1] = 0x00;
		break;
	}
}

void SpiSimulationDataGenerator::FileOpen(ABP_MsgType* msg_ptr, MessageType message_type, const CHAR* file_name, UINT8 file_name_length)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = (U16)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmd = ABP_FSI_CMD_FILE_OPEN;
	msg_ptr->sHeader.bCmdExt0 = 0x00; // Read only mode
	msg_ptr->sHeader.bCmdExt1 = 0x00;

	switch (message_type)
	{
	case MessageType::Command:
		msg_ptr->sHeader.bCmd |= ABP_MSG_HEADER_C_BIT;
		msg_ptr->sHeader.iDataSize = file_name_length;
		memcpy(msg_ptr->abData, file_name, file_name_length);
		break;
	case MessageType::Response:
		msg_ptr->sHeader.iDataSize = 0x0000;
		break;
	}
}

void SpiSimulationDataGenerator::GetFileSize(ABP_MsgType* msg_ptr, MessageType message_type)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = (U16)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmd = ABP_CMD_GET_ATTR;
	msg_ptr->sHeader.bCmdExt0 = ABP_FSI_IA_FILE_SIZE;
	msg_ptr->sHeader.bCmdExt1 = 0x00;

	switch (message_type)
	{
	case MessageType::Command:
		msg_ptr->sHeader.bCmd |= ABP_MSG_HEADER_C_BIT;
		msg_ptr->sHeader.iDataSize = 0x0000;
		break;
	case MessageType::Response:
		U32 dwFileSize = sizeof(fileData) - 1;
		msg_ptr->sHeader.iDataSize = ABP_FSI_IA_FILE_SIZE_DS;
		memcpy(msg_ptr->abData, &dwFileSize, ABP_FSI_IA_FILE_SIZE_DS);
		break;
	}
}

void SpiSimulationDataGenerator::FileRead(ABP_MsgType* msg_ptr, MessageType message_type, const CHAR* file_data, UINT32 file_data_length)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = (U16)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmd = ABP_FSI_CMD_FILE_READ;

	switch (message_type)
	{
	case MessageType::Command:
		msg_ptr->sHeader.bCmd |= ABP_MSG_HEADER_C_BIT;
		msg_ptr->sHeader.iDataSize = 0x0000;
		msg_ptr->sHeader.bCmdExt0 = (FILE_READ_CHUNK_SIZE >> 0) & 0xFF;
		msg_ptr->sHeader.bCmdExt1 = (FILE_READ_CHUNK_SIZE >> 8) & 0xFF;
		break;
	case MessageType::Response:
		msg_ptr->sHeader.bCmdExt0 = 0x00; // Reserved
		msg_ptr->sHeader.bCmdExt1 = 0x00; // Reserved

		if (file_data_length > FILE_READ_CHUNK_SIZE)
		{
			// Truncate payload if it exceeds chunk size
			file_data_length = FILE_READ_CHUNK_SIZE;
		}

		msg_ptr->sHeader.iDataSize = (U16)file_data_length;
		memcpy(msg_ptr->abData, file_data, file_data_length);
		break;
	}
}

void SpiSimulationDataGenerator::FileClose(ABP_MsgType* msg_ptr, MessageType message_type, UINT32 file_size)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = (U16)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmd = ABP_FSI_CMD_FILE_CLOSE;
	msg_ptr->sHeader.bCmdExt0 = 0;
	msg_ptr->sHeader.bCmdExt1 = 0;

	switch (message_type)
	{
	case MessageType::Command:
		msg_ptr->sHeader.bCmd |= ABP_MSG_HEADER_C_BIT;
		msg_ptr->sHeader.iDataSize = 0x0000;
		break;
	case MessageType::Response:
		msg_ptr->sHeader.iDataSize = sizeof(file_size);
		memcpy(msg_ptr->abData, &file_size, sizeof(file_size));
		break;
	}
}

void SpiSimulationDataGenerator::DeleteFileInstance(ABP_MsgType* msg_ptr, MessageType message_type)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = 0x0000;
	msg_ptr->sHeader.bCmd = ABP_CMD_DELETE;
	msg_ptr->sHeader.bCmdExt0 = (U8)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmdExt1 = 0x00;
	msg_ptr->sHeader.iDataSize = 0x0000;

	switch (message_type)
	{
	case MessageType::Command:
		msg_ptr->sHeader.bCmd |= ABP_MSG_HEADER_C_BIT;
		break;
	case MessageType::Response:
		break;
	}
}

void SpiSimulationDataGenerator::RunFileTransferStateMachine(MessageResponseType msg_response_type)
{
	memset(&mMisoMsgData, 0, MSG_HEADER_SIZE);
	memset(&mMosiMsgData, 0, MSG_HEADER_SIZE);

	mMosiMsgData.sHeader.bSourceId = mSourceId;
	mMisoMsgData.sHeader.bSourceId = mSourceId;

	switch (static_cast<SimulationState>(mMsgCmdRespState))
	{
	case SimulationState::CreateInstanceCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		CreateFileInstance(&mMisoMsgData, MessageType::Command);
		mTotalMsgDataBytesToSend = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::CreateInstanceResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		CreateFileInstance(&mMosiMsgData, MessageType::Response);
		mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileOpenCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		FileOpen(&mMisoMsgData, MessageType::Command, filename, sizeof(filename) - 1);
		mTotalMsgDataBytesToSend = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileOpenResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		FileOpen(&mMosiMsgData, MessageType::Response, nullptr, 0);
		mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::GetFileSizeCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		GetFileSize(&mMisoMsgData, MessageType::Command);
		mTotalMsgDataBytesToSend = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::GetFileSizeResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		GetFileSize(&mMosiMsgData, MessageType::Response);
		mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileReadCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		FileRead(&mMisoMsgData, MessageType::Command, nullptr, 0);
		mTotalMsgDataBytesToSend = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileReadResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		FileRead(&mMosiMsgData, MessageType::Response, fileData, sizeof(fileData) - 1);
		mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileCloseCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		FileClose(&mMisoMsgData, MessageType::Command, 0);
		mTotalMsgDataBytesToSend = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileCloseResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		FileClose(&mMosiMsgData, MessageType::Response, sizeof(fileData) - 1);
		mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::DeleteInstanceCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		DeleteFileInstance(&mMisoMsgData, MessageType::Command);
		mTotalMsgDataBytesToSend = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::DeleteInstanceResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		DeleteFileInstance(&mMosiMsgData, MessageType::Response);
		mTotalMsgDataBytesToSend = mMosiMsgData.sHeader.iDataSize;
		break;
	default:
		/* no message */
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		mTotalMsgDataBytesToSend = 0;
		break;
	}

	if ((msg_response_type == MessageResponseType::Error) &&
		((mMsgCmdRespState % 2) != 0) &&
		(mMessageDataOffset <= MSG_HEADER_SIZE))
	{
		mMosiMsgData.sHeader.bCmd |= ABP_MSG_HEADER_E_BIT;
		mMosiMsgData.abData[0] = ABP_ERR_GENERAL_ERROR;
		mMosiMsgData.sHeader.iDataSize = 1;
		mTotalMsgDataBytesToSend = MSG_HEADER_SIZE + 1;

		// Jump back to specific states to simulate the ABCC closing down the file instance correctly, as needed
		switch (static_cast<SimulationState>(mMsgCmdRespState))
		{
		case SimulationState::CreateInstanceResponse:
			// Transition to the end of the 'delete instance', to enter IDLE
			mMsgCmdRespState = (U16)SimulationState::DeleteInstanceResponse;
			mMosiMsgData.abData[0] = ABP_ERR_NO_RESOURCES;
			break;
		case SimulationState::FileOpenResponse:
			// Transition to the end of 'file close', to enter 'delete instance'
			mMsgCmdRespState = (U16)SimulationState::FileCloseResponse;
			SetMosiObjectSpecificError(ABP_FSI_ERR_FILE_OPEN_FAILED);
			break;
		case SimulationState::GetFileSizeResponse:
			// Transition to the end of 'file read', to enter 'file close'
			mAbortTransfer = true;
			mMessageDataOffset = 0;
			mMsgCmdRespState = (U16)SimulationState::FileReadResponse;
			break;
		case SimulationState::FileReadResponse:
			// Abort remaining 'file read' and continue with closing down the file.
			mAbortTransfer = true;
			mMessageDataOffset = 0;
			SetMosiObjectSpecificError(ABP_FSI_ERR_FILE_COPY_OPEN_READ_FAILED);
			break;
		case SimulationState::FileCloseResponse:
			SetMosiObjectSpecificError(ABP_FSI_ERR_FILE_CLOSE_FAILED);
			break;
		case SimulationState::DeleteInstanceResponse:
			SetMosiObjectSpecificError(ABP_FSI_ERR_FILE_DELETE_FAILED);
			break;
		default:
			// Remaining cases are treated no different than normal responses.
			break;
		}
	}
	else
	{
		mMosiMsgData.sHeader.bCmd &= ~ABP_MSG_HEADER_E_BIT;

		// Add in the message header size
		mTotalMsgDataBytesToSend += MSG_HEADER_SIZE;
	}
}

void SpiSimulationDataGenerator::UpdateFileTransferStateMachine()
{
	const U8 idleGapCount = 20;

	if (mMsgCmdRespState < (U16)SimulationState::SizeOfEnum)
	{
		// Update the source ID after completion of each command-response substate
		if ((mMsgCmdRespState % 2) != 0)
		{
			// Update source ID
			mSourceId = (mSourceId + 1) % 256;
		}
	}

	mMsgCmdRespState++;

	// After the sequence has completed, insert the specified idle gap
	// to break up the messaging a bit.
	if (mMsgCmdRespState >= (idleGapCount + (U16)SimulationState::SizeOfEnum))
	{
		mMsgCmdRespState = 0;
	}
}

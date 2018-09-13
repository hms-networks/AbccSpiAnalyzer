/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
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

//TODO joca: add a simulation parameter that causes a clocking violation (exceed 20MHz on SPI clock)

#define _USE_MATH_DEFINES

#define FILE_INSTANCE_TO_USE	0x01
#define FILE_READ_CHUNK_SIZE	511

#define MSG_HEADER_SIZE			12

#define SET_MOSI_OBJECT_SPECIFIC_ERROR(err) \
	mMosiMsgData.sHeader.iDataSize = 2;     \
	mMosiMsgData.abData[0] = 0xFF;          \
	mMosiMsgData.abData[1] = err

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

SpiSimulationDataGenerator::SpiSimulationDataGenerator()
{
	mMsgCmdRespState = (U16)SimulationState::SizeOfEnum;
	mPacketOffset = 0;

	mToggleBit = 0;
	mSourceId = 0;
	mNetTime = 0x00000001;

	memset(&mMisoPacket, 0, sizeof(mMisoPacket));
	memset(&mMosiPacket, 0, sizeof(mMosiPacket));
	memset(&mMisoMsgData, 0, sizeof(mMisoMsgData));
	memset(&mMosiMsgData, 0, sizeof(mMosiMsgData));

	mMosiPacket.pdLen = ABCC_CFG_MAX_PROCESS_DATA_SIZE >> 1;
	mMosiPacket.msgLen = ABCC_CFG_SPI_MSG_FRAG_LEN >> 1;
	mMisoPacket.anbStat = ABP_ANB_STATE_PROCESS_ACTIVE;

	mAbortTransfer = false;
}

SpiSimulationDataGenerator::~SpiSimulationDataGenerator()
{
}

void SpiSimulationDataGenerator::Initialize(U32 simulation_sample_rate, SpiAnalyzerSettings* settings)
{
	const double maxAbccSpiClockFrequencyHz = 20000000.0;
	const double min3WireAbccSpiClockFrequencyHz = 100000.0;

	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	// Use a 1/10th rule for clock frequency versus sample rate to provide good sample characteristics
	mTargetClockFrequencyHz = mSimulationSampleRateHz / 10;

	if (mTargetClockFrequencyHz > maxAbccSpiClockFrequencyHz)
	{
		mTargetClockFrequencyHz = maxAbccSpiClockFrequencyHz;
	}

	if (((mEnable == nullptr) && (mSettings->m4WireOn3Channels == false)) || (mSettings->m3WireOn4Channels == true))
	{
		if (mTargetClockFrequencyHz < min3WireAbccSpiClockFrequencyHz)
		{
			mTargetClockFrequencyHz = min3WireAbccSpiClockFrequencyHz;
		}
	}

	mClockGenerator.Init(mTargetClockFrequencyHz, mSimulationSampleRateHz);

	if (settings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mMiso = mSpiSimulationChannels.Add(settings->mMisoChannel, mSimulationSampleRateHz, BitState::BIT_LOW);
	}
	else
	{
		mMiso = nullptr;
	}

	if (settings->mMosiChannel != UNDEFINED_CHANNEL)
	{
		mMosi = mSpiSimulationChannels.Add(settings->mMosiChannel, mSimulationSampleRateHz, BitState::BIT_LOW);
	}
	else
	{
		mMosi = nullptr;
	}

	mClock = mSpiSimulationChannels.Add(settings->mClockChannel, mSimulationSampleRateHz, BitState::BIT_HIGH);

	if (settings->mEnableChannel != UNDEFINED_CHANNEL)
	{
		BitState enableInitState = BitState::BIT_HIGH;

		if (mSettings->m3WireOn4Channels)
		{
			enableInitState = BitState::BIT_LOW;
		}

		mEnable = mSpiSimulationChannels.Add(settings->mEnableChannel, mSimulationSampleRateHz, enableInitState);
	}
	else
	{
		mEnable = nullptr;
	}

	// Insert 1.5 * the minimum 3-wire idle gap time
	mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * (1.5f * MIN_IDLE_GAP_TIME )));

	mValue = 0;
}

U32 SpiSimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	U64 adjustedLargestSampleRequested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

	while (mClock->GetCurrentSampleNumber() < adjustedLargestSampleRequested)
	{
		CreateSpiTransaction();

		// Insert 1.5 * the minimum 3-wire idle gap time
		mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * (1.5f * MIN_IDLE_GAP_TIME )));
	}

	*simulation_channels = mSpiSimulationChannels.GetArray();
	return mSpiSimulationChannels.GetCount();
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
	x = 2.0f * 3.14159265359f * freq * t;
	mMosiProcessData = (S32)(65535 * sin(x));
	mMisoProcessData = (S32)(65535 * cos(x));

	memcpy(&mMosiPacket.processData, &mMosiProcessData, sizeof(mMosiProcessData));
	memcpy(&mMisoPacket.processData, &mMisoProcessData, sizeof(mMisoProcessData));

	mMosiPacket.spiCtrl |= ABP_SPI_CTRL_WRPD_VALID;
	mMisoPacket.spiStat |= ABP_SPI_STATUS_NEW_PD;
}

void SpiSimulationDataGenerator::CreateSpiTransaction()
{
	ClockIdleLevel clockIdleHigh;
	AbccCrc mosiChecksum = AbccCrc();
	AbccCrc misoChecksum = AbccCrc();

	std::random_device rd;
	std::mt19937 prng(rd());

	std::uniform_int_distribution<> fragmentSize(1, sizeof(AbccMosiPacket_t) - 1);

	// Create a set of random bernoulli random
	// sequences to control random events in the simulation
	std::bernoulli_distribution generateClockIdleLow(0.10);
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
	bool mosiCrcError = generateMosiCrcError(prng);
	bool misoCrcError = generateMisoCrcError(prng) || mosiCrcError;

	bool oneByteFragmentError = false;
	bool clockIdleLow = generateClockIdleLow(prng);
	bool fragmentError = generateFragmentError(prng);
	bool errorResponse = generateMosiErrorRespMsg(prng);
	bool clockingError = generateClockingError(prng);
	bool outOfBandClocking = generateOutOfBandClocking(prng);
	bool errorPresent = false;

	U8* pMosiData = (U8*)&mMosiMsgData;
	U8* pMisoData = (U8*)&mMisoMsgData;

	if (fragmentError || misoCrcError || mosiCrcError)
	{
		errorPresent = true;
	}

	if (((mEnable == nullptr) && (mSettings->m4WireOn3Channels == false)) || (mSettings->m3WireOn4Channels == true))
	{
		oneByteFragmentError = generate1ByteFragError(prng);
		errorPresent = errorPresent || oneByteFragmentError;
	}

	// Reinitialize SPI packets each cycle
	memset(&mMisoPacket, 0, sizeof(mMisoPacket));
	memset(&mMosiPacket, 0, sizeof(mMosiPacket));

	// Obtain time information from the analyzer's current sample and sample frequency
	mNetTime = (U32)(mClock->GetCurrentSampleNumber() / (double)mSimulationSampleRateHz * (double)1e9) + 1;

	// Update the network time
	mMisoPacket.netTime_lo = mNetTime & 0xFFFF;
	mMisoPacket.netTime_hi = (mNetTime >> 16) & 0xFFFF;

	UpdateProcessData();

	mMosiPacket.pdLen = ABCC_CFG_MAX_PROCESS_DATA_SIZE >> 1;
	mMosiPacket.msgLen = ABCC_CFG_SPI_MSG_FRAG_LEN >> 1;
	mMosiPacket.spiCtrl |= mToggleBit | ABP_SPI_CTRL_CMDCNT;
	mMisoPacket.spiStat |= ABP_SPI_STATUS_CMDCNT;
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

	// If a valid message is available, copy message data to SPI buffer.
	if (mMisoPacket.spiStat & ABP_SPI_STATUS_M)
	{
		memcpy(mMisoPacket.msgData, &pMisoData[mPacketOffset], ABCC_CFG_SPI_MSG_FRAG_LEN);
	}

	if (mMosiPacket.spiCtrl & ABP_SPI_CTRL_M)
	{
		memcpy(mMosiPacket.msgData, &pMosiData[mPacketOffset], ABCC_CFG_SPI_MSG_FRAG_LEN);
	}

	// Update the LAST_FRAG flag to indicate if more fragments follow or not
	if ((mPacketOffset + ABCC_CFG_SPI_MSG_FRAG_LEN) < mPacketMsgFieldSize)
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
	else
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

	// Update the CRC32
	mosiChecksum.Init();
	misoChecksum.Init();

	mosiChecksum.Update(&((U8*)&mMosiPacket)[0], sizeof(AbccMosiPacket_t) - 6);
	misoChecksum.Update(&((U8*)&mMisoPacket)[0], sizeof(AbccMisoPacket_t) - 4);
	mMosiPacket.crc32_lo = mosiChecksum.Crc32() & 0xFFFF;
	mMosiPacket.crc32_hi = (mosiChecksum.Crc32() >> 16) & 0xFFFF;
	mMisoPacket.crc32_lo = misoChecksum.Crc32() & 0xFFFF;
	mMisoPacket.crc32_hi = (misoChecksum.Crc32() >> 16) & 0xFFFF;

	// Create occasional CRC errors
	if (mosiCrcError)
	{
		mMosiPacket.crc32_lo++;
	}

	if (misoCrcError)
	{
		mMisoPacket.crc32_lo++;
	}

	clockIdleHigh = static_cast<ClockIdleLevel>(mClock->GetCurrentBitState() == BitState::BIT_HIGH);

	if ((mEnable != nullptr) && (mSettings->m3WireOn4Channels == false))
	{
		// Assert SPI Enable and move forward in time
		mEnable->Transition();
		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2.0));

		if (fragmentError)
		{
			// Create a fragmented SPI packet which will be short by 1 or more bytes
			SendPacketData(clockIdleHigh, fragmentSize(prng));
		}
		else
		{
			// Produce the SPI packet
			SendPacketData(clockIdleHigh, sizeof(AbccMosiPacket_t));

			if (clockingError)
			{
				// Send an additional SPI packet before enable goes high (causes clocking errors)
				SendPacketData(clockIdleHigh, sizeof(AbccMosiPacket_t));
			}
		}

		// Deassert SPI Enable
		mEnable->Transition();

		if (outOfBandClocking)
		{
			// Send an out-of-band SPI packet, this communication is ignored by the analyzer
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			OutputByte_CPOL1_CPHA1(mValue, mValue + 1);
			mValue++;
		}

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));

		// Select between "Clock Idle Low" and "Clock Idle High" SPI configurations
		if (clockIdleLow)
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
			SendPacketData(clockIdleHigh, fragmentSize(prng));
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
		}
		else if (oneByteFragmentError)
		{
			// Create a fragmented SPI packet (1 byte)
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			OutputByte_CPOL1_CPHA1(mValue, mValue + 1);
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			mValue++;
		}
		else
		{
			// Produce the SPI packet
			SendPacketData(clockIdleHigh, sizeof(AbccMosiPacket_t));

			if (clockingError)
			{
				// Send an additional SPI packet before enable goes high (causes clocking errors)
				mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
				OutputByte_CPOL1_CPHA1(mValue, mValue + 1);
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
				mValue++;
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
		mPacketOffset += ABCC_CFG_SPI_MSG_FRAG_LEN;

		// Check if full message has been sent, if so update the
		// state of the file transfer state machine, and reset
		// the fragmentation packet offset
		if ((mPacketOffset >= mPacketMsgFieldSize) || mAbortTransfer)
		{
			mAbortTransfer = false;
			mPacketOffset = 0;
			UpdateFileTransferStateMachine();
		}
	}
}

void SpiSimulationDataGenerator::SendPacketData(ClockIdleLevel clock_idle_level, U32 length)
{
	if (length > sizeof(AbccMosiPacket_t))
	{
		length = sizeof(AbccMosiPacket_t);
	}

	for (U16 i = 0; i < length; i++)
	{
		if (clock_idle_level == ClockIdleLevel::High)
		{
			OutputByte_CPOL1_CPHA1(((U8*)&mMosiPacket)[i], ((U8*)&mMisoPacket)[i]);
		}
		else
		{
			OutputByte_CPOL0_CPHA0(((U8*)&mMosiPacket)[i], ((U8*)&mMisoPacket)[i]);
		}
	}
}

void SpiSimulationDataGenerator::OutputByte_CPOL0_CPHA0(U64 mosi_data, U64 miso_data)
{
	const U32 bitsPerTransfer = 8;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);

	// First ensure clock is low
	if (mClock->GetCurrentBitState() == BitState::BIT_HIGH)
	{
		// Wrong beginning polarity, don't bother sending anything
		return;
	}

	for (auto i = 0; i < bitsPerTransfer; i++)
	{
		mMosi->TransitionIfNeeded(mosi_bits.GetNextBit());
		mMiso->TransitionIfNeeded(miso_bits.GetNextBit());

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();
	}

	mMosi->TransitionIfNeeded(BitState::BIT_LOW);
	mMiso->TransitionIfNeeded(BitState::BIT_LOW);

	mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
}

void SpiSimulationDataGenerator::OutputByte_CPOL1_CPHA1(U64 mosi_data, U64 miso_data)
{
	const U32 bitsPerTransfer = 8;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);

	// First ensure clock is high
	if (mClock->GetCurrentBitState() == BitState::BIT_LOW)
	{
		// Wrong beginning polarity, don't bother sending anything
		return;
	}

	for (auto i = 0; i < bitsPerTransfer; i++)
	{
		mClock->Transition();
		mMosi->TransitionIfNeeded(mosi_bits.GetNextBit());
		mMiso->TransitionIfNeeded(miso_bits.GetNextBit());

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
	}

	mMosi->TransitionIfNeeded(BitState::BIT_LOW);
	mMiso->TransitionIfNeeded(BitState::BIT_LOW);

	mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
}

void SpiSimulationDataGenerator::CreateFileInstance(ABP_MsgType *msg_ptr, MessageType message_type)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = 0x0000;
	msg_ptr->sHeader.bCmd = ABP_CMD_CREATE;
	msg_ptr->sHeader.bCmdExt0 = 0x00;
	msg_ptr->sHeader.bCmdExt1 = 0x00;

	switch(message_type)
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

void SpiSimulationDataGenerator::FileOpen(ABP_MsgType* msg_ptr, MessageType message_type, CHAR* file_name, UINT8 file_name_length)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = (U16)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmd = ABP_FSI_CMD_FILE_OPEN;
	msg_ptr->sHeader.bCmdExt0 = 0x00; // Read only mode
	msg_ptr->sHeader.bCmdExt1 = 0x00;

	switch(message_type)
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

	switch(message_type)
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

void SpiSimulationDataGenerator::FileRead(ABP_MsgType* msg_ptr, MessageType message_type, CHAR* file_data, UINT32 file_data_length)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = (U16)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmd = ABP_FSI_CMD_FILE_READ;

	switch(message_type)
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

	switch(message_type)
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

void SpiSimulationDataGenerator::DeleteFileInstance(ABP_MsgType *msg_ptr, MessageType message_type)
{
	msg_ptr->sHeader.bDestObj = ABP_OBJ_NUM_AFSI;
	msg_ptr->sHeader.iInstance = 0x0000;
	msg_ptr->sHeader.bCmd = ABP_CMD_DELETE;
	msg_ptr->sHeader.bCmdExt0 = (U8)FILE_INSTANCE_TO_USE;
	msg_ptr->sHeader.bCmdExt1 = 0x00;
	msg_ptr->sHeader.iDataSize = 0x0000;

	switch(message_type)
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
		mPacketMsgFieldSize = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::CreateInstanceResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		CreateFileInstance(&mMosiMsgData, MessageType::Response);
		mPacketMsgFieldSize = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileOpenCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		FileOpen(&mMisoMsgData, MessageType::Command, (CHAR*)&filename[0], sizeof(filename) - 1);
		mPacketMsgFieldSize = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileOpenResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		FileOpen(&mMosiMsgData, MessageType::Response, nullptr, 0);
		mPacketMsgFieldSize = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::GetFileSizeCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		GetFileSize(&mMisoMsgData, MessageType::Command);
		mPacketMsgFieldSize = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::GetFileSizeResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		GetFileSize(&mMosiMsgData, MessageType::Response);
		mPacketMsgFieldSize = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileReadCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		FileRead(&mMisoMsgData, MessageType::Command, nullptr, 0);
		mPacketMsgFieldSize = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileReadResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		FileRead(&mMosiMsgData, MessageType::Response, (CHAR*)&fileData[0], sizeof(fileData) - 1);
		mPacketMsgFieldSize = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileCloseCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		FileClose(&mMisoMsgData, MessageType::Command, 0);
		mPacketMsgFieldSize = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::FileCloseResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		FileClose(&mMosiMsgData, MessageType::Response, sizeof(fileData) - 1);
		mPacketMsgFieldSize = mMosiMsgData.sHeader.iDataSize;
		break;
	case SimulationState::DeleteInstanceCommand:
		mMisoPacket.spiStat |= ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		DeleteFileInstance(&mMisoMsgData, MessageType::Command);
		mPacketMsgFieldSize = mMisoMsgData.sHeader.iDataSize;
		break;
	case SimulationState::DeleteInstanceResponse:
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl |= ABP_SPI_CTRL_M;
		DeleteFileInstance(&mMosiMsgData, MessageType::Response);
		mPacketMsgFieldSize = mMosiMsgData.sHeader.iDataSize;
		break;
	default:
		/* no message */
		mMisoPacket.spiStat &= ~ABP_SPI_STATUS_M;
		mMosiPacket.spiCtrl &= ~ABP_SPI_CTRL_M;
		mPacketMsgFieldSize = 0;
		break;
	}

	if ((msg_response_type == MessageResponseType::Error) &&
		((mMsgCmdRespState % 2) != 0) &&
		(mPacketOffset <= MSG_HEADER_SIZE))
	{
		mMosiMsgData.sHeader.bCmd |= ABP_MSG_HEADER_E_BIT;
		mMosiMsgData.abData[0] = ABP_ERR_GENERAL_ERROR;
		mMosiMsgData.sHeader.iDataSize = 1;
		mPacketMsgFieldSize = MSG_HEADER_SIZE + 1;

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
			SET_MOSI_OBJECT_SPECIFIC_ERROR(ABP_FSI_ERR_FILE_OPEN_FAILED);
			break;
		case SimulationState::GetFileSizeResponse:
			// Transition to the end of 'file read', to enter 'file close'
			mAbortTransfer = true;
			mPacketOffset = 0;
			mMsgCmdRespState = (U16)SimulationState::FileReadResponse;
			break;
		case SimulationState::FileReadResponse:
			// Abort remaining 'file read' and continue with closing down the file.
			mAbortTransfer = true;
			mPacketOffset = 0;
			SET_MOSI_OBJECT_SPECIFIC_ERROR(ABP_FSI_ERR_FILE_COPY_OPEN_READ_FAILED);
			break;
		case SimulationState::FileCloseResponse:
			SET_MOSI_OBJECT_SPECIFIC_ERROR(ABP_FSI_ERR_FILE_CLOSE_FAILED);
			break;
		case SimulationState::DeleteInstanceResponse:
			SET_MOSI_OBJECT_SPECIFIC_ERROR(ABP_FSI_ERR_FILE_DELETE_FAILED);
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
		mPacketMsgFieldSize += MSG_HEADER_SIZE;
	}
}

void SpiSimulationDataGenerator::UpdateFileTransferStateMachine(void)
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

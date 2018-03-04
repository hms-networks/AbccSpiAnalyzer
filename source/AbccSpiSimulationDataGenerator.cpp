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

#include <random>
#include "AbccSpiSimulationDataGenerator.h"
#include "AbccSpiAnalyzerSettings.h"
#include "AbccSpiAnalyzer.h"
#include "AbccCrc.h"
#include "abcc_td.h"
#include "abcc_abp/abp.h"

/* Initial MOSI frame state */
static const tAbccMosiPacket sMosiData =
{
	0x1F,						/* SPI_CTRL */
	0x00,						/* RESERVED */
	0x0008,						/* MSG_LEN */
	0x0002,						/* PD_LEN */
	0x00,						/* APP_STAT */
	0x10,						/* INT_MSK */
	0x0004,						/* MD_SIZE */
	0x0000,						/* RESERVED */
	0x00,						/* SRC_ID */
	0xFE,						/* OBJ */
	0x0001,						/* INST */
	0x01,						/* CMD */
	0x00,						/* RESEVERED */
	0x0005,						/* CMDEXT */
	{ 0x11, 0x22, 0x33, 0x44 },	/* MSG_DATA */
	{ 0x1A, 0x2B, 0x3C, 0x4D },	/* PROCESS_DATA */
	0x0000,						/* CRC32 LOW */
	0x0000,						/* CRC32 HIGH */
	0x0000						/* PAD */
};

/* Initial MISO frame state */
static const tAbccMisoPacket sMisoData =
{
	0x0000,						/* RESERVED */
	0x0000,						/* LED_STAT */
	0x0C,						/* ANB_STAT */
	0x3E,						/* SPI_STAT */
	0x0000,						/* NET_TIME LOW */
	0x0000,						/* NET_TIME HIGH */
	0x0000,						/* MD_SIZE */
	0x0000,						/* RESERVED */
	0x00,						/* SRC_ID */
	0xFE,						/* OBJ */
	0x0001,						/* INST */
	0x41,						/* CMD */
	0x00,						/* RESERVED */
	0x0005,						/* CMDEXT */
	{ 0x00, 0x00, 0x00, 0x00 },	/* MSG_DATA */
	{ 0xAA, 0xBB, 0xCC, 0xDD },	/* PROCESS_DATA */
	0x0000,						/* CRC32 LOW */
	0x0000						/* CRC32 HIGH */
};

SpiSimulationDataGenerator::SpiSimulationDataGenerator()
{
	mMsgCmdRespState = 0;
	mNetTime = 0x00000001;
	memcpy(&mMosiData, &sMosiData, sizeof(tAbccMosiPacket));
	memcpy(&mMisoData, &sMisoData, sizeof(tAbccMosiPacket));
}

SpiSimulationDataGenerator::~SpiSimulationDataGenerator()
{
}

void SpiSimulationDataGenerator::Initialize(U32 simulation_sample_rate, SpiAnalyzerSettings* settings)
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mClockGenerator.Init(simulation_sample_rate / 10, simulation_sample_rate);

	if (settings->mMisoChannel != UNDEFINED_CHANNEL)
		mMiso = mSpiSimulationChannels.Add(settings->mMisoChannel, mSimulationSampleRateHz, BIT_LOW);
	else
		mMiso = NULL;

	if (settings->mMosiChannel != UNDEFINED_CHANNEL)
		mMosi = mSpiSimulationChannels.Add(settings->mMosiChannel, mSimulationSampleRateHz, BIT_LOW);
	else
		mMosi = NULL;

	mClock = mSpiSimulationChannels.Add(settings->mClockChannel, mSimulationSampleRateHz, BIT_HIGH);

	if (settings->mEnableChannel != UNDEFINED_CHANNEL)
	{
		BitState enableInitState = BIT_HIGH;
		if(mSettings->m3WireOn4Channels)
		{
			enableInitState = BIT_LOW;
		}
		mEnable = mSpiSimulationChannels.Add(settings->mEnableChannel, mSimulationSampleRateHz, enableInitState);
	}
	else
		mEnable = NULL;

	/* Insert >10us idle */
	mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));

	mValue = 0;
}

U32 SpiSimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	U64 adjustedLargestSampleRequested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

	while (mClock->GetCurrentSampleNumber() < adjustedLargestSampleRequested)
	{
		CreateSpiTransaction();
		/* Insert >10us idle */
		mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
	}

	*simulation_channels = mSpiSimulationChannels.GetArray();
	return mSpiSimulationChannels.GetCount();
}

void SpiSimulationDataGenerator::CreateSpiTransaction()
{
	const int maxMsgCmdRespStates = 7;
	bool clockIdleHigh;
	std::random_device rd;
	std::mt19937 prng(rd());
	std::uniform_int_distribution<> distr(0, 100);
	AbccCrc mosiChecksum = AbccCrc();
	AbccCrc misoChecksum = AbccCrc();

	bool generateClockIdleLow = (distr(prng) < 10);
	bool generateOutOfBandClocking = (distr(prng) < 5);
	bool generateMosiErrorRespMsg = (distr(prng) < 1);

	bool generateClockingError = (distr(prng) < 1);

	bool generateMisoCrcError = (distr(prng) < 2);
	bool generateMosiCrcError = (distr(prng) < 2);
	bool generateFragmentError = (distr(prng) < 1);
	bool generate1ByteFragError = false;
	bool errorPresent = false;

	/* In this simulation, a MOSI CRC error implies a MISO CRC error as well
	** which simulates the error detection/reporting mechanism of the ABCC. */
	generateMisoCrcError = generateMisoCrcError || generateMosiCrcError;

	if (generateFragmentError || generateMisoCrcError || generateMosiCrcError)
	{
		errorPresent = true;
	}

	if (((mEnable == NULL) && (mSettings->m4WireOn3Channels == false)) || (mSettings->m3WireOn4Channels == true))
	{
		generate1ByteFragError = (distr(prng) < 1);
		errorPresent = errorPresent || generate1ByteFragError;
	}

	clockIdleHigh = (mClock->GetCurrentBitState() == BIT_HIGH);

	if ((mEnable != NULL) && (mSettings->m3WireOn4Channels == false))
		mEnable->Transition();

	mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2.0));

	switch (mMsgCmdRespState)
	{
	case 1:
		/* new miso (command) message */
		mMisoData.spiStat |= (ABP_SPI_STATUS_M | ABP_SPI_STATUS_LAST_FRAG);
		break;
	case 2:
		/* new mosi (response) message */
		mMosiData.spiCtrl |= (ABP_SPI_CTRL_M | ABP_SPI_CTRL_LAST_FRAG);
		if (generateMosiErrorRespMsg)
		{
			mMosiData.cmd |= ABP_MSG_HEADER_E_BIT;
			mMosiData.msgData[0] = ABP_ERR_GENERAL_ERROR;
		}
		else
		{
			mMosiData.cmd &= ~ABP_MSG_HEADER_E_BIT;
			mMosiData.msgData[0] = 0x11;
		}
		break;
	default:
		/* no message */
		break;
	}

	/* Update Network Time */
	mMisoData.netTime_lo = mNetTime & 0xFFFF;
	mMisoData.netTime_hi = (mNetTime >> 16) & 0xFFFF;
	mNetTime += 0x1234;

	/* Update the CRC32 */
	mosiChecksum.Init();
	misoChecksum.Init();
	mosiChecksum.Update(&((U8*)&mMosiData)[0], sizeof(tAbccMosiPacket) - 6);
	misoChecksum.Update(&((U8*)&mMisoData)[0], sizeof(tAbccMisoPacket) - 4);
	mMosiData.crc32_lo = mosiChecksum.Crc32() & 0xFFFF;
	mMosiData.crc32_hi = (mosiChecksum.Crc32() >> 16) & 0xFFFF;
	mMisoData.crc32_lo = misoChecksum.Crc32() & 0xFFFF;
	mMisoData.crc32_hi = (misoChecksum.Crc32() >> 16) & 0xFFFF;

	/* Create occasional CRC errors */
	if (generateMosiCrcError)
	{
		mMosiData.crc32_lo++;
	}

	if (generateMisoCrcError)
	{
		mMisoData.crc32_lo++;
	}

	if ((mEnable != NULL) && (mSettings->m3WireOn4Channels == false))
	{
		if (generateFragmentError)
		{
			/* Create a fragment (short 1 byte) */
			mEnable->Transition();
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			mEnable->Transition();
			mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2.0));
			SendPacketData(clockIdleHigh, sizeof(tAbccMosiPacket)-1);
		}
		else
		{
			/* Produce the transaction */
			SendPacketData(clockIdleHigh, sizeof(tAbccMosiPacket));

			/* Occasionally create additional disturbances after the packet */
			if (generateClockingError)
			{
				/* Create an additional transaction before enable goes high (cause clocking errors) */
				SendPacketData(clockIdleHigh, sizeof(tAbccMosiPacket));
			}
		}

		mEnable->Transition();
		if (generateOutOfBandClocking)
		{
			/* Create an out-of-band SPI transaction to illustrate that the analyzer will
			** ignore data as long as the enable-line is high */
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			OutputByte_CPOL1_CPHA1(mValue, mValue + 1);
			mValue++;
		}

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));

		/* Switch Clock IDLE LOW/HIGH */
		if (generateClockIdleLow)
		{
			mClock->TransitionIfNeeded(BIT_LOW);
		}
		else
		{
			mClock->TransitionIfNeeded(BIT_HIGH);
		}
	}
	else
	{
		if (generateFragmentError)
		{
			/* Create a fragment (1 byte) */
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			OutputByte_CPOL1_CPHA1(mValue, mValue + 1);
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			mValue++;
		}
		else if (generate1ByteFragError)
		{
			/* Create a fragment (short 1 byte) */
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			SendPacketData(clockIdleHigh, sizeof(tAbccMosiPacket)-1);
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
		}
		else
		{
			/* Produce the transaction */
			SendPacketData(clockIdleHigh, sizeof(tAbccMosiPacket));

			/* Occasionally create additional disturbances after the packet (~10% chance) */
			if (generateClockingError)
			{
				/* Add extra clocking */
				mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
				OutputByte_CPOL1_CPHA1(mValue, mValue + 1);
				mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
				mValue++;
			}
		}
	}

	/* Update toggle bit only when no error in communication was
	** generated. The toggle bit should be left as-is in case of
	** errors to indicate the need for a retransmission. */
	if (!errorPresent)
	{
		/* Update toggle bit */
		mMosiData.spiCtrl ^= ABP_SPI_CTRL_T;

		/* Deassert message flags */
		mMisoData.spiStat &= ~(ABP_SPI_STATUS_M | ABP_SPI_STATUS_LAST_FRAG);
		mMosiData.spiCtrl &= ~(ABP_SPI_CTRL_M | ABP_SPI_CTRL_LAST_FRAG);

		mMsgCmdRespState++;

		/* Update the source ID each cycle of the message command-response state */
		if (mMsgCmdRespState >= maxMsgCmdRespStates)
		{
			mMsgCmdRespState = 0;

			/* Update source ID */
			mMisoData.srcId++;
			mMosiData.srcId = mMisoData.srcId;
		}
	}
}

void SpiSimulationDataGenerator::SendPacketData(bool is_clock_idle_high, U32 length)
{
	if (length > sizeof(tAbccMosiPacket))
	{
		length = sizeof(tAbccMosiPacket);
	}
	for (U16 i = 0; i < length; i++)
	{
		if (is_clock_idle_high == true)
		{
			OutputByte_CPOL1_CPHA1(((uAbccPacket*)&mMosiData)->raw[i], ((uAbccPacket*)&mMisoData)->raw[i]);
		}
		else
		{
			OutputByte_CPOL0_CPHA0(((uAbccPacket*)&mMosiData)->raw[i], ((uAbccPacket*)&mMisoData)->raw[i]);
		}
	}
}

void SpiSimulationDataGenerator::OutputByte_CPOL0_CPHA0(U64 mosi_data, U64 miso_data)
{
	const U32 bitsPerTransfer = 8;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);

	/* First ensure clock is low */
	if (mClock->GetCurrentBitState() == BIT_HIGH)
	{
		/* Wrong beginning polarity, don't bother sending anything */
		return;
	}

	for (U32 i = 0; i < bitsPerTransfer; i++)
	{
		mMosi->TransitionIfNeeded(mosi_bits.GetNextBit());
		mMiso->TransitionIfNeeded(miso_bits.GetNextBit());

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();  /* data invalid */

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();  /* data invalid */
	}

	mMosi->TransitionIfNeeded(BIT_LOW);
	mMiso->TransitionIfNeeded(BIT_LOW);

	mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
}

void SpiSimulationDataGenerator::OutputByte_CPOL1_CPHA1(U64 mosi_data, U64 miso_data)
{
	const U32 bitsPerTransfer = 8;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, bitsPerTransfer);

	/* First ensure clock is high */
	if (mClock->GetCurrentBitState() == BIT_LOW)
	{
		/* Wrong beginning polarity, don't bother sending anything */
		return;
	}

	for (U32 i = 0; i < bitsPerTransfer; i++)
	{
		mClock->Transition();  /* data invalid */
		mMosi->TransitionIfNeeded(mosi_bits.GetNextBit());
		mMiso->TransitionIfNeeded(miso_bits.GetNextBit());

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
		mClock->Transition();  /* data invalid */

		mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
	}

	mMosi->TransitionIfNeeded(BIT_LOW);
	mMiso->TransitionIfNeeded(BIT_LOW);

	mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
}

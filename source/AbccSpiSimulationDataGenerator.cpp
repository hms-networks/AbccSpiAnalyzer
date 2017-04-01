/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
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

static tAbccMosiPacket sMosiData = { 0x1F,		/* SPI_CTRL */
									 0x00,		/* RESERVED */
									 0x0008,	/* MSG_LEN */
									 0x0002,	/* PD_LEN */
									 0x00,		/* APP_STAT */
									 0x10,		/* INT_MSK */
									 0x0000,	/* MD_SIZE */
									 0x0000,	/* RESERVED */
									 0x01,		/* SRC_ID */
									 0xFE,		/* OBJ */
									 0x0001,	/* INST */
									 0x41,		/* CMD */
									 0x00,		/* RESEVERED */
									 0x0005,	/* CMDEXT */
									 { 0x00, 0x00, 0x00, 0x00 },	/* MSG_DATA */
									 { 0x11, 0x22, 0x33, 0x44 },	/* PROCESS_DATA */
									 0x0000,	/* CRC32 LOW */
									 0x0000,	/* CRC32 HIGH */
									 0x0000 };	/* PAD */

static tAbccMisoPacket sMisoData = { 0x0000,	/* RESERVED */
									 0x0000,	/* LED_STAT */
									 0x0C,		/* ANB_STAT */
									 0x3E,		/* SPI_STAT */
									 0x0000,	/* NET_TIME LOW */
									 0x0000,	/* NET_TIME HIGH */
									 0x0004,	/* MD_SIZE */
									 0x0000,	/* RESERVED */
									 0x00,		/* SRC_ID */
									 0xFE,		/* OBJ */
									 0x0001,	/* INST */
									 0x01,		/* CMD */
									 0x00,		/* RESERVED */
									 0x0005,	/* CMDEXT */
									 { 0x11, 0x22, 0x33, 0x44 },	/* MSG_DATA */
									 { 0xAA, 0xBB, 0xCC, 0xDD },	/* PROCESS_DATA */
									 0x0000,	/* CRC32 LOW */
									 0x0000 };	/* CRC32 HIGH */

SpiSimulationDataGenerator::SpiSimulationDataGenerator()
{
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
		mEnable = mSpiSimulationChannels.Add(settings->mEnableChannel, mSimulationSampleRateHz, BIT_HIGH);
	else
		mEnable = NULL;

	/* Insert >10us idle */
	mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));

	mValue = 0;
}

U32 SpiSimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

	while (mClock->GetCurrentSampleNumber() < adjusted_largest_sample_requested)
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
	bool fClockIdleHigh;
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(0, 100);
	static U32 netTime = 0x00000001;
	AbccCrc mosiChecksum = AbccCrc();
	AbccCrc misoChecksum = AbccCrc();

	fClockIdleHigh = (mClock->GetCurrentBitState() == BIT_HIGH);

	if (mEnable != NULL)
		mEnable->Transition();

	mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2.0));

	/* Update toggle bit */
	sMosiData.spiCtrl ^= ABP_SPI_CTRL_T;

	/* Update Network Time */
	sMisoData.netTime_lo = netTime & 0xFFFF;
	sMisoData.netTime_hi = (netTime >> 16) & 0xFFFF;
	netTime += 0x1234;

	/* Update source ID */
	sMisoData.srcId = sMosiData.srcId;
	sMosiData.srcId++;

	/* Update the CRC32 */
	mosiChecksum.Init();
	misoChecksum.Init();
	mosiChecksum.Update(&((U8*)&sMosiData)[0], sizeof(tAbccMosiPacket) - 6);
	misoChecksum.Update(&((U8*)&sMisoData)[0], sizeof(tAbccMisoPacket) - 4);
	sMosiData.crc32_lo = mosiChecksum.Crc32() & 0xFFFF;
	sMosiData.crc32_hi = (mosiChecksum.Crc32() >> 16) & 0xFFFF;
	sMisoData.crc32_lo = misoChecksum.Crc32() & 0xFFFF;
	sMisoData.crc32_hi = (misoChecksum.Crc32() >> 16) & 0xFFFF;

	/* Create occasional CRC errors */
	if (distr(eng) < 10)
	{
		sMosiData.crc32_lo++;
	}
	else if (distr(eng) < 10)
	{
		sMisoData.crc32_lo++;
	}

	/* Produce the transaction (10% using CPHA0; 90% using CPHA1) */
	for (U16 i = 0; i < sizeof(tAbccMosiPacket); i++)
	{
		if (fClockIdleHigh == true)
		{
			OutputWord_CPHA1(((uAbccPacket*)&sMosiData)->raw[i], ((uAbccPacket*)&sMisoData)->raw[i]);
		}
		else
		{
			OutputWord_CPHA0(((uAbccPacket*)&sMosiData)->raw[i], ((uAbccPacket*)&sMisoData)->raw[i]);
		}
	}

	if (mEnable != NULL)
	{
		/* Occassionally create additional distubances after the packet (~20% chance) */
		if (distr(eng) < 10)
		{
			/* Create an additional transaction before enable goes high (cause clocking errors) */
			for (U16 i = 0; i < sizeof(tAbccMosiPacket); i++)
			{
				OutputWord_CPHA1(((uAbccPacket*)&sMosiData)->raw[i], ((uAbccPacket*)&sMisoData)->raw[i]);
			}
		}
		else if (distr(eng) < 20)
		{
			/* Create a fragment (short 1 byte) */
			mEnable->Transition();
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			mEnable->Transition();
			mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2.0));
			for (U16 i = 0; i < sizeof(tAbccMosiPacket) - 1; i++)
			{
				OutputWord_CPHA1(((uAbccPacket*)&sMosiData)->raw[i], ((uAbccPacket*)&sMisoData)->raw[i]);
			}
		}

		/* Create an out-of-band SPI transaction to illustrate that the analyzer will
		** ignore data as long as the enable-line is high */
		mEnable->Transition();
		mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
		OutputWord_CPHA1(mValue, mValue + 1);
		mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
		mValue++;

		/* Switch Clock IDLE LOW/HIGH (10/90%) */
		if (distr(eng) < 10)
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
		/* Occassionally create additional distubances after the packet (~20% chance) */
		if (distr(eng) < 10)
		{
			/* Add extra clocking */
			mSpiSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(0.5));
			OutputWord_CPHA1(mValue, mValue + 1);
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			mValue++;
		}
		else if (distr(eng) < 15)
		{
			/* Create a fragment (1 byte) */
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			OutputWord_CPHA1(mValue, mValue + 1);
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			mValue++;
		}
		else if (distr(eng) < 20)
		{
			/* Create a fragment (short 1 byte) */
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
			for (U16 i = 0; i < sizeof(tAbccMosiPacket) - 1; i++)
			{
				OutputWord_CPHA1(((uAbccPacket*)&sMosiData)->raw[i], ((uAbccPacket*)&sMisoData)->raw[i]);
			}
			mSpiSimulationChannels.AdvanceAll((U32)(mSimulationSampleRateHz * MIN_IDLE_GAP_TIME));
		}
	}
}

void SpiSimulationDataGenerator::OutputWord_CPHA0(U64 mosi_data, U64 miso_data)
{
	const U32 dwBitsPerTransfer = 8;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);

	/* First ensure clock is low */
	if (mClock->GetCurrentBitState() == BIT_HIGH)
	{
		/* Wrong beginning polarity, don't bother sending anything */
		return;
	}

	for (U32 i = 0; i < dwBitsPerTransfer; i++)
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

void SpiSimulationDataGenerator::OutputWord_CPHA1(U64 mosi_data, U64 miso_data)
{
	const U32 dwBitsPerTransfer = 8;
	BitExtractor mosi_bits(mosi_data, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);
	BitExtractor miso_bits(miso_data, AnalyzerEnums::MsbFirst, dwBitsPerTransfer);

	/* First ensure clock is high */
	if (mClock->GetCurrentBitState() == BIT_LOW)
	{
		/* Wrong beginning polarity, don't bother sending anything */
		return;
	}

	for (U32 i = 0; i < dwBitsPerTransfer; i++)
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

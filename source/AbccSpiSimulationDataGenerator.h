/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerDataGenerator.h
**    Summary: DLL-DataGenerator header
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_SIMULATION_DATA_GENERATOR_H
#define ABCC_SPI_SIMULATION_DATA_GENERATOR_H

#include <AnalyzerHelpers.h>

typedef struct tAbccMosiPacket
{
	U8	spiCtrl;
	U8	res1;
	U16	msgLen;
	U16	pdLen;
	U8	appStat;
	U8	intMask;
	U16	msgSize;
	U16	res2;
	U8	srcId;
	U8	obj;
	U16	inst;
	U8	cmd;
	U8	res3;
	U16	cmdExt;
	U8	msgData[4];
	U8	processData[4];
	U16	crc32_lo;
	U16	crc32_hi;
	U16	pad;
}tAbccMosiPacket;

typedef struct tAbccMisoPacket
{
	U16	res1;
	U16	ledStat;
	U8	anbStat;
	U8	spiStat;
	U16	netTime_lo;
	U16	netTime_hi;
	U16	msgSize;
	U16	res2;
	U8	srcId;
	U8	obj;
	U16	inst;
	U8	cmd;
	U8	res3;
	U16	cmdExt;
	U8	msgData[4];
	U8	processData[4];
	U16	crc32_lo;
	U16	crc32_hi;
}tAbccMisoPacket;

typedef union uAbccPacket
{
	tAbccMisoPacket miso;
	tAbccMosiPacket mosi;
	U8 raw[28];
}uAbccPacket;

class SpiAnalyzerSettings;

class SpiSimulationDataGenerator
{
public:
	SpiSimulationDataGenerator();
	~SpiSimulationDataGenerator();

	void Initialize(U32 simulation_sample_rate, SpiAnalyzerSettings* settings);
	U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels);

protected:
	SpiAnalyzerSettings* mSettings;
	tAbccMosiPacket mMosiData;
	tAbccMisoPacket mMisoData;
	U32 mSimulationSampleRateHz;
	U64 mValue;
	U32 mNetTime;
	U8  mMsgCmdRespState;

protected: /* SPI specific */
	ClockGenerator mClockGenerator;

	void CreateSpiTransaction();
	void SendPacketData(bool is_clock_idle_high, U32 length);
	void OutputByte_CPOL0_CPHA0(U64 mosi_data, U64 miso_data);
	void OutputByte_CPOL1_CPHA1(U64 mosi_data, U64 miso_data);


	SimulationChannelDescriptorGroup mSpiSimulationChannels;
	SimulationChannelDescriptor* mMiso;
	SimulationChannelDescriptor* mMosi;
	SimulationChannelDescriptor* mClock;
	SimulationChannelDescriptor* mEnable;
};
#endif /* ABCC_SPI_SIMULATION_DATA_GENERATOR_H */

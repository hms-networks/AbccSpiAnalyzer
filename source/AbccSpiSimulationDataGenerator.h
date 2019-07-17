/******************************************************************************
**  Copyright (C) 2015-2019 HMS Industrial Networks Inc, all rights reserved
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
#include "abcc_td.h"
#include "abcc_abp/abp.h"

#define NUM_BYTES_2_WORDS(x)				( ( (x) + 1 ) >> 1 )

#define ABCC_CFG_MAX_MSG_SIZE				( 1524 )
#define ABCC_CFG_SPI_MSG_FRAG_LEN			( 16 )
#define ABCC_CFG_MAX_PROCESS_DATA_SIZE		( 4 )

#define CRC_WORD_LEN_IN_WORDS				( 2 )
#define SPI_FRAME_SIZE_EXCLUDING_DATA		( 7 )

#if ABCC_CFG_SPI_MSG_FRAG_LEN > ABCC_CFG_MAX_MSG_SIZE
#error  spi fragmentation length cannot exceed max msg size
#endif

#define MAX_PAYLOAD_WORD_LEN ( ( NUM_BYTES_2_WORDS( ABCC_CFG_SPI_MSG_FRAG_LEN ) ) + \
							   ( NUM_BYTES_2_WORDS( ABCC_CFG_MAX_PROCESS_DATA_SIZE ) ) + \
							   ( CRC_WORD_LEN_IN_WORDS ) )

class SpiAnalyzerSettings;

class SpiSimulationDataGenerator
{
public:

	SpiSimulationDataGenerator();
	~SpiSimulationDataGenerator();

	void Initialize(U32 simulation_sample_rate, SpiAnalyzerSettings* settings);
	U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels);

protected: /* Enums, Types, and Classes */

	enum class MessageType : U8
	{
		Command,
		Response
	};

	enum class MessageResponseType : U8
	{
		Normal,
		Error
	};

	enum class ClockIdleLevel : U8
	{
		Low,
		High
	};

	typedef struct AbccMosiPacket
	{
		U8	spiCtrl;
		U8	res1;
		U16	msgLen;
		U16	pdLen;
		U8	appStat;
		U8	intMask;
		U8	msgData[ABCC_CFG_SPI_MSG_FRAG_LEN];
		U8	processData[ABCC_CFG_MAX_PROCESS_DATA_SIZE];
		U16	crc32_lo;
		U16	crc32_hi;
		U16	pad;
	} AbccMosiPacket_t;

	typedef struct AbccMisoPacket
	{
		U16	res1;
		U16	ledStat;
		U8	anbStat;
		U8	spiStat;
		U16	netTime_lo;
		U16	netTime_hi;
		U8	msgData[ABCC_CFG_SPI_MSG_FRAG_LEN];
		U8	processData[ABCC_CFG_MAX_PROCESS_DATA_SIZE];
		U16	crc32_lo;
		U16	crc32_hi;
	} AbccMisoPacket_t;

protected: /* Members */

	SpiAnalyzerSettings * mSettings;
	U32 mSimulationSampleRateHz;
	U64 mValue;

	ClockGenerator mClockGenerator;
	SimulationChannelDescriptorGroup mSpiSimulationChannels;
	SimulationChannelDescriptor* mMiso;
	SimulationChannelDescriptor* mMosi;
	SimulationChannelDescriptor* mClock;
	SimulationChannelDescriptor* mEnable;

	double mTargetClockFrequencyHz;

	/* SPI (fragmentation) packet variables */
	AbccMisoPacket_t mMisoPacket;
	AbccMosiPacket_t mMosiPacket;
	U32 mPacketOffset;
	U32 mPacketMsgFieldSize;
	bool mAbortTransfer;

	/* Full (unfragmented) message buffers */
	ABP_MsgType mMisoMsgData;
	ABP_MsgType mMosiMsgData;

	U32 mNetTime;
	U8  mSourceId;
	U8  mToggleBit;
	U16 mMsgCmdRespState;
	S32 mMosiProcessData;
	S32 mMisoProcessData;

protected: /* Methods */

	inline void SetMosiObjectSpecificError(U8 error_code);

	void RunFileTransferStateMachine(MessageResponseType msg_response_type);
	void UpdateFileTransferStateMachine();

	void CreateFileInstance(ABP_MsgType* msg_ptr, MessageType message_type);
	void FileOpen(ABP_MsgType* msg_ptr, MessageType message_type, CHAR* file_name, UINT8 file_name_length);
	void GetFileSize(ABP_MsgType* msg_ptr, MessageType message_type);
	void FileRead(ABP_MsgType* msg_ptr, MessageType message_type, CHAR* file_data, UINT32 file_data_length);
	void FileClose(ABP_MsgType* msg_ptr, MessageType message_type, UINT32 file_size);
	void DeleteFileInstance(ABP_MsgType* msg_ptr, MessageType message_type);

	void UpdateProcessData();
	void CreateSpiTransaction();
	void SendPacketData(ClockIdleLevel clock_idle_level, U32 length);
	void OutputByte_CPOL0_CPHA0(U64 mosi_data, U64 miso_data);
	void OutputByte_CPOL1_CPHA1(U64 mosi_data, U64 miso_data);
};
#endif /* ABCC_SPI_SIMULATION_DATA_GENERATOR_H */

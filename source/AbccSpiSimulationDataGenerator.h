/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerDataGenerator.h
**    Summary: Handles simulation of ABCC SPI communication.
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_SIMULATION_DATA_GENERATOR_H
#define ABCC_SPI_SIMULATION_DATA_GENERATOR_H

#include <AnalyzerHelpers.h>
#include "abcc_td.h"
#include "abcc_abp/abp.h"
#include "AbccLogFileParser.h"

#ifdef _MSC_VER
	#include <stdlib.h>
	#define bswap_16(x) _byteswap_ushort(x)
#elif defined(__APPLE__)
	#include <libkern/OSByteOrder.h>
	#define bswap_16(x) OSSwapInt16(x)
#else
	#include <byteswap.h>
#endif

#define ABCC_CFG_MAX_MSG_SIZE				( 1524 )
#define ABCC_CFG_MAX_PROCESS_DATA_SIZE		( 4 )

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

	enum class ClockIdleMode : U8
	{
		Low,
		High,
		Auto
	};

	typedef struct AbccMosiPacket
	{
		U8	spiCtrl;
		U8	res1;
		U16	msgLen;
		U16	pdLen;
		U8	appStat;
		U8	intMask;
		U8	msgData[ABCC_CFG_MAX_MSG_SIZE];
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
		U8	msgData[ABCC_CFG_MAX_MSG_SIZE];
		U8	processData[ABCC_CFG_MAX_PROCESS_DATA_SIZE];
		U16	crc32_lo;
		U16	crc32_hi;
	} AbccMisoPacket_t;

protected: /* Members */

	ClockGenerator mClockGenerator;
	SimulationChannelDescriptorGroup mSpiSimulationChannels;
	SimulationChannelDescriptor* mMiso;
	SimulationChannelDescriptor* mMosi;
	SimulationChannelDescriptor* mClock;
	SimulationChannelDescriptor* mEnable;

	SpiAnalyzerSettings* mSettings;
	AbccLogFileParser* mLogFileParser;

	/* Dummy value used as the payload for various random SPI events. */
	U64 mIncrementingValue;

	bool m3WireMode;
	bool mLogFileSimulation;
	bool mAbortTransfer;
	MessageReturnType mLogFileMessageType;
	ClockIdleMode mClockIdleMode;
	ClockIdleMode mNextClockIdleMode;
	U32 mNetTime;
	U8  mSourceId;
	U8  mToggleBit;
	U16 mMsgCmdRespState;

	/* Simulation timing variables */
	U32 mSimulationSampleRateHz;
	double mTargetClockFrequencyHz;
	double mInterPacketGapTime;
	double mInterByteGapTime;
	double mChipSelectDelay;

	/* SPI fragmentation state variables */
	bool mDynamicMsgFragmentationLength;
	U16 mDefaultMsgFragmentationLength;
	U16 mMsgFragmentationLength;
	U16 mMessageFieldOffset;
	U16 mTotalMsgBytesToSend;

	/* Counter is conveyed in process data during log file simulation. */
	U32 mMessageCount;

	/* SPI (fragmentation) packet buffers */
	AbccMisoPacket_t mMisoPacket;
	AbccMosiPacket_t mMosiPacket;

	/* Full (unfragmented) message buffers */
	ABP_MsgType mMisoMsgData;
	ABP_MsgType mMosiMsgData;

	/* Pointers to the actual position of the process data in the SPI packets.
	** Actual location depends on the currently set message data length field. */
	U8* mMosiProcessDataPtr;
	U8* mMisoProcessDataPtr;

	/* Pointers to the actual position of the CRC in the SPI packets.
	** Actual location depends on the currently set message data length field. */
	U8* mMosiCrc32Ptr;
	U8* mMisoCrc32Ptr;

	/* Length used for CRC32 computation */
	U16 mMosiCrcPacketLength;
	U16 mMisoCrcPacketLength;

	/* Represents the total number of bytes to send per SPI packet */
	U16 mNumBytesInSpiPacket;

protected: /* Methods */

	void InitializeSpiClockIdleMode();
	void InitializeSpiTimingCharacteristics();
	void InitializeSpiChannels();

	inline void SetMosiObjectSpecificError(U8 error_code);

	void RunFileTransferStateMachine(MessageResponseType msg_response_type);
	void UpdateFileTransferStateMachine();

	void CreateFileInstance(ABP_MsgType* msg_ptr, MessageType message_type);
	void FileOpen(ABP_MsgType* msg_ptr, MessageType message_type, const CHAR* file_name, UINT8 file_name_length);
	void GetFileSize(ABP_MsgType* msg_ptr, MessageType message_type);
	void FileRead(ABP_MsgType* msg_ptr, MessageType message_type, const CHAR* file_data, UINT32 file_data_length);
	void FileClose(ABP_MsgType* msg_ptr, MessageType message_type, UINT32 file_size);
	void DeleteFileInstance(ABP_MsgType* msg_ptr, MessageType message_type);

	U16 CalculateNewMessageFragmentation();
	void UpdatePacketDynamicFormat(U16 message_data_field_length, U16 process_data_field_length);
	void UpdateProcessData();
	bool UpdateMessageData(U8* mosi_msg_data_source, U8* miso_msg_data_source);
	void UpdateCrc32(bool generate_mosi_crc_error, bool generate_miso_crc_error);
	bool CreateSpiTransaction();
	void SendPacketData(ClockIdleMode clock_idle_level, U32 length);
	void OutputByte_CPOL0_CPHA0(U64 mosi_data, U64 miso_data, bool word_mode = false);
	void OutputByte_CPOL1_CPHA1(U64 mosi_data, U64 miso_data, bool word_mode = false);
};
#endif /* ABCC_SPI_SIMULATION_DATA_GENERATOR_H */

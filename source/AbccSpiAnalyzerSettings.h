/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerSettings.h
**    Summary: DLL-Settings header
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_ANALYZER_SETTINGS_H
#define ABCC_SPI_ANALYZER_SETTINGS_H

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>
#include "rapidxml-1.13/rapidxml.hpp"

#include "AbccSpiAnalyzerTypes.h"
#include "abcc_td.h"
#include "abcc_abp/abp.h"

enum class MessageIndexing : U32
{
	Disabled,
	Compact,
	Detailed,
	SizeOfEnum
};

enum class TimestampIndexing : U32
{
	Disabled,
	AllPackets,
	WriteProcessDataValid,
	NewReadProcessData,
	SizeOfEnum
};

enum class ExportType : U32
{
	Frames,
	ProcessData,
	MessageData,
	SizeOfEnum
};

namespace NetworkTypeIndex
{
	typedef enum : U16
	{
		Unspecified,
		//PDPV0,			/* PROFIBUS DP-V0 */
		PDPV1,				/* PROFIBUS DP-V1 */
		COP,				/* CANopen */
		DEV,				/* DeviceNet */
		//RTU,				/* Modbus-RTU */
		//CNT,				/* ControlNet */
		//ETN_1P,			/* Modbus-TCP */
		//PRT,				/* PROFINET RT */
		//EIP_1P,			/* EtherNet/IP */
		ECT,				/* EtherCAT */
		PIR,				/* PROFINET IRT */
		CCL,				/* CC-Link */
		ETN_2P,				/* Modbus-TCP 2-Port */
		//CPN,				/* CompoNet */
		//PRT_2P,			/* PROFINET RT 2-port */
		//SRC3,				/* SERCOS III */
		//BMP,				/* BACnet MS/TP */
		BIP,				/* BACnet/IP */
		EIP_2P_BB,			/* EtherNet/IP 2-Port BB DLR */
		//EIP_2P,			/* EtherNet/IP 2-Port */
		PIR_FO,				/* PROFINET IRT FO */
		EPL,				/* POWERLINK */
		CFN,				/* CC-Link IE Field Network */
		CET,				/* Common Ethernet */
		EIP_2P_BB_IIOT,		/* EtherNet/IP IIoT */
		PIR_IIOT,			/* PROFINET IRT IIoT */
		PIR_FO_IIOT,		/* PROFINET IRT FO IIoT */
		CET_IIOT			/* Common Ethernet IIoT */
	} Enum;
};

class SpiAnalyzerSettings : public AnalyzerSettings
{
public:

	SpiAnalyzerSettings();
	virtual ~SpiAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	virtual void LoadSettings(const char* settings);
	virtual const char* SaveSettings();

	void UpdateInterfacesFromSettings();

public: /* Members */

	Channel mMosiChannel;
	Channel mMisoChannel;
	Channel mClockChannel;
	Channel mEnableChannel;

	U32 mNetworkType;
	U8 mChangeID;

	DisplayPriority mMsgDataPriority;
	DisplayPriority mProcessDataPriority;

	MessageIndexing mMessageIndexingVerbosityLevel;
	TimestampIndexing mTimestampIndexing;

	bool mMessageSrcIdIndexing;
	bool mErrorIndexing;
	bool mAnybusStatusIndexing;
	bool mApplStatusIndexing;

	const char* mAdvSettingsPath;
	bool m3WireOn4Channels;
	bool m4WireOn3Channels;
	std::string mExportDelimiter;
	S32 mClockingAlertLimit;
	bool mExpandBitFrames;

	std::string mSimulateLogFilePath;
	U32 mSimulateLogFileDefaultState;
	S32 mSimulateClockIdleHigh;
	S32 mSimulateClockFrequency;
	S32 mSimulatePacketGapNs;
	S32 mSimulateByteGapNs;
	S32 mSimulateChipSelectNs;
	S32 mSimulateMsgDataLength;
	bool mSimulateWordMode;

protected: /* Members */

	std::unique_ptr< AnalyzerSettingInterfaceChannel >		mMosiChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceChannel >		mMisoChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceChannel >		mClockChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceChannel >		mEnableChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mNetworkTypeInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mMessageIndexingVerbosityLevelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mMsgDataPriorityInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mProcessDataPriorityInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mIndexTimestampsInterface;
	std::unique_ptr< AnalyzerSettingInterfaceBool >			mIndexMessageSrcIdInterface;
	std::unique_ptr< AnalyzerSettingInterfaceBool >			mIndexErrorsInterface;
	std::unique_ptr< AnalyzerSettingInterfaceBool >			mIndexAnybusStatusInterface;
	std::unique_ptr< AnalyzerSettingInterfaceBool >			mIndexApplStatusInterface;

protected: /* Methods */

	std::unique_ptr< AnalyzerSettingInterfaceText >			mAdvancedSettingsInterface;
	bool ParseAdvancedSettingsFile();
	void ParseSimulationSettings(rapidxml::xml_node<>* simulation_node);
	void SetDefaultAdvancedSettings();

	void SetSettingError( const std::string& setting_name, const std::string& error_text );
	U8 SaveSettingChangeID();
};

#endif /* ABCC_SPI_ANALYZER_SETTINGS_H */

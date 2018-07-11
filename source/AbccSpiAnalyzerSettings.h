/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
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

#include "AbccSpiAnalyzerTypes.h"

#define ENABLE_ADVANCED_SETTINGS 1

enum class MessageIndexing : U32
{
	Disabled,
	Compact,
	Detailed,
	SizeOfEnum
};

enum class SourceIdIndexing : U32 //TODO joca: use this or remove it
{
	Disabled,
	Enabled,
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

typedef enum NetworkType
{
	e_NW_TYPE_UNSPECIFIED,
	e_NW_TYPE_PDPV0,			/* PROFIBUS DP-V0 */
	e_NW_TYPE_PDPV1,			/* PROFIBUS DP-V1 */
	e_NW_TYPE_COP,				/* CANopen */
	e_NW_TYPE_DEV,				/* DeviceNet */
	e_NW_TYPE_RTU,				/* Modbus-RTU */
	e_NW_TYPE_CNT,				/* ControlNet */
	e_NW_TYPE_ETN_1P,			/* Modbus-TCP */
	e_NW_TYPE_PRT,				/* PROFINET RT */
	e_NW_TYPE_EIP_1P,			/* EtherNet/IP */
	e_NW_TYPE_ECT,				/* EtherCAT */
	e_NW_TYPE_PIR,				/* PROFINET IRT */
	e_NW_TYPE_CCL,				/* CC-Link */
	e_NW_TYPE_ETN_2P,			/* Modbus-TCP 2-Port */
	e_NW_TYPE_CPN,				/* CompoNet */
	e_NW_TYPE_PRT_2P,			/* PROFINET RT 2-port */
	e_NW_TYPE_SRC3,				/* SERCOS III */
	e_NW_TYPE_BMP,				/* BACnet MS/TP */
	e_NW_TYPE_BIP,				/* BACnet/IP */
	e_NW_TYPE_EIP_2P_BB,		/* EtherNet/IP 2-Port BB DLR */
	e_NW_TYPE_EIP_2P,			/* EtherNet/IP 2-Port */
	e_NW_TYPE_PIR_FO,			/* PROFINET IRT FO */
	e_NW_TYPE_EPL,				/* POWERLINK */
	e_NW_TYPE_CFN,				/* CC-Link IE Field Network */
	e_NW_TYPE_CET,				/* Common Ethernet */
	e_NW_TYPE_EIP_2P_BB_IIOT,	/* EtherNet/IP IIoT */
	e_NW_TYPE_PIR_IIOT,			/* PROFINET IRT IIoT */
	e_NW_TYPE_PIR_FO_IIOT		/* PROFINET IRT FO IIoT */
} NetworkType_t;

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

#if ENABLE_ADVANCED_SETTINGS
	const char* mAdvSettingsPath;
	bool m3WireOn4Channels;
	bool m4WireOn3Channels;
#endif

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

#if ENABLE_ADVANCED_SETTINGS
	std::unique_ptr< AnalyzerSettingInterfaceText >			mAdvancedSettingsInterface;
	bool ParseAdvancedSettingsFile(void);
#endif

	U8 SaveSettingChangeID();
};

#endif /* ABCC_SPI_ANALYZER_SETTINGS_H */

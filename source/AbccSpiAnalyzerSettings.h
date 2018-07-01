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

#define ENABLE_ADVANCED_SETTINGS 1

typedef enum tExportType {
	e_EXPORT_FRAMES,
	e_EXPORT_PROCESS_DATA,
	e_EXPORT_MESSAGE_DATA,
} tExportType;

typedef enum tNetworkType {
	e_NW_TYPE_UNSPECIFIED,
	e_NW_TYPE_PDPV0,		  /* PROFIBUS DP-V0 */
	e_NW_TYPE_PDPV1,		  /* PROFIBUS DP-V1 */
	e_NW_TYPE_COP,			  /* CANopen */
	e_NW_TYPE_DEV,			  /* DeviceNet */
	e_NW_TYPE_RTU,			  /* Modbus-RTU */
	e_NW_TYPE_CNT,			  /* ControlNet */
	e_NW_TYPE_ETN_1P,		  /* Modbus-TCP */
	e_NW_TYPE_PRT,			  /* PROFINET RT */
	e_NW_TYPE_EIP_1P,		  /* EtherNet/IP */
	e_NW_TYPE_ECT,			  /* EtherCAT */
	e_NW_TYPE_PIR,			  /* PROFINET IRT */
	e_NW_TYPE_CCL,			  /* CC-Link */
	e_NW_TYPE_ETN_2P,		  /* Modbus-TCP 2-Port */
	e_NW_TYPE_CPN,			  /* CompoNet */
	e_NW_TYPE_PRT_2P,		  /* PROFINET RT 2-port */
	e_NW_TYPE_SRC3,			  /* SERCOS III */
	e_NW_TYPE_BMP,			  /* BACnet MS/TP */
	e_NW_TYPE_BIP,			  /* BACnet/IP */
	e_NW_TYPE_EIP_2P_BB,	  /* EtherNet/IP 2-Port BB DLR */
	e_NW_TYPE_EIP_2P,		  /* EtherNet/IP 2-Port */
	e_NW_TYPE_PIR_FO,		  /* PROFINET IRT FO */
	e_NW_TYPE_EPL,			  /* POWERLINK */
	e_NW_TYPE_CFN,			  /* CC-Link IE Field Network */
	e_NW_TYPE_CET,			  /* Common Ethernet */
	e_NW_TYPE_EIP_2P_BB_IIOT, /* EtherNet/IP IIoT */
	e_NW_TYPE_PIR_IIOT,		  /* PROFINET IRT IIoT */
	e_NW_TYPE_PIR_FO_IIOT	  /* PROFINET IRT FO IIoT */
} tNetworkType;

typedef enum tDecodeVerbosityLevel {
	e_VERBOSITY_LEVEL_DISABLED,
	e_VERBOSITY_LEVEL_COMPACT,
	e_VERBOSITY_LEVEL_DETAILED
} tDecodeVerbosityLevel;

typedef enum tDecodeTimestampMode
{
	e_TIMESTAMP_DISABLED,
	e_TIMESTAMP_ALL_PACKETS,
	e_TIMESTAMP_WRITE_PROCESS_DATA_VALID,
	e_TIMESTAMP_NEW_READ_PROCESS_DATA,
}tDecodeTimestampMode;

typedef enum tMsgDataPriority
{
	e_MSG_DATA_PRIORITIZE_DATA,
	e_MSG_DATA_PRIORITIZE_TAG
}tMsgDataPriority;

typedef enum tProcessDataPriority
{
	e_PROCESS_DATA_PRIORITIZE_DATA,
	e_PROCESS_DATA_PRIORITIZE_TAG
}tProcessDataPriority;

class SpiAnalyzerSettings : public AnalyzerSettings
{
public:
	SpiAnalyzerSettings();
	virtual ~SpiAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	virtual void LoadSettings(const char* settings);
	virtual const char* SaveSettings();

	void UpdateInterfacesFromSettings();

	U8 SaveSettingChangeID();
	U8 mChangeID;

	Channel mMosiChannel;
	Channel mMisoChannel;
	Channel mClockChannel;
	Channel mEnableChannel;
	U32 mNetworkType;
	U32 mMessageIndexingVerbosityLevel;
	U32 mMsgDataPriority;
	U32 mProcessDataPriority;
	bool mMessageSrcIdIndexing;
	bool mErrorIndexing;
	U32 mTimestampIndexing;
	bool mAnybusStatusIndexing;
	bool mApplStatusIndexing;
#if ENABLE_ADVANCED_SETTINGS
	const char* mAdvSettingsPath;
	bool m3WireOn4Channels;
	bool m4WireOn3Channels;
#endif


protected:
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
#if ENABLE_ADVANCED_SETTINGS
	std::unique_ptr< AnalyzerSettingInterfaceText >			mAdvancedSettingsInterface;
	bool ParseAdavancedSettingsFile( void );
#endif
};

#endif /* ABCC_SPI_ANALYZER_SETTINGS_H */

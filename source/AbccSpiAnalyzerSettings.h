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

typedef enum tDecodeVerbosityLevel
{
	e_VERBOSITY_LEVEL_DISABLED,
	e_VERBOSITY_LEVEL_COMPACT,
	e_VERBOSITY_LEVEL_DETAILED
}tDecodeVerbosityLevel;

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
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMosiChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMisoChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mClockChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mEnableChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mMessageIndexingVerbosityLevelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mMsgDataPriorityInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mProcessDataPriorityInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mIndexTimestampsInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > 		mIndexMessageSrcIdInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > 		mIndexErrorsInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > 		mIndexAnybusStatusInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > 		mIndexApplStatusInterface;
#if ENABLE_ADVANCED_SETTINGS
	std::auto_ptr< AnalyzerSettingInterfaceText > 		mAdvancedSettingsInterface;
	bool ParseAdavancedSettingsFile( void );
#endif
};

#endif /* ABCC_SPI_ANALYZER_SETTINGS_H */

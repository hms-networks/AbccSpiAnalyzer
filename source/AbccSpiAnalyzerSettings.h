/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
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

typedef enum
{
	e_VERBOSITY_LEVEL_COMPACT,
	e_VERBOSITY_LEVEL_DETAILED
}t_DecodeVerbosityLevel;

class SpiAnalyzerSettings : public AnalyzerSettings
{
public:
	SpiAnalyzerSettings();
	virtual ~SpiAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	virtual void LoadSettings(const char* settings);
	virtual const char* SaveSettings();

	void UpdateInterfacesFromSettings();

	Channel mMosiChannel;
	Channel mMisoChannel;
	Channel mClockChannel;
	Channel mEnableChannel;
	AnalyzerEnums::ShiftOrder mShiftOrder;
	U32 mBitsPerTransfer;
	BitState mClockInactiveState;
	AnalyzerEnums::Edge mDataValidEdge;
	BitState mEnableActiveState;
	U32 mVerbosityLevel;
	bool mMessageIndexing;
	bool mMessageSrcIdIndexing;
	bool mErrorIndexing;
	bool mTimestampIndexing;
	bool mAnybusStatusIndexing;
	bool mApplStatusIndexing;


protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMosiChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMisoChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mClockChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mEnableChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mVerbosityLevelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mIndexMessagesInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mIndexMessageSrcIdInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mIndexErrorsInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mIndexTimestampsInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mIndexAnybusStatusInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mIndexApplStatusInterface;
};

#endif /* ABCC_SPI_ANALYZER_SETTINGS_H */

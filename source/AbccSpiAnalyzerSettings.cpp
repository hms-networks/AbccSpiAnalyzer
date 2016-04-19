/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerSettings.cpp
**    Summary: DLL-Settings source
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#include "AbccSpiAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

SpiAnalyzerSettings::SpiAnalyzerSettings()
	: mMosiChannel(UNDEFINED_CHANNEL),
	mMisoChannel(UNDEFINED_CHANNEL),
	mClockChannel(UNDEFINED_CHANNEL),
	mEnableChannel(UNDEFINED_CHANNEL),
	mShiftOrder(AnalyzerEnums::MsbFirst),
	mBitsPerTransfer(8),
	mClockInactiveState(BIT_HIGH),
	mDataValidEdge(AnalyzerEnums::TrailingEdge),
	mEnableActiveState(BIT_LOW),
	mVerbosityLevel(e_VERBOSITY_LEVEL_COMPACT),
	mMessageIndexing(true),
	mMessageSrcIdIndexing(false),
	mErrorIndexing(true),
	mTimestampIndexing(false),
	mAnybusStatusIndexing(false),
	mApplStatusIndexing(false)

{
	mMosiChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mMosiChannelInterface->SetTitleAndTooltip("SPI MOSI Channel :", "Master Out, Slave In (Host to Module)");
	mMosiChannelInterface->SetChannel(mMosiChannel);

	mMisoChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mMisoChannelInterface->SetTitleAndTooltip("SPI MISO Channel :", "Master In, Slave Out (Module to Host)");
	mMisoChannelInterface->SetChannel(mMisoChannel);

	mClockChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mClockChannelInterface->SetTitleAndTooltip("SPI Clock Channel :", "Clock (CLK)");
	mClockChannelInterface->SetChannel(mClockChannel);

	mEnableChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mEnableChannelInterface->SetTitleAndTooltip("SPI Enable Channel :", "Enable (SS, Slave Select)");
	mEnableChannelInterface->SetChannel(mEnableChannel);
	mEnableChannelInterface->SetSelectionOfNoneIsAllowed(true);

	mVerbosityLevelInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mVerbosityLevelInterface->SetTitleAndTooltip("Decoded Verbosity Level :", "Specifies how detailed the decoded protcols entries are.");
	mVerbosityLevelInterface->AddNumber(e_VERBOSITY_LEVEL_COMPACT,  "Compact Results", "Message header information is added to a single tabular result.");
	mVerbosityLevelInterface->AddNumber(e_VERBOSITY_LEVEL_DETAILED, "Verbose Results", "Message header information is added to tabular results individually.");
	mVerbosityLevelInterface->SetNumber(mVerbosityLevel);


	mIndexMessagesInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexMessagesInterface->SetTitleAndTooltip("ABCC Message Indexing :", "Enable indexed searching of ABCC messages.");
	mIndexMessagesInterface->SetValue(mMessageIndexing);

	mIndexMessageSrcIdInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexMessageSrcIdInterface->SetTitleAndTooltip("ABCC Message 'Source ID' Indexing :", "Enable indexed searching of the source ID associated with an ABCC message.");
	mIndexMessageSrcIdInterface->SetValue(mMessageSrcIdIndexing);

	mIndexErrorsInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexErrorsInterface->SetTitleAndTooltip("Error Indexing :", "Enable indexed searching of errors.");
	mIndexErrorsInterface->SetValue(mErrorIndexing);

	mIndexTimestampsInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexTimestampsInterface->SetTitleAndTooltip("Timestamp Indexing :", "Enable indexed searching of timestamps.");
	mIndexTimestampsInterface->SetValue(mTimestampIndexing);

	mIndexAnybusStatusInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexAnybusStatusInterface->SetTitleAndTooltip("Anybus Status Indexing :", "Enable indexed searching of Anybus status.");
	mIndexAnybusStatusInterface->SetValue(mAnybusStatusIndexing);

	mIndexApplStatusInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexApplStatusInterface->SetTitleAndTooltip("Application Status Indexing :", "Enable indexed searching of application status.");
	mIndexApplStatusInterface->SetValue(mApplStatusIndexing);

	AddInterface(mMosiChannelInterface.get());
	AddInterface(mMisoChannelInterface.get());
	AddInterface(mClockChannelInterface.get());
	AddInterface(mEnableChannelInterface.get());

	AddInterface(mVerbosityLevelInterface.get());
	AddInterface(mIndexMessagesInterface.get());
	AddInterface(mIndexMessageSrcIdInterface.get());
	AddInterface(mIndexErrorsInterface.get());
	AddInterface(mIndexTimestampsInterface.get());
	AddInterface(mIndexAnybusStatusInterface.get());
	AddInterface(mIndexApplStatusInterface.get());

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
	AddExportOption(0, "Export as text/csv file");
	AddExportExtension(0, "text", "txt");
	AddExportExtension(0, "csv", "csv");

	ClearChannels();
	AddChannel(mMosiChannel, "MOSI", false);
	AddChannel(mMisoChannel, "MISO", false);
	AddChannel(mClockChannel, "CLOCK", false);
	AddChannel(mEnableChannel, "ENABLE", false);
}

SpiAnalyzerSettings::~SpiAnalyzerSettings()
{
}

bool SpiAnalyzerSettings::SetSettingsFromInterfaces()
{
	Channel mosi = mMosiChannelInterface->GetChannel();
	Channel miso = mMisoChannelInterface->GetChannel();
	Channel clock = mClockChannelInterface->GetChannel();
	Channel enable = mEnableChannelInterface->GetChannel();

	std::vector<Channel> channels;
	channels.push_back(mosi);
	channels.push_back(miso);
	channels.push_back(clock);
	channels.push_back(enable);

	if (AnalyzerHelpers::DoChannelsOverlap(&channels[0], (U32)channels.size()) == true)
	{
		SetErrorText("Please select different channels for each input.");
		return false;
	}

	if ((mosi == UNDEFINED_CHANNEL) && (miso == UNDEFINED_CHANNEL))
	{
		SetErrorText("Please select at least one input for either MISO or MOSI.");
		return false;
	}

	mMosiChannel = mMosiChannelInterface->GetChannel();
	mMisoChannel = mMisoChannelInterface->GetChannel();
	mClockChannel = mClockChannelInterface->GetChannel();
	mEnableChannel = mEnableChannelInterface->GetChannel();

	mVerbosityLevel = U32(mVerbosityLevelInterface->GetNumber());
	mMessageIndexing = bool(mIndexMessagesInterface->GetValue());
	mMessageSrcIdIndexing = bool(mIndexMessageSrcIdInterface->GetValue());
	mErrorIndexing = bool(mIndexErrorsInterface->GetValue());
	mTimestampIndexing = bool(mIndexTimestampsInterface->GetValue());
	mAnybusStatusIndexing = bool(mIndexAnybusStatusInterface->GetValue());
	mApplStatusIndexing = bool(mIndexApplStatusInterface->GetValue());

	ClearChannels();
	AddChannel(mMosiChannel, "MOSI", mMosiChannel != UNDEFINED_CHANNEL);
	AddChannel(mMisoChannel, "MISO", mMisoChannel != UNDEFINED_CHANNEL);
	AddChannel(mClockChannel, "CLOCK", mClockChannel != UNDEFINED_CHANNEL);
	AddChannel(mEnableChannel, "ENABLE", mEnableChannel != UNDEFINED_CHANNEL);

	return true;
}

void SpiAnalyzerSettings::LoadSettings(const char* settings)
{
	SimpleArchive text_archive;
	const char* name_string;

	text_archive.SetString(settings);

	/* the first thing in the archive is the name of the protocol analyzer that the data belongs to. */
	text_archive >> &name_string;
	if (strcmp(name_string, "AbccSpiAnalyzer") != 0)
	{
		AnalyzerHelpers::Assert("AbccSpiAnalyzer: Provided with a settings string that doesn't belong to us.");
	}

	text_archive >> mMosiChannel;
	text_archive >> mMisoChannel;
	text_archive >> mClockChannel;
	text_archive >> mEnableChannel;

	text_archive >> mVerbosityLevel;
	text_archive >> mMessageIndexing;
	text_archive >> mMessageSrcIdIndexing;
	text_archive >> mErrorIndexing;
	text_archive >> mTimestampIndexing;
	text_archive >> mAnybusStatusIndexing;
	text_archive >> mApplStatusIndexing;

	//bool success = text_archive >> mUsePackets;  //new paramater added -- do this for backwards compatibility
	//if( success == false )
	//	mUsePackets = false; //if the archive fails, set the default value

	ClearChannels();
	AddChannel(mMosiChannel, "MOSI", mMosiChannel != UNDEFINED_CHANNEL);
	AddChannel(mMisoChannel, "MISO", mMisoChannel != UNDEFINED_CHANNEL);
	AddChannel(mClockChannel, "CLOCK", mClockChannel != UNDEFINED_CHANNEL);
	AddChannel(mEnableChannel, "ENABLE", mEnableChannel != UNDEFINED_CHANNEL);

	UpdateInterfacesFromSettings();
}

const char* SpiAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << "AbccSpiAnalyzer";
	text_archive << mMosiChannel;
	text_archive << mMisoChannel;
	text_archive << mClockChannel;
	text_archive << mEnableChannel;

	text_archive << mVerbosityLevel;
	text_archive << mMessageIndexing;
	text_archive << mMessageSrcIdIndexing;
	text_archive << mErrorIndexing;
	text_archive << mTimestampIndexing;
	text_archive << mAnybusStatusIndexing;
	text_archive << mApplStatusIndexing;

	return SetReturnString(text_archive.GetString());
}

void SpiAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mMosiChannelInterface->SetChannel(mMosiChannel);
	mMisoChannelInterface->SetChannel(mMisoChannel);
	mClockChannelInterface->SetChannel(mClockChannel);
	mEnableChannelInterface->SetChannel(mEnableChannel);

	mVerbosityLevelInterface->SetNumber(mVerbosityLevel);
	mIndexMessagesInterface->SetValue(mMessageIndexing);
	mIndexMessageSrcIdInterface->SetValue(mMessageSrcIdIndexing);
	mIndexErrorsInterface->SetValue(mErrorIndexing);
	mIndexTimestampsInterface->SetValue(mTimestampIndexing);
	mIndexAnybusStatusInterface->SetValue(mAnybusStatusIndexing);
	mIndexApplStatusInterface->SetValue(mApplStatusIndexing);
}

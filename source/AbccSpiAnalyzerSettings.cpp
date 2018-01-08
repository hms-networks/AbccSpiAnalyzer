/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
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

#include <iostream>
#include <fstream>
#include <vector>
#include "rapidxml-1.13/rapidxml.hpp"

/* Anytime behavior or definition of settings change, increment this counter. */
#define SETTINGS_REVISION_STRING "REVISION_00000002"

/* Default setting states */
static const U32  d_MessageIndexingVerbosityLevel = e_VERBOSITY_LEVEL_DETAILED;
static const U32  d_MsgDataPriority               = e_MSG_DATA_PRIORITIZE_TAG;
static const U32  d_ProcessDataPriority           = e_PROCESS_DATA_PRIORITIZE_TAG;
static const U32  d_TimestampIndexing             = e_TIMESTAMP_DISABLED;
static const bool d_MessageSrcIdIndexing          = true;
static const bool d_ErrorIndexing                 = true;
static const bool d_AnybusStatusIndexing          = true;
static const bool d_ApplStatusIndexing            = true;
static const char* d_AdvSettingsPath              = "";

SpiAnalyzerSettings::SpiAnalyzerSettings()
	: mMosiChannel(UNDEFINED_CHANNEL),
	mMisoChannel(UNDEFINED_CHANNEL),
	mClockChannel(UNDEFINED_CHANNEL),
	mEnableChannel(UNDEFINED_CHANNEL),
	mMessageIndexingVerbosityLevel(d_MessageIndexingVerbosityLevel),
	mMsgDataPriority(d_MsgDataPriority),
	mProcessDataPriority(d_ProcessDataPriority),
	mMessageSrcIdIndexing(d_MessageSrcIdIndexing),
	mErrorIndexing(d_ErrorIndexing),
	mTimestampIndexing(d_TimestampIndexing),
	mAnybusStatusIndexing(d_AnybusStatusIndexing),
	mApplStatusIndexing(d_ApplStatusIndexing),
#if ENABLE_ADVANCED_SETTINGS
	mAdvSettingsPath(d_AdvSettingsPath),
	m3WireOn4Channels(false),
	m4WireOn3Channels(false),
#endif
	mChangeID(0)
{
	mMosiChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mMosiChannelInterface->SetTitleAndTooltip("SPI Channel - MOSI :", "Master Out, Slave In (Host to Module)");
	mMosiChannelInterface->SetChannel(mMosiChannel);

	mMisoChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mMisoChannelInterface->SetTitleAndTooltip("SPI Channel - MISO :", "Master In, Slave Out (Module to Host)");
	mMisoChannelInterface->SetChannel(mMisoChannel);

	mClockChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mClockChannelInterface->SetTitleAndTooltip("SPI Channel - Clock :", "Clock (CLK)");
	mClockChannelInterface->SetChannel(mClockChannel);

	mEnableChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mEnableChannelInterface->SetTitleAndTooltip("SPI Channel - Enable :", "Enable (SS, Slave Select)");
	mEnableChannelInterface->SetChannel(mEnableChannel);
	mEnableChannelInterface->SetSelectionOfNoneIsAllowed(true);

	mIndexErrorsInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexErrorsInterface->SetTitleAndTooltip("Index - Errors :", "Enable indexed searching of errors.");
	mIndexErrorsInterface->SetValue(mErrorIndexing);

	mIndexAnybusStatusInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexAnybusStatusInterface->SetTitleAndTooltip("Index - Anybus Status :", "Enable indexed searching of Anybus status.\nEntries are added on status change events.\nFirst valid packet in capture will always trigger this event.");
	mIndexAnybusStatusInterface->SetValue(mAnybusStatusIndexing);

	mIndexApplStatusInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexApplStatusInterface->SetTitleAndTooltip("Index - Application Status :", "Enable indexed searching of application status.\nEntries are added on status change events.\nFirst valid packet in capture will always trigger this event.");
	mIndexApplStatusInterface->SetValue(mApplStatusIndexing);

	mIndexTimestampsInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mIndexTimestampsInterface->SetTitleAndTooltip("Index - Network Timestamp :", "Enable indexed searching of ABCC network timestamps.\nUseful for benchmarking; otherwise it is recommended to keep this option disabled.");
	mIndexTimestampsInterface->AddNumber(e_TIMESTAMP_DISABLED,  "Disabled", "No network timestamps will be added to tabular results.");
	mIndexTimestampsInterface->AddNumber(e_TIMESTAMP_ALL_PACKETS,  "All Packets", "The timestamp from every ABCC SPI packet will be added to tabular results.");
	mIndexTimestampsInterface->AddNumber(e_TIMESTAMP_WRITE_PROCESS_DATA_VALID, "Write Process Data Valid", "The timestamp from ABCC SPI packets containing \"valid\" write process data will be added to tabular results.");
	mIndexTimestampsInterface->AddNumber(e_TIMESTAMP_NEW_READ_PROCESS_DATA, "New Read Process Data", "The timestamp from ABCC SPI packets containing \"new\" read process data will be added to tabular results.");
	mIndexTimestampsInterface->SetNumber(mTimestampIndexing);

	mIndexMessageSrcIdInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexMessageSrcIdInterface->SetTitleAndTooltip("Index - Message 'Source ID' :", "Enable indexed searching of the source ID associated with an ABCC transaction.");
	mIndexMessageSrcIdInterface->SetValue(mMessageSrcIdIndexing);

	mMessageIndexingVerbosityLevelInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mMessageIndexingVerbosityLevelInterface->SetTitleAndTooltip("Index - Message :", "Specifies how detailed the decoded protcols entries are.");
	mMessageIndexingVerbosityLevelInterface->AddNumber(e_VERBOSITY_LEVEL_DISABLED,  "Disabled", "Messages will not be indexed in searchable results.\nUse when object messaging is of no interest.");
	mMessageIndexingVerbosityLevelInterface->AddNumber(e_VERBOSITY_LEVEL_COMPACT,  "Compact Results", "Message header information is added to a single tabular result.\nThis option is useful when looking for very specific messages.");
	mMessageIndexingVerbosityLevelInterface->AddNumber(e_VERBOSITY_LEVEL_DETAILED, "Verbose Results", "Message header information is added to tabular results individually.\nRecommended setting for general use.");
	mMessageIndexingVerbosityLevelInterface->SetNumber(mMessageIndexingVerbosityLevel);

	mMsgDataPriorityInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mMsgDataPriorityInterface->SetTitleAndTooltip("Message Data Priority :", "Specifies if the Message Data or Tag information is given priority in the display of multi-layered bubble-text.");
	mMsgDataPriorityInterface->AddNumber(e_MSG_DATA_PRIORITIZE_DATA, "Prioritize Data", "Message Data will be displayed as first layer of bubble text in analyzer results.");
	mMsgDataPriorityInterface->AddNumber(e_MSG_DATA_PRIORITIZE_TAG, "Prioritize Tag", "Message Data will be displayed as second layer of bubble text in analyzer results.");
	mMsgDataPriorityInterface->SetNumber(mMsgDataPriority);

	mProcessDataPriorityInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mProcessDataPriorityInterface->SetTitleAndTooltip("Process Data Priority :", "Specifies if the Process Data or Tag information is given priority in the display of multi-layered bubble-text.");
	mProcessDataPriorityInterface->AddNumber(e_MSG_DATA_PRIORITIZE_DATA, "Prioritize Data", "Process Data will be displayed as first layer of bubble text in analyzer results.");
	mProcessDataPriorityInterface->AddNumber(e_MSG_DATA_PRIORITIZE_TAG, "Prioritize Tag", "Process Data will be displayed as second layer of bubble text in analyzer results.");
	mProcessDataPriorityInterface->SetNumber(mProcessDataPriority);

#if ENABLE_ADVANCED_SETTINGS
	mAdvancedSettingsInterface.reset(new AnalyzerSettingInterfaceText());
	mAdvancedSettingsInterface->SetTextType(AnalyzerSettingInterfaceText::FilePath);
	mAdvancedSettingsInterface->SetTitleAndTooltip("Advanced Settings :", "Specifies external settings file to control special (advanced) settings of the plugin.\nPlease refer to plugin documentation for more details.\nIf left empty plugin defaults will be used which are suitable for most situations.");
	mAdvancedSettingsInterface->SetText(mAdvSettingsPath);
#endif

	AddInterface(mMosiChannelInterface.get());
	AddInterface(mMisoChannelInterface.get());
	AddInterface(mClockChannelInterface.get());
	AddInterface(mEnableChannelInterface.get());

	AddInterface(mIndexErrorsInterface.get());
	AddInterface(mIndexTimestampsInterface.get());
	AddInterface(mIndexAnybusStatusInterface.get());
	AddInterface(mIndexApplStatusInterface.get());
	AddInterface(mIndexMessageSrcIdInterface.get());
	AddInterface(mMessageIndexingVerbosityLevelInterface.get());
	AddInterface(mMsgDataPriorityInterface.get());
	AddInterface(mProcessDataPriorityInterface.get());
#if ENABLE_ADVANCED_SETTINGS
	AddInterface(mAdvancedSettingsInterface.get());
#endif

	AddExportOption(0, "Export All Frame Data");
	AddExportExtension(0, "All Frame Data", "csv");
	AddExportOption(1, "Export Message Data");
	AddExportExtension(1, "Message Data", "csv");
	AddExportOption(2, "Export Process Data");
	AddExportExtension(2, "Process Data", "csv");

	//AddExportOption(3, "Export as XML file");
	//AddExportExtension(3, "XML-File", "xml");

	ClearChannels();
	AddChannel(mMosiChannel, "MOSI", false);
	AddChannel(mMisoChannel, "MISO", false);
	AddChannel(mClockChannel, "CLOCK", false);
	AddChannel(mEnableChannel, "ENABLE", false);
}

SpiAnalyzerSettings::~SpiAnalyzerSettings()
{
}

bool SpiAnalyzerSettings::ParseAdavancedSettingsFile(void)
{
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> * root_node;
	std::string type;
	std::string value;

	/* Read the xml file into a vector */
	if( strlen(mAdvSettingsPath) > 0 )
	{
		std::ifstream filestream(mAdvSettingsPath);

		if(!filestream)
		{
			SetErrorText("Advanced settings: File not found or could not be opened.");
			return false;
		}
		else
		{
			std::vector<char> buffer((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());
			buffer.push_back('\0');

			/* Parse the buffer using the xml file parsing library into doc */
			doc.parse<0>(&buffer[0]);

			/* Jump to root node */
			root_node = doc.first_node("AdvancedSettings");

			if (root_node)
			{
				/* Iterate over each available setting */
				for (rapidxml::xml_node<> * settings_node = root_node->first_node("Setting"); settings_node; settings_node = settings_node->next_sibling())
				{
					rapidxml::xml_attribute<char>* ptr = settings_node->first_attribute("type");

					/* Get the type of setting */
					if (ptr != nullptr)
					{
						type = ptr->value();

						if (type.compare("3-wire-on-4-channels") == 0)
						{
							value = settings_node->value();
							if (value.compare("1") == 0)
							{
								m3WireOn4Channels = true;
							}
							else
							{
								m3WireOn4Channels = false;
							}
						}
						else if (type.compare("4-wire-on-3-channels") == 0)
						{
							value = settings_node->value();
							if (value.compare("1") == 0)
							{
								m4WireOn3Channels = true;
							}
							else
							{
								m4WireOn3Channels = false;
							}
						}
					}
				}

				if (m4WireOn3Channels && m3WireOn4Channels)
				{
					m4WireOn3Channels = false;
					m3WireOn4Channels = false;
					SetErrorText("Advanced settings: 4-wire-on-3-channels and 3-wire-on-4-channels are mutually exclusive features, both cannot be enabled simultaneously.\r\nPlease fix the configuration.");

					return false;
				}
				else
				{
					return true;
				}
			}
			else
			{
				SetErrorText("Advanced settings: File could not be properly parsed.");
				return false;
			}
		}
	}
	else
	{
		return true;
	}
}

bool SpiAnalyzerSettings::SetSettingsFromInterfaces()
{
	Channel mosi   = mMosiChannelInterface->GetChannel();
	Channel miso   = mMisoChannelInterface->GetChannel();
	Channel clock  = mClockChannelInterface->GetChannel();
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

	if ((mosi == UNDEFINED_CHANNEL) || (miso == UNDEFINED_CHANNEL) || (clock == UNDEFINED_CHANNEL))
	{
		SetErrorText("Please select a channel for MOSI, MISO, SCLK.");
		return false;
	}

	mMosiChannel   = mMosiChannelInterface->GetChannel();
	mMisoChannel   = mMisoChannelInterface->GetChannel();
	mClockChannel  = mClockChannelInterface->GetChannel();
	mEnableChannel = mEnableChannelInterface->GetChannel();

	mMessageIndexingVerbosityLevel = U32(mMessageIndexingVerbosityLevelInterface->GetNumber());
	mMsgDataPriority               = U32(mMsgDataPriorityInterface->GetNumber());
	mProcessDataPriority           = U32(mProcessDataPriorityInterface->GetNumber());
	mMessageSrcIdIndexing          = bool(mIndexMessageSrcIdInterface->GetValue());
	mErrorIndexing                 = bool(mIndexErrorsInterface->GetValue());
	mTimestampIndexing             = U32(mIndexTimestampsInterface->GetNumber());
	mAnybusStatusIndexing          = bool(mIndexAnybusStatusInterface->GetValue());
	mApplStatusIndexing            = bool(mIndexApplStatusInterface->GetValue());
	#if ENABLE_ADVANCED_SETTINGS
	mAdvSettingsPath               = mAdvancedSettingsInterface->GetText();
	#endif

	ClearChannels();
	AddChannel(mMosiChannel,   "MOSI",   mMosiChannel   != UNDEFINED_CHANNEL);
	AddChannel(mMisoChannel,   "MISO",   mMisoChannel   != UNDEFINED_CHANNEL);
	AddChannel(mClockChannel,  "CLOCK",  mClockChannel  != UNDEFINED_CHANNEL);
	AddChannel(mEnableChannel, "ENABLE", mEnableChannel != UNDEFINED_CHANNEL);
#if ENABLE_ADVANCED_SETTINGS
	mAdvSettingsPath = mAdvancedSettingsInterface->GetText();
	if (!ParseAdavancedSettingsFile())
	{
		return false;
	}
#endif
	return true;
}

void SpiAnalyzerSettings::LoadSettings(const char* settings)
{
	SimpleArchive text_archive;
	const char* pcPluginName;
	const char* pcSettingsVersionString;

	text_archive.SetString(settings);

	/* The first thing in the archive is the name of the protocol analyzer that the data belongs to. */
	text_archive >> &pcPluginName;
	if (strcmp(pcPluginName, "AbccSpiAnalyzer") != 0)
	{
		AnalyzerHelpers::Assert("AbccSpiAnalyzer: Provided with a settings string that doesn't belong to us.");
	}

	text_archive >> mMosiChannel;
	text_archive >> mMisoChannel;
	text_archive >> mClockChannel;
	text_archive >> mEnableChannel;

	/* Compare version in archive to what the plugin's "settings" version is */
	text_archive >> &pcSettingsVersionString;
	if (strcmp(pcSettingsVersionString, SETTINGS_REVISION_STRING) == 0)
	{
		text_archive >> mMessageIndexingVerbosityLevel;
		text_archive >> mMsgDataPriority;
		text_archive >> mProcessDataPriority;
		text_archive >> mMessageSrcIdIndexing;
		text_archive >> mErrorIndexing;
		text_archive >> mTimestampIndexing;
		text_archive >> mAnybusStatusIndexing;
		text_archive >> mApplStatusIndexing;
		#if ENABLE_ADVANCED_SETTINGS
		text_archive >> &mAdvSettingsPath;
		#endif
	}

	ClearChannels();
	AddChannel(mMosiChannel,   "MOSI",   mMosiChannel   != UNDEFINED_CHANNEL);
	AddChannel(mMisoChannel,   "MISO",   mMisoChannel   != UNDEFINED_CHANNEL);
	AddChannel(mClockChannel,  "CLOCK",  mClockChannel  != UNDEFINED_CHANNEL);
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

	text_archive << SETTINGS_REVISION_STRING;
	text_archive << mMessageIndexingVerbosityLevel;
	text_archive << mMsgDataPriority;
	text_archive << mProcessDataPriority;
	text_archive << mMessageSrcIdIndexing;
	text_archive << mErrorIndexing;
	text_archive << mTimestampIndexing;
	text_archive << mAnybusStatusIndexing;
	text_archive << mApplStatusIndexing;
	#if ENABLE_ADVANCED_SETTINGS
	text_archive << mAdvSettingsPath;
	#endif

	SaveSettingChangeID();

	return SetReturnString(text_archive.GetString());
}

void SpiAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mMosiChannelInterface->SetChannel(mMosiChannel);
	mMisoChannelInterface->SetChannel(mMisoChannel);
	mClockChannelInterface->SetChannel(mClockChannel);
	mEnableChannelInterface->SetChannel(mEnableChannel);

	mMessageIndexingVerbosityLevelInterface->SetNumber(mMessageIndexingVerbosityLevel);
	mMsgDataPriorityInterface->SetNumber(mMsgDataPriority);
	mProcessDataPriorityInterface->SetNumber(mProcessDataPriority);
	mIndexMessageSrcIdInterface->SetValue(mMessageSrcIdIndexing);
	mIndexErrorsInterface->SetValue(mErrorIndexing);
	mIndexTimestampsInterface->SetNumber(mTimestampIndexing);
	mIndexAnybusStatusInterface->SetValue(mAnybusStatusIndexing);
	mIndexApplStatusInterface->SetValue(mApplStatusIndexing);
	#if ENABLE_ADVANCED_SETTINGS
	mAdvancedSettingsInterface->SetText(mAdvSettingsPath);
	#endif
}

U8 SpiAnalyzerSettings::SaveSettingChangeID( void )
{
	mChangeID++;
	return mChangeID;
}

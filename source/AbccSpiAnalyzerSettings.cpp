/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerSettings.cpp
**    Summary: Handles storing and loading the user settings.
**
*******************************************************************************
******************************************************************************/

#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cctype>
#include <locale>

#include "rapidxml-1.13/rapidxml.hpp"

#include "AbccSpiAnalyzerSettings.h"
#include "AnalyzerHelpers.h"
#include "AbccSpiAnalyzerTypes.h"
#include "AbccSpiAnalyzerHelpers.h"

/* Any time behavior or definition of settings change where some level of
** incompatibility is introduced, increment this counter. This should be
** maintained at the commit level to improve reliability of custom builds
** at any point in the commit history. */
#define SETTINGS_REVISION_STRING "REVISION_00000012"

#define ANALYZER_NAME "AbccSpiAnalyzer"

#define MOSI_CHANNEL_NAME "MOSI"
#define MISO_CHANNEL_NAME "MISO"
#define SCLK_CHANNEL_NAME "SCLK"
#define NSS_CHANNEL_NAME  "NSS"

/*
** Overloads reading the SimpleArchive as a U32 and feeding the result
** into an enum class. For this to work the enum class MUST define a
** contiguous range of enums starting at 0 and end with "SizeOfEnum".
*/
template <class T>
bool operator>> (SimpleArchive &archive, T &custom_enum_class)
{
	U32 temp;
	archive >> temp;

	if (temp >= static_cast<U32>(T::SizeOfEnum))
	{
		custom_enum_class = T::SizeOfEnum;
	}

	custom_enum_class = static_cast<T>(temp);
	return true;
}

/*
** This routine is just here to provide some symmetry with accessing
** the SimpleArchive with enum classes
*/
template <class T>
bool operator<< (SimpleArchive &archive, T custom_enum_class)
{
	archive << static_cast<U32>(custom_enum_class);
	return true;
}

bool operator>> (SimpleArchive &archive, std::string &value)
{
	const char* temp;
	archive >> &temp;
	value.assign(temp);
	return true;
}

SpiAnalyzerSettings::SpiAnalyzerSettings()
	: mMosiChannel(UNDEFINED_CHANNEL),
	mMisoChannel(UNDEFINED_CHANNEL),
	mClockChannel(UNDEFINED_CHANNEL),
	mEnableChannel(UNDEFINED_CHANNEL),
	mNetworkType(NetworkTypeIndex::Unspecified),
	mMessageIndexingVerbosityLevel(MessageIndexing::Detailed),
	mMsgDataPriority(DisplayPriority::Tag),
	mProcessDataPriority(DisplayPriority::Tag),
	mTimestampIndexing(TimestampIndexing::Disabled),
	mMessageSrcIdIndexing(true),
	mErrorIndexing(true),
	mAnybusStatusIndexing(true),
	mApplStatusIndexing(true),
	mAdvSettingsPath(""),
	mChangeID(0)
{
	SetDefaultAdvancedSettings();

	mMosiChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mMosiChannelInterface->SetTitleAndTooltip(MOSI_CHANNEL_NAME, "Master Out, Slave In (Host to Module)");
	mMosiChannelInterface->SetChannel(mMosiChannel);

	mMisoChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mMisoChannelInterface->SetTitleAndTooltip(MISO_CHANNEL_NAME, "Master In, Slave Out (Module to Host)");
	mMisoChannelInterface->SetChannel(mMisoChannel);

	mClockChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mClockChannelInterface->SetTitleAndTooltip(SCLK_CHANNEL_NAME, "SPI Clock");
	mClockChannelInterface->SetChannel(mClockChannel);

	mEnableChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mEnableChannelInterface->SetTitleAndTooltip(NSS_CHANNEL_NAME, "Active Low Slave Select");
	mEnableChannelInterface->SetChannel(mEnableChannel);
	mEnableChannelInterface->SetSelectionOfNoneIsAllowed(true);


	mNetworkTypeInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mNetworkTypeInterface->SetTitleAndTooltip("Network Type :",
		"Used to process network specific details such as the network configuration object's instance names.\n"
		"Can be set to \"Unspecified\", if unsure or if such details are not important.");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::Unspecified,		"Unspecified","Use this option if you dont want to associate the capture with a specific network type.");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PDPV0,			"PROFIBUS DP-V0", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PDPV1,			"PROFIBUS DP-V1", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::COP,				"CANopen", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::DEV,				"DeviceNet", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::RTU,			"Modbus-RTU", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CNT,			"ControlNet", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::ETN_1P,		"Modbus-TCP", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PRT,			"PROFINET RT", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::EIP_1P,		"EtherNet/IP", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::ECT,				"EtherCAT", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PIR,				"PROFINET IRT", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CCL,				"CC-Link", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::ETN_2P,			"Modbus-TCP 2-Port", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CPN,			"CompoNet", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PRT_2P,		"PROFINET RT 2-port", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::SRC3,			"SERCOS III", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::BMP,			"BACnet MS/TP", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::BIP,				"BACnet/IP", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::EIP_2P_BB,		"EtherNet/IP 2-Port BB DLR", "");
	//mNetworkTypeInterface->AddNumber(NetworkTypeIndex::EIP_2P,		"EtherNet/IP 2-Port", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PIR_FO,			"PROFINET IRT FO", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::EPL,				"POWERLINK", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CFN,				"CC-Link IE Field Network", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CET,				"Common Ethernet", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::EIP_2P_BB_IIOT,	"EtherNet/IP IIoT", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PIR_IIOT,		"PROFINET IRT IIoT", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::PIR_FO_IIOT,		"PROFINET IRT FO IIoT", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CET_IIOT,		"Common Ethernet IIoT", "");
	mNetworkTypeInterface->AddNumber(NetworkTypeIndex::CIET,			"CC-Link IE TSN", "");
	mNetworkTypeInterface->SetNumber(mNetworkType);

	mIndexErrorsInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexErrorsInterface->SetTitleAndTooltip("Index - Errors :", "Enable indexed searching of errors.");
	mIndexErrorsInterface->SetValue(mErrorIndexing);

	mIndexAnybusStatusInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexAnybusStatusInterface->SetTitleAndTooltip("Index - Anybus Status :",
		"Enable indexed searching of Anybus status.\n"
		"Entries are added on status change events.\n"
		"First valid packet in capture will always trigger this event.");
	mIndexAnybusStatusInterface->SetValue(mAnybusStatusIndexing);

	mIndexApplStatusInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexApplStatusInterface->SetTitleAndTooltip("Index - Application Status :",
		"Enable indexed searching of application status.\n"
		"Entries are added on status change events.\n"
		"First valid packet in capture will always trigger this event.");
	mIndexApplStatusInterface->SetValue(mApplStatusIndexing);

	mIndexTimestampsInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mIndexTimestampsInterface->SetTitleAndTooltip("Index - Network Timestamp :",
		"Enable indexed searching of ABCC network timestamps.\n"
		"Useful for benchmarking; otherwise it is recommended to keep this option disabled.");
	mIndexTimestampsInterface->AddNumber(static_cast<double>(TimestampIndexing::Disabled),
		"Disabled", "No network timestamps will be added to tabular results.");
	mIndexTimestampsInterface->AddNumber(static_cast<double>(TimestampIndexing::AllPackets),
		"All Packets", "The timestamp from every ABCC SPI packet will be added to tabular results.");
	mIndexTimestampsInterface->AddNumber(static_cast<double>(TimestampIndexing::WriteProcessDataValid),
		"Write Process Data Valid", "The timestamp from ABCC SPI packets containing \"valid\" write process data will be added to tabular results.");
	mIndexTimestampsInterface->AddNumber(static_cast<double>(TimestampIndexing::NewReadProcessData),
		"New Read Process Data", "The timestamp from ABCC SPI packets containing \"new\" read process data will be added to tabular results.");
	mIndexTimestampsInterface->SetNumber(static_cast<double>(mTimestampIndexing));

	mIndexMessageSrcIdInterface.reset(new AnalyzerSettingInterfaceBool());
	mIndexMessageSrcIdInterface->SetTitleAndTooltip("Index - Message 'Source ID' :",
		"Enable indexed searching of the source ID associated with an ABCC transaction.");
	mIndexMessageSrcIdInterface->SetValue(mMessageSrcIdIndexing);

	mMessageIndexingVerbosityLevelInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mMessageIndexingVerbosityLevelInterface->SetTitleAndTooltip("Index - Message :",
		"Specifies how detailed the decoded protcols entries are.");
	mMessageIndexingVerbosityLevelInterface->AddNumber(static_cast<double>(MessageIndexing::Disabled),
		"Disabled", "Messages will not be indexed in searchable results.\n"
		"Use when object messaging is of no interest.");
	mMessageIndexingVerbosityLevelInterface->AddNumber(static_cast<double>(MessageIndexing::Compact),
		"Compact Results", "Message header information is added to a single tabular result.\n"
		"This option is useful when looking for very specific messages.");
	mMessageIndexingVerbosityLevelInterface->AddNumber(static_cast<double>(MessageIndexing::Detailed),
		"Verbose Results", "Message header information is added to tabular results individually.\n"
		"Recommended setting for general use.");
	mMessageIndexingVerbosityLevelInterface->SetNumber(static_cast<double>(mMessageIndexingVerbosityLevel));

	mMsgDataPriorityInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mMsgDataPriorityInterface->SetTitleAndTooltip("Message Data Priority :",
		"Specifies if the Message Data or Tag information is given priority in the display of multi-layered bubble-text.");
	mMsgDataPriorityInterface->AddNumber(static_cast<double>(DisplayPriority::Value),
		"Prioritize Data", "Message Data will be displayed as first layer of bubble text in analyzer results.");
	mMsgDataPriorityInterface->AddNumber(static_cast<double>(DisplayPriority::Tag),
		"Prioritize Tag", "Message Data will be displayed as second layer of bubble text in analyzer results.");
	mMsgDataPriorityInterface->SetNumber(static_cast<double>(mMsgDataPriority));

	mProcessDataPriorityInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mProcessDataPriorityInterface->SetTitleAndTooltip("Process Data Priority :",
		"Specifies if the Process Data or Tag information is given priority in the display of multi-layered bubble-text.");
	mProcessDataPriorityInterface->AddNumber(static_cast<double>(DisplayPriority::Value),
		"Prioritize Data", "Process Data will be displayed as first layer of bubble text in analyzer results.");
	mProcessDataPriorityInterface->AddNumber(static_cast<double>(DisplayPriority::Tag),
		"Prioritize Tag", "Process Data will be displayed as second layer of bubble text in analyzer results.");
	mProcessDataPriorityInterface->SetNumber(static_cast<double>(mProcessDataPriority));

	mAdvancedSettingsInterface.reset(new AnalyzerSettingInterfaceText());
	mAdvancedSettingsInterface->SetTextType(AnalyzerSettingInterfaceText::FilePath);
	mAdvancedSettingsInterface->SetTitleAndTooltip("Advanced Settings :",
		"Specifies external settings file to control special (advanced) settings of the plugin.\n"
		"Please refer to plugin documentation for more details.\n"
		"If left empty plugin defaults will be used which are suitable for most situations.\n"
		"NOTE: Relative paths are respective to where Logic executable resides.");
	mAdvancedSettingsInterface->SetText(mAdvSettingsPath);

	AddInterface(mMosiChannelInterface.get());
	AddInterface(mMisoChannelInterface.get());
	AddInterface(mClockChannelInterface.get());
	AddInterface(mEnableChannelInterface.get());

	AddInterface(mNetworkTypeInterface.get());
	AddInterface(mIndexErrorsInterface.get());
	AddInterface(mIndexTimestampsInterface.get());
	AddInterface(mIndexAnybusStatusInterface.get());
	AddInterface(mIndexApplStatusInterface.get());
	AddInterface(mIndexMessageSrcIdInterface.get());
	AddInterface(mMessageIndexingVerbosityLevelInterface.get());
	AddInterface(mMsgDataPriorityInterface.get());
	AddInterface(mProcessDataPriorityInterface.get());
	AddInterface(mAdvancedSettingsInterface.get());

	AddExportOption(static_cast<U32>(ExportType::Frames), "Export All Frame Data");
	AddExportExtension(static_cast<U32>(ExportType::Frames), "All Frame Data", "csv");
	AddExportOption(static_cast<U32>(ExportType::ProcessData), "Export Process Data");
	AddExportExtension(static_cast<U32>(ExportType::ProcessData), "Process Data", "csv");
	AddExportOption(static_cast<U32>(ExportType::MessageData), "Export Message Data");
	AddExportExtension(static_cast<U32>(ExportType::MessageData), "Message Data", "csv");

	ClearChannels();
	AddChannel(mMosiChannel, MOSI_CHANNEL_NAME, false);
	AddChannel(mMisoChannel, MISO_CHANNEL_NAME, false);
	AddChannel(mClockChannel, SCLK_CHANNEL_NAME, false);
	AddChannel(mEnableChannel, NSS_CHANNEL_NAME, false);
}

SpiAnalyzerSettings::~SpiAnalyzerSettings()
{
}

void SpiAnalyzerSettings::SetDefaultAdvancedSettings()
{
	m3WireOn4Channels = false;
	m4WireOn3Channels = false;
	mExportDelimiter.assign(",");
	mClockingAlertLimit = -1;
	mExpandBitFrames = true;
	mSimulateLogFilePath = "";
	mSimulateLogFileDefaultState = ABP_ANB_STATE_SETUP;
	mSimulateClockIdleHigh = -1;
	mSimulateClockFrequency = 0;
	mSimulatePacketGapNs = 0;
	mSimulateByteGapNs = 0;
	mSimulateChipSelectNs = 0;
	mSimulateWordMode = false;
	mSimulateMsgDataLength = 8;
}

void SpiAnalyzerSettings::ParseSimulationSettings(rapidxml::xml_node<>* simulation_node)
{
	// Simulation node must have the following data in the order specified:
	const char* firstNode = "LogFilePath";
	const char* secondNode = "LogFileDefaultAnbState";
	const char* thirdNode = "SpiClockIdleHigh";
	const char* fourthNode = "SpiClockFrequency";
	const char* fifthNode = "SpiPacketGapNs";
	const char* sixthNode = "SpiByteGapNs";
	const char* seventhNode = "SpiChipSelectDelayNs";
	const char* eighthNode = "SpiDataSize";
	const char* ninthNode = "SpiMessageDataLength";

	rapidxml::xml_node<>* node = simulation_node->first_node(firstNode);

	if (node)
	{
		mSimulateLogFilePath = node->value();
		TrimString(mSimulateLogFilePath);
		node = node->next_sibling(secondNode);
	}
	else
	{
		node = simulation_node->first_node(secondNode);
	}

	if (node)
	{
		const int reservedValue = 6;
		long parsedValue = strtol(node->value(), nullptr, 0);

		if ((parsedValue >= ABP_ANB_STATE_SETUP) &&
			(parsedValue <= ABP_ANB_STATE_EXCEPTION) &&
			(parsedValue != reservedValue))
		{
			mSimulateLogFileDefaultState = static_cast<U32>(parsedValue);
		}

		node = node->next_sibling(thirdNode);
	}
	else
	{
		node = simulation_node->first_node(thirdNode);
	}

	if (node)
	{
		long parsedValue = strtol(node->value(), nullptr, 0);

		mSimulateClockIdleHigh = static_cast<S32>(parsedValue);
		node = node->next_sibling(fourthNode);
	}
	else
	{
		node = simulation_node->first_node(fourthNode);
	}

	if (node)
	{
		long parsedValue = strtol(node->value(), nullptr, 0);

		mSimulateClockFrequency = static_cast<S32>(parsedValue);
		node = node->next_sibling(fifthNode);
	}
	else
	{
		node = simulation_node->first_node(fifthNode);
	}

	if (node)
	{
		long parsedValue = strtol(node->value(), nullptr, 0);

		mSimulatePacketGapNs = static_cast<S32>(parsedValue);
		node = node->next_sibling(sixthNode);
	}
	else
	{
		node = simulation_node->first_node(sixthNode);
	}

	if (node)
	{
		long parsedValue = strtol(node->value(), nullptr, 0);

		mSimulateByteGapNs = static_cast<S32>(parsedValue);
		node = node->next_sibling(seventhNode);
	}
	else
	{
		node = simulation_node->first_node(seventhNode);
	}

	if (node)
	{
		long parsedValue = strtol(node->value(), nullptr, 0);

		if (parsedValue < 0)
		{
			parsedValue = 0;
		}

		mSimulateChipSelectNs = static_cast<S32>(parsedValue);
		node = node->next_sibling(eighthNode);
	}
	else
	{
		node = simulation_node->first_node(eighthNode);
	}

	if (node)
	{
		const int wordMode = 16;
		long parsedValue = strtol(node->value(), nullptr, 0);

		mSimulateWordMode = (parsedValue == wordMode);
		node = node->next_sibling(ninthNode);
	}
	else
	{
		node = simulation_node->first_node(ninthNode);
	}

	if (node)
	{
		const int defaultValue = 8;
		const int maxMessageDataWords = 762;
		long parsedValue = strtol(node->value(), nullptr, 0);

		if (parsedValue == 0 ||
			parsedValue > maxMessageDataWords ||
			parsedValue < -maxMessageDataWords)
		{
			mSimulateMsgDataLength = defaultValue;
		}
		else
		{
			mSimulateMsgDataLength = static_cast<S32>(parsedValue);
		}
	}
}

bool SpiAnalyzerSettings::ParseAdvancedSettingsFile()
{
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> * rootNode;
	std::string trimmedPath(mAdvSettingsPath);
	std::string nodeName;
	std::string nodeValue;
	const std::string settingName = "Advanced settings";
	bool settingsValid = true;

	SetDefaultAdvancedSettings();

	/*
	** Trim the file path so that an entry with nothing
	** but whitespace is ignored as a valid file path
	*/
	TrimString(trimmedPath);

	/* Read the xml file into a vector */
	if (trimmedPath.length() > 0)
	{
		std::ifstream filestream(trimmedPath);

		if (!filestream)
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
			rootNode = doc.first_node("AdvancedSettings");

			if (rootNode)
			{
				/* Iterate over each available setting */
				for (rapidxml::xml_node<> * settings_node = rootNode->first_node("Setting"); settings_node; settings_node = settings_node->next_sibling())
				{
					rapidxml::xml_attribute<char>* ptr = settings_node->first_attribute("name");

					/* Get the type of setting */
					if (ptr != nullptr)
					{
						nodeName = ptr->value();
						nodeValue = settings_node->value();
						TrimString(nodeValue);

						if (nodeName.compare("3-wire-on-4-channels") == 0)
						{
							m3WireOn4Channels = (nodeValue.compare("1") == 0);
						}
						else if (nodeName.compare("4-wire-on-3-channels") == 0)
						{
							m4WireOn3Channels = (nodeValue.compare("1") == 0);
						}
						else if (nodeName.compare("export-delimiter") == 0)
						{
							if (nodeValue.length() == 1)
							{
								mExportDelimiter = nodeValue.at(0);
							}
							else if (nodeValue.length() == 2)
							{
								if ((nodeValue.at(0) == '\\') && (nodeValue.at(1) == 't'))
								{
									mExportDelimiter.assign("\t");
								}
							}
						}
						else if (nodeName.compare("clocking-alert-limit") == 0)
						{
							try
							{
								mClockingAlertLimit = (S32)std::stol(nodeValue, nullptr);
							}
							catch (const std::invalid_argument& ia)
							{
								(void)ia;
								mClockingAlertLimit = -1;
							}
						}
						else if (nodeName.compare("expand-bit-frames") == 0)
						{
							mExpandBitFrames = (nodeValue.compare("1") == 0);
						}
						else if (nodeName.compare("simulation") == 0)
						{
							// Attempt to get applicable settings for simulation from child nodes.
							ParseSimulationSettings(settings_node);
						}
					}
					else
					{
						SetSettingError(settingName, "XML node must contain a valid \"name\" attribute.");
						settingsValid = false;
						break;
					}
				}

				if (m4WireOn3Channels && m3WireOn4Channels)
				{
					settingsValid = false;
					m4WireOn3Channels = false;
					m3WireOn4Channels = false;
					SetSettingError(settingName, "4-wire-on-3-channels and 3-wire-on-4-channels are mutually exclusive features, both cannot be enabled simultaneously.\r\nPlease fix the configuration.");
				}
			}
			else
			{
				SetSettingError(settingName, "File could not be properly parsed.");
			}
		}
	}

	return settingsValid;
}

void SpiAnalyzerSettings::SetSettingError(
	const std::string &setting_name,
	const std::string &error_text)
{
	std::string s = "";

	s += setting_name + " : " + error_text;

	SetErrorText(s.c_str());
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

	if (AnalyzerHelpers::DoChannelsOverlap(&channels[0], (U32)channels.size()))
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

	mNetworkType                   = U32(mNetworkTypeInterface->GetNumber());
	mMessageIndexingVerbosityLevel = static_cast<MessageIndexing>(U32(mMessageIndexingVerbosityLevelInterface->GetNumber()));
	mMsgDataPriority               = static_cast<DisplayPriority>(U32(mMsgDataPriorityInterface->GetNumber()));
	mProcessDataPriority           = static_cast<DisplayPriority>(U32(mProcessDataPriorityInterface->GetNumber()));
	mMessageSrcIdIndexing          = mIndexMessageSrcIdInterface->GetValue();
	mErrorIndexing                 = mIndexErrorsInterface->GetValue();
	mTimestampIndexing             = static_cast<TimestampIndexing>(U32(mIndexTimestampsInterface->GetNumber()));
	mAnybusStatusIndexing          = mIndexAnybusStatusInterface->GetValue();
	mApplStatusIndexing            = mIndexApplStatusInterface->GetValue();
	mAdvSettingsPath               = mAdvancedSettingsInterface->GetText();

	ClearChannels();
	AddChannel(mMosiChannel,   MOSI_CHANNEL_NAME,   mMosiChannel   != UNDEFINED_CHANNEL);
	AddChannel(mMisoChannel,   MISO_CHANNEL_NAME,   mMisoChannel   != UNDEFINED_CHANNEL);
	AddChannel(mClockChannel,  SCLK_CHANNEL_NAME,   mClockChannel  != UNDEFINED_CHANNEL);
	AddChannel(mEnableChannel, NSS_CHANNEL_NAME,    mEnableChannel != UNDEFINED_CHANNEL);

	return ParseAdvancedSettingsFile();
}

void SpiAnalyzerSettings::LoadSettings(const char* settings)
{
	SimpleArchive textArchive;
	const char* pluginName;
	const char* settingsVersionString;

	textArchive.SetString(settings);

	/* The first thing in the archive is the name of the protocol analyzer that the data belongs to. */
	textArchive >> &pluginName;
	if (strcmp(pluginName, ANALYZER_NAME) != 0)
	{
		AnalyzerHelpers::Assert("AbccSpiAnalyzer: Provided with a settings string that doesn't belong to us.");
	}

	textArchive >> mMosiChannel;
	textArchive >> mMisoChannel;
	textArchive >> mClockChannel;
	textArchive >> mEnableChannel;

	/* Compare version in archive to what the plugin's "settings" version is */
	textArchive >> &settingsVersionString;
	if (strcmp(settingsVersionString, SETTINGS_REVISION_STRING) == 0)
	{
		textArchive >> mNetworkType;
		textArchive >> mMessageIndexingVerbosityLevel;
		textArchive >> mMsgDataPriority;
		textArchive >> mProcessDataPriority;
		textArchive >> mMessageSrcIdIndexing;
		textArchive >> mErrorIndexing;
		textArchive >> mTimestampIndexing;
		textArchive >> mAnybusStatusIndexing;
		textArchive >> mApplStatusIndexing;
		textArchive >> m3WireOn4Channels;
		textArchive >> m4WireOn3Channels;
		textArchive >> mExportDelimiter;
		textArchive >> mClockingAlertLimit;
		textArchive >> mExpandBitFrames;
		textArchive >> &mAdvSettingsPath;
	}

	ClearChannels();
	AddChannel(mMosiChannel,   MOSI_CHANNEL_NAME,   mMosiChannel   != UNDEFINED_CHANNEL);
	AddChannel(mMisoChannel,   MISO_CHANNEL_NAME,   mMisoChannel   != UNDEFINED_CHANNEL);
	AddChannel(mClockChannel,  SCLK_CHANNEL_NAME,   mClockChannel  != UNDEFINED_CHANNEL);
	AddChannel(mEnableChannel, NSS_CHANNEL_NAME,    mEnableChannel != UNDEFINED_CHANNEL);

	UpdateInterfacesFromSettings();
}

const char* SpiAnalyzerSettings::SaveSettings()
{
	SimpleArchive textArchive;

	textArchive << ANALYZER_NAME;
	textArchive << mMosiChannel;
	textArchive << mMisoChannel;
	textArchive << mClockChannel;
	textArchive << mEnableChannel;

	textArchive << SETTINGS_REVISION_STRING;
	textArchive << mNetworkType;
	textArchive << mMessageIndexingVerbosityLevel;
	textArchive << mMsgDataPriority;
	textArchive << mProcessDataPriority;
	textArchive << mMessageSrcIdIndexing;
	textArchive << mErrorIndexing;
	textArchive << mTimestampIndexing;
	textArchive << mAnybusStatusIndexing;
	textArchive << mApplStatusIndexing;
	textArchive << m3WireOn4Channels;
	textArchive << m4WireOn3Channels;
	textArchive << mExportDelimiter.c_str();
	textArchive << mClockingAlertLimit;
	textArchive << mExpandBitFrames;
	textArchive << mAdvSettingsPath;

	SaveSettingChangeID();

	return SetReturnString(textArchive.GetString());
}

void SpiAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mMosiChannelInterface->SetChannel(mMosiChannel);
	mMisoChannelInterface->SetChannel(mMisoChannel);
	mClockChannelInterface->SetChannel(mClockChannel);
	mEnableChannelInterface->SetChannel(mEnableChannel);

	mNetworkTypeInterface->SetNumber(mNetworkType);
	mMessageIndexingVerbosityLevelInterface->SetNumber(static_cast<double>(mMessageIndexingVerbosityLevel));
	mMsgDataPriorityInterface->SetNumber(static_cast<double>(mMsgDataPriority));
	mProcessDataPriorityInterface->SetNumber(static_cast<double>(mProcessDataPriority));
	mIndexMessageSrcIdInterface->SetValue(mMessageSrcIdIndexing);
	mIndexErrorsInterface->SetValue(mErrorIndexing);
	mIndexTimestampsInterface->SetNumber(static_cast<double>(mTimestampIndexing));
	mIndexAnybusStatusInterface->SetValue(mAnybusStatusIndexing);
	mIndexApplStatusInterface->SetValue(mApplStatusIndexing);
	mAdvancedSettingsInterface->SetText(mAdvSettingsPath);
}

U8 SpiAnalyzerSettings::SaveSettingChangeID()
{
	mChangeID++;
	return mChangeID;
}

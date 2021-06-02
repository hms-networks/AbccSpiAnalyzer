/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerResults.h
**    Summary: Handles various tasks for converting frame and packet data into
**             bubble-text, tabular-text, and exported CSV files.
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_ANALYZER_RESULTS_H
#define ABCC_SPI_ANALYZER_RESULTS_H

#include "AnalyzerResults.h"
#include "AbccSpiAnalyzerTypes.h"

#ifndef FORMATTED_STRING_BUFFER_SIZE
#define FORMATTED_STRING_BUFFER_SIZE 256
#endif

#ifndef NUM_DATA_CHANNELS
#define NUM_DATA_CHANNELS 2
#endif

enum class ErrorEvent : U32
{
	None,
	RetransmitWarning,
	CrcError,
	SpiFragmentationError,
	SizeOfEnum
};

class SpiAnalyzer;
class SpiAnalyzerSettings;

class SpiAnalyzerResults : public AnalyzerResults
{
public:

	SpiAnalyzerResults(SpiAnalyzer* analyzer, SpiAnalyzerSettings* settings);
	virtual ~SpiAnalyzerResults();

	virtual void GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base);
	virtual void GenerateExportFile(const char* file, DisplayBase display_base, U32 export_type_user_id);

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base);
	virtual void GeneratePacketTabularText(U64 packet_id, DisplayBase display_base);
	virtual void GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base);

	virtual U64 GetFrameIdOfAbccFieldContainedInPacket(U64 packet_index, SpiChannel_t channel, U8 type);

protected: /* Enums, Types, and Classes */

	typedef union AbccSpiStatesUnion
	{
		U8 bState;
		AbccMisoStates::Enum eMiso;
		AbccMosiStates::Enum eMosi;
	} AbccSpiStatesUnion_t;

protected:  /* Members */

	SpiAnalyzerSettings* mSettings;
	SpiAnalyzer* mAnalyzer;
	char mMsgSizeStr[NUM_DATA_CHANNELS][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgSrcStr[NUM_DATA_CHANNELS][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgObjStr[NUM_DATA_CHANNELS][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgInstStr[NUM_DATA_CHANNELS][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgCmdStr[NUM_DATA_CHANNELS][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgExtStr[NUM_DATA_CHANNELS][FORMATTED_STRING_BUFFER_SIZE];
	bool mMsgValidFlag[NUM_DATA_CHANNELS];
	bool mMsgErrorRspFlag[NUM_DATA_CHANNELS];

protected: /* Methods */

	void WriteBubbleText(const char* tag, const char* value, const char* verbose, NotifEvent_t notification, DisplayPriority disp_priority = DisplayPriority::Tag);
	void WriteTabularText(SpiChannel_t channel, const char* text, NotifEvent_t notification);
	void FormatTabularTextBuffer(char* buffer, size_t buffer_size, const char* tag, const char* text, NotifEvent_t notification);

	void BuildSpiCtrlString(U8 spi_control, DisplayBase display_base);
	void BuildSpiStsString(U8 spi_status, DisplayBase display_base);
	bool BuildCmdString(U8 command, U8 obj, DisplayBase display_base);
	void BuildErrorRsp(U8 error_code, DisplayBase display_base);
	void BuildErrorRsp(bool nw_spec_err, U8 nw_type_idx, U8 error_code, U8 obj, DisplayBase display_base);
	void BuildAbccStatus(U8 abcc_status, DisplayBase display_base);
	void BuildApplStatus(U8 appl_status, DisplayBase display_base);
	void BuildIntMask(U8 int_mask, DisplayBase display_base);
	void BuildObjectString(U8 object_num, DisplayBase display_base);

	void BuildInstString(U8 nw_type_idx, U8 obj, U16 inst, DisplayBase display_base);
	void BuildAttrString(const MsgHeaderInfo_t* msg_header_ptr, U16 attr, AttributeAccessMode_t access_mode, DisplayBase display_base);

	void ExportAllFramesToFile(const char* file, DisplayBase display_base);
	void ExportMessageDataToFile(const char* file, DisplayBase display_base);
	void ExportProcessDataToFile(const char* file, DisplayBase display_base);

	void BufferCsvMessageMsgEntry(
		Frame& frame,
		std::stringstream& ss_csv_data,
		bool& align_msg_fields,
		DisplayBase display_base);

	void BufferCsvMessageMisoEntry(
		U32 sample_rate,
		U64 trigger_sample,
		U64 packet_id,
		Frame& frame,
		std::stringstream& ss_csv_head,
		std::stringstream& ss_csv_body,
		std::stringstream& ss_csv_tail,
		ErrorEvent& mosi_event,
		ErrorEvent& miso_event,
		bool& fragmentation,
		bool& anb_stat_reached,
		bool& align_msg_fields,
		bool& add_entry,
		DisplayBase display_base);

	void BufferCsvMessageMosiEntry(
		U32 sample_rate,
		U64 trigger_sample,
		U64 packet_id,
		Frame &frame,
		std::stringstream &ss_csv_head,
		std::stringstream &ss_csv_body,
		std::stringstream &ss_csv_tail,
		ErrorEvent &mosi_event,
		ErrorEvent &miso_event,
		bool &fragmentation,
		bool &app_stat_reached,
		bool &align_msg_fields,
		bool &add_entry,
		DisplayBase display_base );

	void AppendCsvMessageEntry(void* file, std::stringstream &ss_csv_head, std::stringstream &ss_csv_body, std::stringstream &ss_csv_tail, ErrorEvent event);
	void AppendCsvSafeString(std::stringstream &ss_csv_data, char* input_data_str, DisplayBase display_base);

	void GenerateMessageTabularText(SpiChannel_t channel, Frame &frame, DisplayBase display_base);
	void GenerateMisoTabularText(U64 frame_index, Frame &frame, DisplayBase display_base);
	void GenerateMosiTabularText(U64 frame_index, Frame &frame, DisplayBase display_base);

	void GenerateMessageBubbleText(Frame &frame, DisplayBase display_base);
	void GenerateMisoBubbleText(Frame &frame, DisplayBase display_base);
	void GenerateMosiBubbleText(Frame &frame, DisplayBase display_base);
};

#endif /* ABCC_SPI_ANALYZER_RESULTS_H */

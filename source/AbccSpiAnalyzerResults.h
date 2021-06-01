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
#define FORMATTED_STRING_BUFFER_SIZE		256
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
	char mMsgSizeStr[2][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgSrcStr[2][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgObjStr[2][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgInstStr[2][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgCmdStr[2][FORMATTED_STRING_BUFFER_SIZE];
	char mMsgExtStr[2][FORMATTED_STRING_BUFFER_SIZE];
	bool mMsgValidFlag[2];
	bool mMsgErrorRspFlag[2];

protected: /* Methods */

	void StringBuilder(const char* tag, const char* value, const char* verbose, NotifEvent_t notification);
	void StringBuilder(const char* tag, const char* value, const char* verbose, NotifEvent_t notification, DisplayPriority disp_priority);
	void TableBuilder(SpiChannel_t channel, const char* text, NotifEvent_t notification);

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

	void AppendCsvMessageEntry(void* file, std::stringstream &ss_csv_head, std::stringstream &ss_csv_body, std::stringstream &ss_csv_tail, ErrorEvent event);
	void AppendCsvSafeString(std::stringstream &ss_csv_data, char* input_data_str, DisplayBase display_base);
};

#endif /* ABCC_SPI_ANALYZER_RESULTS_H */

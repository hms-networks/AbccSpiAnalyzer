/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerResults.h
**    Summary: DLL-Results header
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_ANALYZER_RESULTS_H
#define ABCC_SPI_ANALYZER_RESULTS_H

#include <AnalyzerResults.h>

#define SPI_ERROR_FLAG ( 1 << 0 ) /* Indicates a SPI settings error (e.g. CPOL, CPHA, EN Active Hi/Lo) */
#define SPI_MOSI_FLAG ( 1 << 1 )  /* Direction flag. When asserted, MOSI, when deasserted MISO */

#define SPI_MSG_FIRST_FRAG_FLAG ( 1 << 2 ) /* Indicates the first message in a fragmented message transfer */
#define SPI_MSG_FRAG_FLAG ( 1 << 3 ) /* Indicates that message fragmentation is in progress */

#define SPI_FRAG_ERROR_FLAG ( 1 << 4 )  /* Error flag to indicate an error occurred due to a fragmented SPI transaction.
										** This means that chipselect was brought high before the statemachine reached idle */
#define SPI_PROTO_EVENT_FLAG ( 1 << 5 ) /* Event flag to indicate any critical events that are part of the ABCC SPI protocol
										** This flag is field-specific.
										** This flag is relevant for the following fields (not all supported yet):
										**   - SPI_CTL: signals a toggle error (retransmission event)
										**   - ANB_STS: signals a Anybus status changed event
										**   - SPI_STS: signals a toggle error (retransmission event)
										**   - APP_STS: signals an application status changed event
										**   - MSG_CMD: signals an error response message
										**   - CRC32: signals a checksum error */

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

	virtual U64 GetFrameIdOfAbccFieldContainedInPacket(U64 packet_index, bool fMosiChannel, U8 type);

protected: /* functions */
	virtual void StringBuilder(char* tag, char* value, char* verbose, bool alert);
	virtual void StringBuilder(char* tag, char* value, char* verbose, bool alert, bool prioritizeValue);
	virtual void TableBuilder(bool fMosiChannel, char* text, bool alert);

	virtual void BuildSpiCtrlString(U8 val, DisplayBase display_base);
	virtual void BuildSpiStsString(U8 val, DisplayBase display_base);
	virtual bool BuildCmdString(U8 val, U8 obj, DisplayBase display_base);
	virtual void BuildErrorRsp(U8 val, DisplayBase display_base);
	virtual void BuildAbccStatus(U8 val, DisplayBase display_base);
	virtual void BuildApplStatus(U8 val, DisplayBase display_base);
	virtual void BuildIntMask(U8 val, DisplayBase display_base);
	virtual void BuildObjectString(U8 val, DisplayBase display_base);

	virtual void BuildAttrString(U8 obj, U16 inst, U16 val, bool indexed, DisplayBase display_base);

protected:  /* variables */
	SpiAnalyzerSettings* mSettings;
	SpiAnalyzer* mAnalyzer;
};

#endif /* ABCC_SPI_ANALYZER_RESULTS_H */

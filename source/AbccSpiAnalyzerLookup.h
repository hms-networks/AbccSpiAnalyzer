/******************************************************************************
**  Copyright (C) 2015-2022 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerLookup.h
**    Summary: Primarily contains lookup table routines for converting various
**             information into verbose information.
**
*******************************************************************************
******************************************************************************/
#ifndef ABCC_SPI_ANALYZER_LOOKUP_H_
#define ABCC_SPI_ANALYZER_LOOKUP_H_

#include "Analyzer.h"
#include "AbccSpiAnalyzerTypes.h"

enum class BaseType : U8
{
	Character,
	Numeric
};

enum class SegmentationType : U8
{
	None,
	Command,
	Response
};

typedef struct LookupTable_t
{
	const U16 value;
	const char* name;
	const NotifEvent_t notification;
} LookupTable_t;

typedef struct AttrLookupTable_t
{
	const U16 value;
	const char* name;
	const BaseType msgDataType;
	const NotifEvent_t notification;
} AttrLookupTable_t;

typedef struct CmdLookupTable_t
{
	const U16 value;
	const char* name;
	const BaseType commandType;
	const BaseType responseType;
	const NotifEvent_t notification;
} CmdLookupTable_t;

inline bool IsAttributeCmd(U8 cmd)
{
	return (
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR) ||
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
}

inline bool IsIndexedAttributeCmd(U8 cmd)
{
	return (
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_INDEXED_ATTR) ||
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_INDEXED_ATTR));
}

inline bool IsNonIndexedAttributeCmd(U8 cmd)
{
	return (
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_GET_ATTR) ||
		(static_cast<ABP_MsgCmdType>(cmd & ABP_MSG_HEADER_CMD_BITS) == ABP_CMD_SET_ATTR));
}

inline bool IsCommandMessage(const MsgHeaderInfo_t* msg_header)
{
	return (msg_header->cmd & ABP_MSG_HEADER_C_BIT);
}

inline bool IsSegmentedMessage(bool cmd_message, SegmentationType segmentation_type)
{
	return (
		(cmd_message && (segmentation_type == SegmentationType::Command)) ||
		(!cmd_message && (segmentation_type == SegmentationType::Response)));
}

NotifEvent_t GetSpiCtrlString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetSpiStsString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetApplStsString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetAbccStatusString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetErrorRspString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetErrorRspString(bool nw_spec_err, U8 nw_type_idx, U8 obj, U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetIntMaskString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetLedStatusString(U16 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetNamedAttrString(U16 inst, U8 val,
								char* str, U16 max_str_len,
								DisplayBase display_base,
								const AttrLookupTable_t* obj_names, U8 num_obj_names,
								const AttrLookupTable_t* inst_names, U8 num_inst_names);

NotifEvent_t GetObjectString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetObjSpecificCmdString(U8 val, char* str, U16 max_str_len, const CmdLookupTable_t* command_names, U8 num_commands, DisplayBase display_base);

NotifEvent_t GetCmdString(U8 val, U8 obj, char* str, U16 max_str_len, DisplayBase display_base);

bool GetInstString(U8 nw_type_idx, U8 obj, U16 val, char* str, U16 max_str_len, NotifEvent_t* notif_ptr, DisplayBase display_base);

bool GetAttrString(U8 obj, U16 inst, U16 val, char* str, U16 max_str_len, AttributeAccessMode_t access_mode, NotifEvent_t* notif_ptr, DisplayBase display_base);

void GetNumberString(U64 number, DisplayBase display_base, U32 num_data_bits, char* result_string, U32 result_string_max_length, BaseType base_type);

bool GetExceptionTableIndex(bool nw_object, U8 nw_type_idx, const MsgHeaderInfo_t* msg_header, U16* table_index);

NotifEvent_t GetExceptionString(bool nw_object, U16 table_index, U8 val, char* str, U16 max_str_len, DisplayBase display_base);

BaseType GetAttrBaseType(U8 obj, U16 inst, U8 attr);

BaseType GetCmdBaseType(U8 obj, U8 cmd);

SegmentationType GetMessageSegmentationType(const MsgHeaderInfo_t* msg_header);

#endif /* ABCC_SPI_ANALYZER_LOOKUP_H_ */

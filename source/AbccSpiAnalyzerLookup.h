/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerLookup.h
**    Summary: Lookup Table routines for DLL-Results
**     Author: Jon Carrier
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

NotifEvent_t GetSpiCtrlString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetSpiStsString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetApplStsString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetAbccStatusString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetErrorRspString(U8 val, char* str, U16 max_str_len, DisplayBase display_base);

NotifEvent_t GetErrorRspString(U8 nw_type_idx, U8 obj, U8 val, char* str, U16 max_str_len, DisplayBase display_base);

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

BaseType GetAttrBaseType(U8 obj, U16 inst, U8 attr);

BaseType GetCmdBaseType(U8 obj, U8 cmd);

#endif /* ABCC_SPI_ANALYZER_LOOKUP_H_ */

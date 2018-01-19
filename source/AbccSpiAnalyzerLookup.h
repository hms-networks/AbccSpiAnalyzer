/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
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

bool GetSpiCtrlString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetSpiStsString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetApplStsString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetAbccStatusString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetErrorRspString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetErrorRspString(U8 obj, U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetIntMaskString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetLedStatusString(U16 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetNamedAttrString(U16 inst, U8 val,
						char *str, U16 max_str_len,
						DisplayBase display_base,
						tValueName *pasObjNames, U8 NoObjNames,
						tValueName *pasInstNames, U8 NoInstNames);

bool GetObjectString(U8 val, char *str, U16 max_str_len, DisplayBase display_base);

bool GetObjSpecificCmdString(U8 val, char *str, U16 max_str_len, tValueName *command_names, U8 num_commands, DisplayBase display_base);

bool GetCmdString(U8 val, U8 obj, char *str, U16 max_str_len, DisplayBase display_base);

bool GetInstString(U8 nw_type_idx, U8 obj, U16 val, char *str, U16 max_str_len, bool *pAlert, DisplayBase display_base);

bool GetAttrString(U8 obj, U16 inst, U16 val, char *str, U16 max_str_len, bool indexed, bool *pAlert, DisplayBase display_base);

#endif /* ABCC_SPI_ANALYZER_LOOKUP_H_ */

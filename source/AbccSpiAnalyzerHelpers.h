/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerHelpers.h
**    Summary: Defines miscellaneous functions which may be utilized by various
**             units of the AbccSpiAnalyzer plugin.
**
*******************************************************************************
******************************************************************************/

#include <string>

/*
** Replace ocurrances of a character with another
*/
void StringReplace(std::string& line, const char search_ch, const char replace_ch);

/*
** Trim from start (in place)
*/
void TrimLeft(std::string& s);

/*
** Trim from end (in place)
*/
void TrimRight(std::string& s);

/*
** Trim from both ends (in place)
*/
void TrimString(std::string& s);

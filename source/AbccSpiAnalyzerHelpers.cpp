/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiAnalyzerHelpers.cpp
**    Summary: Defines miscellaneous functions which may be utilized by various
**             units of the AbccSpiAnalyzer plugin.
**
*******************************************************************************
******************************************************************************/

#include <string>
#include <algorithm>
#include "AbccSpiAnalyzerHelpers.h"

/*
** Replace ocurrances of a character with another
*/
void StringReplace(std::string& line, const char search_ch, const char replace_ch)
{
	size_t offset = 0;

	while (true)
	{
		offset = line.find(search_ch, offset);

		if (offset == std::string::npos)
		{
			break;
		}

		line[offset++] = replace_ch;
	}
}

/*
** Trim from start (in place)
*/
void TrimLeft(std::string& s)
{
	s.erase(
		s.begin(),
		std::find_if(
			s.begin(),
			s.end(),
			[](int ch) {
				return !std::isspace(ch);
			}
		)
	);
}

/*
** Trim from end (in place)
*/
void TrimRight(std::string& s)
{
	s.erase(
		std::find_if(
			s.rbegin(),
			s.rend(),
			[](int ch) {
				return !std::isspace(ch);
			}
		).base(),
		s.end()
	);
}

/*
** Trim from both ends (in place)
*/
void TrimString(std::string& s)
{
	TrimLeft(s);
	TrimRight(s);
}

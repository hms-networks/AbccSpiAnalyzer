/******************************************************************************
**  Copyright (C) 2015-2018 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccCrc.h
**    Summary: ABCC Crc-Unit header
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/
#ifndef ABCC_CRC_H
#define ABCC_CRC_H

#include "Analyzer.h"

class AbccCrc
{
public:
	void Init();
	void Update(U8* pbBufferStart, U16 iLength);
	U16 Crc16();
	U32 Crc32();

private:
	U32 mCrc32;
	U16 mCrc16;

	U32 CRC_Crc32(U32 iInitCrc, U8* pbBufferStart, U16 iLength);
	U16 CRC_Crc16(U16 iInitCrc, U8* pbBufferStart, U16 iLength);
	U32 CRC_FormatCrc32(U32 lCrc);
	U16 CRC_FormatCrc16(U16 lCrc);
	//protected: //functions
};

#endif /* ABCC_CRC_H */

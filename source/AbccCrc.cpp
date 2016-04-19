/******************************************************************************
**  Copyright (C) 1996-2016 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccCrc.cpp
**    Summary: ABCC Crc-Unit source
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/
#include <Analyzer.h>
#include "AbccCrc.h"

static const U8 abBitReverseTable16[] = 
{ 
	0x0, 0x8, 0x4, 0xC, 
	0x2, 0xA, 0x6, 0xE,
	0x1, 0x9, 0x5, 0xD, 
	0x3, 0xB, 0x7, 0xF 
};

static const U32 crc_table32[] = 
{
	0x4DBDF21CUL, 0x500AE278UL, 0x76D3D2D4UL, 0x6B64C2B0UL,
	0x3B61B38CUL, 0x26D6A3E8UL, 0x000F9344UL, 0x1DB88320UL,
	0xA005713CUL, 0xBDB26158UL, 0x9B6B51F4UL, 0x86DC4190UL,
	0xD6D930ACUL, 0xCB6E20C8UL, 0xEDB71064UL, 0xF0000000UL
};

void AbccCrc::Init()
{
	mCrc32 = 0;
	mCrc16 = 0;
}
		
void AbccCrc::Update(U8* pbBufferStart, U16 iLength)
{
	mCrc32 = CRC_Crc32(mCrc32, pbBufferStart, iLength);
	mCrc16 = CRC_Crc16(mCrc16, pbBufferStart, iLength);
}

U16 AbccCrc::Crc16()
{
	return mCrc16;
}

U32 AbccCrc::Crc32()
{
	return CRC_FormatCrc32(mCrc32);
}

U32 AbccCrc::CRC_Crc32(U32 iInitCrc, U8* pbBufferStart, U16 iLength)
{
	U8 bCrcReverseByte;
	U16 i;
	U32 lCrc = iInitCrc;
	for (i = 0; i < iLength; i++)
	{
		bCrcReverseByte = (U8)(lCrc ^ abBitReverseTable16[(*pbBufferStart >> 4) & 0xf]);
		lCrc = (lCrc >> 4) ^ crc_table32[bCrcReverseByte & 0xf];
		bCrcReverseByte = (U8)(lCrc ^ abBitReverseTable16[*pbBufferStart & 0xf]);
		lCrc = (lCrc >> 4) ^ crc_table32[bCrcReverseByte & 0xf];
		pbBufferStart++;
	}
	return lCrc;
}

U16 AbccCrc::CRC_Crc16(U16 iInitCrc, U8* pbBufferStart, U16 iLength)
{
	//TODO
	return 0;
}

U32 AbccCrc::CRC_FormatCrc32(U32 lCrc)
{
	return ((U32)abBitReverseTable16[(lCrc & 0x000000F0UL) >> 4]) |
		((U32)abBitReverseTable16[(lCrc & 0x0000000FUL)]) << 4 |
		((U32)abBitReverseTable16[(lCrc & 0x0000F000UL) >> 12] << 8) |
		((U32)abBitReverseTable16[(lCrc & 0x00000F00UL) >> 8] << 12) |
		((U32)abBitReverseTable16[(lCrc & 0x00F00000UL) >> 20] << 16) |
		((U32)abBitReverseTable16[(lCrc & 0x000F0000UL) >> 16] << 20) |
		((U32)abBitReverseTable16[(lCrc & 0xF0000000UL) >> 28] << 24) |
		((U32)abBitReverseTable16[(lCrc & 0x0F000000UL) >> 24] << 28);
}

U16 AbccCrc::CRC_FormatCrc16(U16 lCrc)
{
	//TODO
	return 0;
}


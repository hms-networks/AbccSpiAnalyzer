/******************************************************************************
**  Copyright (C) 2015-2021 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccCrc.h
**    Summary: ABCC Crc-Unit. Uses ABCC40 defined CRC algorithms.
**
*******************************************************************************
******************************************************************************/
#pragma once

#ifndef ABCC_CRC_H
#define ABCC_CRC_H

#include "Analyzer.h"

#ifndef ABCC_CRC_ENABLE_CRC16
	#define ABCC_CRC_ENABLE_CRC16	FALSE
#endif

/*
** @brief A helper class for computing the CRC32 for ABCC SPI communication.
*/
class AbccCrc
{
public:

	/*******************************************************************************
	** @brief Initialize the CRC unit's internal state.
	*/
	void Init();

	/*******************************************************************************
	** @brief Update the current CRC state with the specified data.
	**
	** @param pbBufferStart - The start of a data buffer which to continue
	**                        computing the CRC for.
	** @param iLength       - The length of the data buffer.
	*/
	void Update(U8* pbBufferStart, U16 iLength);

	/*******************************************************************************
	** @brief The currently computed CRC32.
	**
	** @return U32 - The CRC32.
	*/
	U32 Crc32();

#if ABCC_CRC_ENABLE_CRC16
	/*******************************************************************************
	** @brief The currently computed CRC16.
	**
	** @return U16 - The CRC16.
	*/
	U16 Crc16();
#endif

private:

	/*
	** @brief Internal CRC32 state.
	*/
	U32 mCrc32;

	/*******************************************************************************
	** @brief Computes and returns an unformatted CRC32.
	**
	** @param  iInitCrc      - The initial CRC32 state.
	** @param  pbBufferStart - The start of a data buffer which to continue
	**                         computing the CRC32 for.
	** @param  iLength       - The length of the data buffer.
	** @return U32           - The (unformatted) CRC32.
	*/
	U32 CRC_Crc32(U32 iInitCrc, U8* pbBufferStart, U16 iLength);

	/*******************************************************************************
	** @brief Returns the formatted CRC32.
	**
	** @param  lCrc - The unformatted CRC32.
	** @return U32  - The formatted CRC32.
	*/
	U32 CRC_FormatCrc32(U32 lCrc);

#if ABCC_CRC_ENABLE_CRC16
	/*
	** @brief Internal CRC16 state.
	*/
	U16 mCrc16;

	/*******************************************************************************
	** @brief Computes and returns a CRC16.
	**
	** @param  iInitCrc      - The initial CRC16 state.
	** @param  pbBufferStart - The start of a data buffer which to continue
	**                         computing the CRC16 for.
	** @param  iLength       - The length of the data buffer.
	** @return U16           - The CRC16.
	*/
	U16 CRC_Crc16(U16 iInitCrc, U8* pbBufferStart, U16 iLength);

	/*******************************************************************************
	** @brief Returns the formatted CRC16.
	**
	** @param  iCrc - The unformatted CRC16.
	** @return U16  - The formatted CRC16.
	*/
	U16 CRC_FormatCrc16(U16 iCrc);
#endif
};

#endif /* ABCC_CRC_H */

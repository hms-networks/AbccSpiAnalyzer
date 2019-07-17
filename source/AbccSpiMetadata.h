/******************************************************************************
**  Copyright (C) 2015-2019 HMS Industrial Networks Inc, all rights reserved
*******************************************************************************
**
**       File: AbccSpiMetadata.cpp
**    Summary: Metadata for the compiled library file
**     Author: Jon Carrier
**
*******************************************************************************
******************************************************************************/

#ifndef ABCC_SPI_METADATA_H_
#define ABCC_SPI_METADATA_H_

#ifndef FILEVERSION_YEAR
#define FILEVERSION_YEAR					2018
#endif

#ifndef FILEVERSION_MONTH
#define FILEVERSION_MONTH					10
#endif

#ifndef FILEVERSION_DAY
#define FILEVERSION_DAY						10
#endif

#ifndef FILEVERSION_BUILD
#define FILEVERSION_BUILD					1
#endif

#define FILEVERSION_XSTR(x)					#x
#define FILEVERSION_STR(x)					FILEVERSION_XSTR(x)

#define ABCC_SPI_METADATA_FILEVERSION_RAW	FILEVERSION_YEAR,FILEVERSION_MONTH,FILEVERSION_DAY,FILEVERSION_BUILD
#define ABCC_SPI_METADATA_FILEVERSION		FILEVERSION_STR( FILEVERSION_YEAR ) "," \
											FILEVERSION_STR( FILEVERSION_MONTH ) "," \
											FILEVERSION_STR( FILEVERSION_DAY ) "," \
											FILEVERSION_STR( FILEVERSION_BUILD )
#define ABCC_SPI_METADATA_COMPANYNAME		"HMS Industrial Networks, Inc."
#define ABCC_SPI_METADATA_FILEDESCRIPTION	"Anybus CompactCom SPI Protocol Analyzer for Saleae Logic"
#define ABCC_SPI_METADATA_INTERNALNAME		"AbccSpiAnalyzer"
#define ABCC_SPI_METADATA_LEGALCOPYRIGHT	"(C) 2015-2018 HMS Industrial Networks"
#define ABCC_SPI_METADATA_LEGALTRADEMARKS	"Anybus CompactCom"
#define ABCC_SPI_METADATA_ORIGINALFILENAME	"AbccSpiAnalyzer.dll"
#define ABCC_SPI_METADATA_PRODUCTNAME		"ABCC SPI Protocol Analyzer"
#define ABCC_SPI_METADATA_PRODUCTVERSION	"1.1.32"

#define METADATA_COMPANYNAME_KEY			"CompanyName"
#define METADATA_FILEDESCRIPTION_KEY		"FileDescription"
#define METADATA_FILEVERSION_KEY			"FileVersion"
#define METADATA_INTERNALNAME_KEY			"InternalName"
#define METADATA_LEGALCOPYRIGHT_KEY			"LegalCopyright"
#define METADATA_LEGALTRADEMARKS_KEY		"LegalTrademarks"
#define METADATA_ORIGINALFILENAME_KEY		"OriginalFilename"
#define METADATA_PRODUCTNAME_KEY			"ProductName"
#define METADATA_PRODUCTVERSION_KEY			"ProductVersion"

#endif /* ABCC_SPI_METADATA_H_ */

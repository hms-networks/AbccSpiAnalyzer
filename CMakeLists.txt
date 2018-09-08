cmake_minimum_required(VERSION 3.11)
enable_language(CXX RC)

########################
# Target-specific info
########################

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(PATH_SUFFIX "64")
	set(LIB_SUFFIX "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(PATH_SUFFIX "32")
	set(LIB_SUFFIX "")
endif()

if (APPLE)
	set(PATH_SUFFIX "")
	set(LIB_SUFFIX "")
	set(OS "macOS")
	set(CMAKE_COMPILE_OPTIONS "") #TODO joca
elseif(WIN32)
	set(OS "Win")
	set(CMAKE_COMPILE_OPTIONS /D_USRDLL /DABCCSPIANALYZER_EXPORTS)
elseif(UNIX AND NOT APPLE)
	set(OS "Linux")
	set(CMAKE_COMPILE_OPTIONS "") #TODO joca
endif()

project(AbccSpiAnalyzer)

########################
# Executable
########################

add_library(AbccSpiAnalyzer${LIB_SUFFIX} MODULE)

set_target_properties(AbccSpiAnalyzer${LIB_SUFFIX}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "../plugins/${OS}${PATH_SUFFIX}"
	LIBRARY_OUTPUT_DIRECTORY "../plugins/${OS}${PATH_SUFFIX}"
	RUNTIME_OUTPUT_DIRECTORY "../plugins/${OS}${PATH_SUFFIX}"
)

target_compile_options(AbccSpiAnalyzer${LIB_SUFFIX} PUBLIC ${CMAKE_COMPILE_OPTIONS})

target_sources(AbccSpiAnalyzer${LIB_SUFFIX}
	PUBLIC "source/AbccCrc.cpp"
	PUBLIC "source/AbccLogFileParser.cpp"
	PUBLIC "source/AbccSpiAnalyzer.cpp"
	PUBLIC "source/AbccSpiAnalyzerLookup.cpp"
	PUBLIC "source/AbccSpiAnalyzerResults.cpp"
	PUBLIC "source/AbccSpiAnalyzerSettings.cpp"
	PUBLIC "source/AbccSpiSimulationDataGenerator.cpp"
)

if(WIN32)
	target_sources(AbccSpiAnalyzer${LIB_SUFFIX}
		PUBLIC "project/vs2019/AbccSpiAnalyzer.rc"
	)
endif()

########################
# Include directories
########################

include_directories("source/")
include_directories("source/abcc_abp/")
include_directories("source/rapidxml-1.13/")
include_directories("sdk/release/include/")

if(WIN32)
	include_directories("project/vs2019/")
endif()

########################
# Linker settings
########################

find_library(ANALYZER_LIB NAMES Analyzer${LIB_SUFFIX} PATHS sdk/release/lib/)
target_link_libraries(AbccSpiAnalyzer${LIB_SUFFIX} ${ANALYZER_LIB})

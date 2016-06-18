# AbccSpiAnalyzer


## Copyright &copy; 2016 HMS Industrial Networks, Inc.
THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT    
WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR     
THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR     
THAT DEFECTS IN IT CAN BE CORRECTED.

#### Disclaimer
If a feature is desired or a bug has been identified comments/concerns 
are welcome to be shared to help improve this free software for the 
general public but no guarantee of adding/resolving such requests will
be made.

---

## Table of Contents
1. [Description](#description)
2. [System Requirements](#system-requirements)
3. [Installation](#installation)
	1. [Windows](#windows)
	2. [Linux](#linux)
	3. [OS X](#os-x)
4. [Documentation](#documentation)
5. [Future Improvements](#future-improvements)
6. [Change Log](#changelog)
	1. [Revision 1.0.0.0 (beta)](#revision-1000-beta)

---

## Description

With the use of the compiled plugin DLL obtained from this project, a developer
can leverage the use of a Saleae 'Logic' to analyze bi-directional ABCC SPI
communication. This analyzer-DLL parses SPI communication according to the
ABCC SPI messaging protocol. Each field within the SPI telegram is added as 
a multi-layered bubble-text within the Logic software. This means a basic
markup is displayed when zoomed-out and very detailed information when 
zoomed-in on the capture. By coupling this protocol analyzer with other 
physical signals from the target device (such as CAN, Serial, GPIO, etc.)
this tool can help solve difficult-to-diagnose issues that arise during initial
development efforts.

## System Requirements

* Saleae Logic Software (version 1.2.7)
	* Other versions will work so long as **Analyzer SDK version 1.1.32** is compatible
* PC running:
	* Windows 7 or later
	* Ubuntu 12.04.2+
	* Mac OSX 10.7 Lion+

## Installation

### Windows

To compile the project please ensure the **additional** requirements are met:

* Visual Studio 2012 or later *(retargetting project may be neccessary)*

When compiling the project, please ensure that the correct library is linked
for your Windows OS.

- If you are using **Win64**, specify `Analyzer64.lib` in your linker input.
  - The configuration manager has this setup as x64.
- If you are using **Win32**, specify `Analyzer.lib` in your linker input.
  - The configuration manager has this setup as Win32.

Once compiled, a file called `AbccSpiAnalyzer.dll` or 
`AbccSpiAnalyzer64.dll` will reside in the output folder. 
Copy this DLL to your Saleae Logic software installation in the "Analyzers" folder:

- Example: `C:\Program Files\Saleae LLC\Analyzers\AbccSpiAnalyzer.dll`

The analyzer is now ready to be used and can be found in the same way that
other protocol analyzers are added to the Logic software.

### Linux

* TODO

### OS X

* TODO

## Documentation

For information on how to get started please consult the quick start guide
included in this repository's documenation folder.

## Future Improvements

* Further expand on the options for indexed searching:
  - Index a subset of message data with an 'offset' and 'size' (or all data)
  - Index a subset of process data with an 'offset' and 'size' (or all data)
* Improve data generator (simulation) as of now it is very primative.
* Add (better) support for exporting data
* Improve 3-wire transaction alignment by including an inter-packet gap
  parameter. Without this, bad captures are likely in this mode if the
  begining of the capture takes place in the middle of a transaction.
  That said the user can use the built-in feature "Re-run starting at timing
  marker..." option.
* Support multiple instances of the analyzer plugin.
* Once software supports the message transactions concept, associate
  ABCC command and response messages as a transaction based on the source IDs.
* Support the enumeration of object specific error response information.
* Refrain from ending SPI message fragmentation until the next valid message
  is received. This will improve readablity of messages that are left in the
  message buffer.
* Add inter-sample state change detection and warn the user of possible
  jitter, signal-integrity, or baudrate settings issue.
* Improve the useage of "Error Markers".
* Include reserved/unknown/error values seen in other fields as part of error
   indexing.
* Add mechanism to detect mismatch of command message header and response
  message header when Source IDs are related.
* Support indexing option to select between verbose and compact decoded
   protocol results.
* Add support for message segmentation for the subset of objects/cmds that
   support this mechanism. This includes:
   - Write_Assembly_Data (0x10) (Asm Object)
   - Receive (0x15) (Socket Interface Object)
   - Receive_Form (0x16) (Socket Interface Object)
   - Send (0x17) (Socket Interface Object)
   - Send_To (0x18) (Socket Interface Object)
   - Set_Configuration_Data (0x11) (EIP Host Object)
   - Get_Configuration_Data (0x13) (EIP Host Object)
* Support ABCC message fragmentation that occurs before, a complete header
   is received (the ABCC profile driver does this in some cases).

## Changelog

### Revision 1.0.0.0 (beta)

* First implementation of the ABCC SPI protocol analyzer
* Supports basic messaging and error indication
* Supports CRC32 computation and indication on invalid checksum.
* Identifies fragmented SPI telegrams (not to be confussed with the ABCC SPI 
  protocol's message fragmentation or segmentation).
* Supports both 3-wire and 4-wire Full Duplex configurations. 4-wire is
  recommended for improved reliability in protcol analysis.
* Supports ABCC SPI fragmentation protocol
* Support toggle-bit monitoring for identification of retransmissions.
* Byte counter to logic-frames that are part of process data.
* Byte counter to logic-frames that are part of message data
  this will be particularly helpful for tracking long segmented messages.
* Supports bubble-text enumeration for 'most' of the following SPI telegram fields:
  - SPI control/status
  - Application status
  - Anybus status
  - LED status
  - Interrupt Mask
  - Message Object Number
  - Command Byte
  - Command Extension Word (for object/instance attributes)
* Supports filtering options to indexed decoded protocol information.
  Currently the DLL supports indexing: 
  - Message Object and Command information (only "new" messages are indexed)
  - Message Source ID (only "new" messages are indexed)
  - Anybus status
  - Application status
  - Network time
  - Protocol/Logical errors (always prefixed by '!')
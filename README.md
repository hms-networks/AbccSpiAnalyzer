# Anybus CompactCom SPI Protocol Analyzer Plugin

## Copyright &copy; 2016 HMS Industrial Networks, Inc.

THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT</br>
WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR</br>
THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR</br>
THAT DEFECTS IN IT CAN BE CORRECTED.

---

## [Table of Contents](#table-of-contents)

1. [Description](#description)
2. [System Requirements](#system-requirements)
3. [Installation](#installation)
    * [Windows](#windows)
    * [Linux](#linux)
    * [OS X](#os-x)
4. [Documentation](#documentation)
5. [Change Log](#changelog)
    * [Revision 1.0.0.0 (beta)](#revision-1000-beta)

---

## [Description](#table-of-contents)

With the use of the compiled plugin DLL obtained from this project, a developer</br>
can leverage the use of a Saleae 'Logic' to analyze bi-directional ABCC SPI</br>
communication. This analyzer-DLL parses SPI communication according to the ABCC</br>
SPI messaging protocol. Each field within the SPI telegram is added as a multi-</br>
layered bubble-text within the Logic software. This means a basic markup is</br>
displayed when zoomed-out and very detailed information when zoomed-in on the</br>
capture. By coupling this protocol analyzer with other physical signals from</br>
the target device (such as CAN, Serial, GPIO, etc.) this tool can help solve</br>
difficult-to-diagnose issues that arise during initial development efforts.

## [System Requirements](#table-of-contents)

* Saleae Logic Software (version 1.2.7)
  * Other versions will work so long as **Analyzer SDK version 1.1.32** is</br>
    compatible
* PC running:
  * Windows 7 or later
  * Ubuntu 12.04.2+
  * Mac OSX 10.7 Lion+

## [Installation](#table-of-contents)

### [Windows](#table-of-contents)

To compile the project please ensure the **additional** requirements are met:

* Visual Studio 2012 or later *(retargetting project may be necessary)*

When compiling the project, please ensure that the correct library is linked</br>
for your Windows OS.

* If you are using **Win64**, specify `Analyzer64.lib` in your linker input.
  * The configuration manager has this setup as x64.
* If you are using **Win32**, specify `Analyzer.lib` in your linker input.
  * The configuration manager has this setup as Win32.

Once compiled, a file called `AbccSpiAnalyzer.dll` or `AbccSpiAnalyzer64.dll`</br>
will reside in the output folder. Copy this DLL to your Saleae Logic software</br>
installation in the "Analyzers" folder:

* Example: `C:\Program Files\Saleae LLC\Analyzers\AbccSpiAnalyzer.dll`

The analyzer is now ready to be used and can be found in the same way that</br>
other protocol analyzers are added to the Logic software.

### [Linux](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script. This script will</br>
  compile the appropriate shared object library based on the host system.

Once compiled, a file called `AbccSpiAnalyzer.so` or `AbccSpiAnalyzer64.so`</br>
will reside in the either `./plugins/Linux/` or `./plugins/Linux64` folder,</br>
respectively. Copy this shared object to your Saleae Logic software</br>
installation in the "Analyzers" folder.

### [OS X](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script.

Once compiled, a dynamic library called `AbccSpiAnalyzer.dylib` will reside</br>
in the `./plugins/OSX/` folder. Copy this dynamic object to your Saleae Logic</br>
software installation in the "Analyzers" folder.

## [Documentation](#table-of-contents)

For information on how to get started please consult the quick start guide</br>
included in this repository's documentation folder.

## [Changelog](#table-of-contents)

### [Revision 1.0.0.0 (beta)](#table-of-contents)

* First implementation of the ABCC SPI protocol analyzer
* Supports basic messaging and error indication
* Supports CRC32 computation and indication on invalid checksum.
* Identifies fragmented SPI telegrams (not to be confused with the ABCC SPI</br>
  protocol's message fragmentation or segmentation).
* Supports both 3-wire and 4-wire configurations.
* Supports ABCC SPI fragmentation protocol
  * Fragmentation starting at the message header is currently not supported (PLANNED)
* Support toggle-bit monitoring for identification of retransmissions.
* Byte counter to logic-frames that are part of process data.
* Byte counter to logic-frames that are part of message data this will be</br>
  particularly helpful for tracking long segmented messages.
* Markup to aid in distinguishing valid message data from invalid message data.
* Supports bubble-text enumeration for 'most' of the following SPI telegram</br>
  fields:
  * SPI control/status
  * Application status
  * Anybus status
  * LED status
  * Interrupt Mask
  * Message Object Number
  * Command Byte
  * Command Extension Word (for object/instance attributes)
  * Error Response Bytes (including some object specific codes)
* Supports filtering options to indexed decoded protocol information. Currently</br>
  the plugin supports indexing:
  * Message Object and Command information (only "new" messages are indexed)
  * Message Source ID (only "new" messages are indexed)
  * Anybus status
  * Application status
  * Network time
  * Protocol/Logical errors (always prefixed by '!')

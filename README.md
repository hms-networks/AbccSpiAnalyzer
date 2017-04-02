# Anybus CompactCom SPI Protocol Analyzer Plugin

## Copyright &copy; 2015-2017 HMS Industrial Networks, Inc.

THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT
WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR
THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR
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
5. [Changelog](#changelog)

---

## [Description](#table-of-contents)

This plugin in conjunction with the Saleae 'Logic' hardware and software provides
an easy-to-use interface for analyzing bi-directional (full-duplex) ABCC SPI
communication. This analyzer plugin parses SPI communication according to the
ABCC SPI protocol. Each field within an SPI telegram is added as a multi-layered
bubble-text within the Logic software. This means basic markup is displayed
when zoomed-out and very detailed information when zoomed-in on the capture.
By coupling this protocol analyzer with other physical signals from the target
device (such as CAN, Serial, GPIO, etc.) this tool can help solve
difficult-to-diagnose issues that arise during initial development efforts. In
spirit, this plugin is intended as an educational tool that equips developers
with an accessible interface into host and module communication following the
SPI communication thus allowing them to observe the real world startup and
operational behavior of a device communicating with the ABCC over the SPI
protocol.

![Overview of Plugin](https://github.com/HMSAB/AbccSpiAnalyzer/wiki/overview.gif "Overview of Plugin")

## [System Requirements](#table-of-contents)

* Saleae Logic Software (version 1.2.7)
  * Other versions will work so long as **Analyzer SDK version 1.1.32** is
    compatible
* PC running:
  * Windows 7 or later
  * Ubuntu 12.04.2+
  * Mac OSX 10.7 Lion+

## [Installation](#table-of-contents)

### [Windows](#table-of-contents)

To compile the project please ensure the **additional** requirements are met:

* Visual Studio 2012 or later *(retargetting project may be necessary)*

When compiling the project, please ensure that the correct library is linked
for your Windows OS.

* If you are using **Win64**, specify `Analyzer64.lib` in your linker input.
  * The configuration manager has this setup as x64.
* If you are using **Win32**, specify `Analyzer.lib` in your linker input.
  * The configuration manager has this setup as Win32.

Once compiled, a file called `AbccSpiAnalyzer.dll` or `AbccSpiAnalyzer64.dll`
in the either ./plugins/Win32/ or ./plugins/Win64 folder, respectively. Copy
this DLL to your Saleae Logic software installation in the "Analyzers" folder:

* Example: `C:\Program Files\Saleae LLC\Analyzers\AbccSpiAnalyzer.dll`

The analyzer is now ready to be used and can be found in the same way that
other protocol analyzers are added to the Logic software.

### [Linux](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script. This script will
  compile the appropriate shared object library based on the host system.

Once compiled, a file called `AbccSpiAnalyzer.so` or `AbccSpiAnalyzer64.so`
will reside in the either `./plugins/Linux/` or `./plugins/Linux64` folder,
respectively. Copy this shared object to your Saleae Logic software
installation in the "Analyzers" folder.

### [OS X](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script.

Once compiled, a dynamic library called `AbccSpiAnalyzer.dylib` will reside
in the `./plugins/OSX/` folder. Copy this dynamic object to your Saleae Logic
software installation in the "Analyzers" folder.

## [Documentation](#table-of-contents)

For complete details regarding this plugin's functionality please consult the
[Quick Start Guide](doc/AbccSpiAnalyzer_Plugin_QSG.pdf) included in this repository's documentation folder.

## [Changelog](#table-of-contents)

Please see [CHANGELOG.md](CHANGELOG.md) provided within this repository for details on the
changelog history.

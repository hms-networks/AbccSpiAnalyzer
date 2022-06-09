# Anybus CompactCom SPI Protocol Analyzer Plugin

[![windows_build](https://github.com/hms-networks/AbccSpiAnalyzer/actions/workflows/windows_build.yml/badge.svg)](https://github.com/hms-networks/AbccSpiAnalyzer/actions/workflows/windows_build.yml)
[![ubuntu_build](https://github.com/hms-networks/AbccSpiAnalyzer/actions/workflows/ubuntu_build.yml/badge.svg)](https://github.com/hms-networks/AbccSpiAnalyzer/actions/workflows/ubuntu_build.yml)
[![macos_build](https://github.com/hms-networks/AbccSpiAnalyzer/actions/workflows/macos_build.yml/badge.svg)](https://github.com/hms-networks/AbccSpiAnalyzer/actions/workflows/macos_build.yml)

## Copyright &copy; 2015-2022 HMS Industrial Networks, Inc.

THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. HMS DOES NOT
WARRANT THAT THE FUNCTIONS OF THE CODE WILL MEET YOUR REQUIREMENTS, OR
THAT THE OPERATION OF THE CODE WILL BE UNINTERRUPTED OR ERROR-FREE, OR
THAT DEFECTS IN IT CAN BE CORRECTED.

---

## [Table of Contents](#table-of-contents)

1. [Description](#description)
2. [System Requirements](#system-requirements)
3. [Precompiled Releases](#precompiled-releases)
4. [Compile & Install](#compile-&-install)
   * [Initial Steps](#initial-steps)
   * [Windows](#windows)
   * [GNU/Linux](#gnulinux)
   * [macOS](#macos)
5. [Generating Releases](#generating-releases)
6. [Documentation](#documentation)
7. [Changelog](#changelog)
8. [Licenses](#licenses)

---

## [Description](#table-of-contents)

This plugin, in conjunction with the [Saleae Logic][link_saleae] hardware and
software, provides an easy-to-use interface for analyzing bi-directional (full-duplex)
[Anybus CompactCom (ABCC)](https://www.anybus.com/products/embedded-index) SPI
protocol communication.

Each field within an SPI packet is added as a multi-layered bubble-text
within the Logic software. Basic markup is displayed when zoomed-out, while
more detailed information is displayed when zoomed-in.

The decoded results are indexed and searchable within the Logic software; for
even more detail, the user can export the message data or process data to a CSV
file to be imported into Excel (or similar) for further analysis. For instance,
this functionality may reduce the effort required to extract file data
transferred from ABCC object messaging, or to plot a waveform extracted from
process data using either local timestamp information or the network timestamps
(if supported by the network protocol).

![Overview of Plugin][mov_overview]

## [System Requirements](#table-of-contents)

* Saleae Logic Software
  * **Recommended Version**: 1.2.29
  * Other versions will work so long as **Analyzer SDK version 1.1.32** is
    compatible.
  * **NOTE**: At this time, Saleae Logic V2 is not supported.
* Saleae Logic Hardware
  * While any of Saleae Logic hardware offerings is expected to be compatible,
  it is important to consider sample rate limitations of the Logic hardware and
  what SPI data rate the ABCC host-module communication is using. The absolute
  minimum requirement for sufficiently sampling the communication is 2x the
  frequency of the SPI SCLK. The ABCC is able to work over a wide range of
  clock rates where the maximum is specified at 20MHz. So to be able to support
  this would require at a minimum the ability to sample at 40MSamples/second.
  Keep in mind that these analyzers may reduce sample rate depending on how many
  channels on the analyzer are active, so it is imperative that the hardware
  is able to sufficiently sample the SPI bus for the number of channels required.
* PC running:
  * Windows 7 through Windows 10
    * NOTE: At this time Window 11 has compatibility issues with version 1.2.29.
      Version 1.2.40 resolves this compatibility issue but Saleae dropped
      support for the decoded protocols subwindow for this version. An
      alternative solution for using 1.2.29 on Windows 11 would be to use `WSLg`
      and attach the Saleae Logic interface via `usbpid.exe`.
      See [How-To: Connect USB devices](https://docs.microsoft.com/en-us/windows/wsl/connect-usb).
  * Ubuntu 12.04.2+
  * Mac OSX 10.7 Lion+
* USB 2.0 or better
  * Using the Saleae Logic on a dedicated USB controller that matches or exceeds
  the Logic's supported USB version is recommended for ensuring the best
  possible sampling performance.

## [Precompiled Releases](#table-of-contents)

Precompiled plugins and documentation are available here: [Plugin Releases][link_releases]

## [Compile & Install](#table-of-contents)

While tagged releases are provided which contain pre-compiled libraries and
associated documentation, the user may still opt to compile the libraries
for themselves.

### [Initial Steps](#table-of-contents)

After cloning this git repository, the user will need to initialize the
submodules command in order to fetch the repositories containing the Logic
Analyzer SDK. Two copies are used of the same repo, one pointed at the
release/master branch and another pointing at the legacy SDK to support debugging.

Example for cloning the repository and fetching/initializing the submodules:

```bash
git clone https://github.com/hms-networks/AbccSpiAnalyzer.git
cd AbccSpiAnalyzer
git submodule update --init --recursive
```

### [Windows](#table-of-contents)

To compile the project please ensure the **additional** requirements are met:

> DEPENDENCIES: **Visual Studio 2019**, other versions may also work
*(project retargeting may be necessary)*

> NOTE: **Express** or **Community** versions of Visual Studio may require
additional steps to replace occurrences of `#include "afxres.h"` with
`#include "windows.h"` or installation of the Visual C++ MFC package (this can
be done by modifying the Visual Studio installation and enabling the associated
feature).

When compiling the project, please ensure that the correct library is linked
for the Windows OS being used. The VS solutions provided are setup with the
expectation that the host system will be x64. The solutions are also setup for
batch build such that an x64 system can compile all supported build configurations
with one request. To access this, right-click the **Solution 'AbccSpiAnalyzer'**
item in the **Solution Explorer** subwindow to access the context menu and select
**Batch Build...**.

#### Cross-compile for GNU/Linux Operating Systems

Also included in the repository is a simple batch file that can be used to quickly
build both the Windows and GNU/Linux libraries via the use of Windows 10 WSL
(Windows Subsystem for Linux). Cross-compiling for GNU/Linux requires the same
set of dependencies documented below for GNU/Linux.

#### Custom Visual Studio Projects

If there is a need to create a new Visual Studio project, ensure the linker is
configured to include the correct library:

* If using **Win64**, specify `./sdk/release/Analyzer64.lib` in the linker input.
  * The configuration manager has this setup as x64.
* If using **Win32**, specify `./sdk/release/Analyzer.lib` in the linker input.
  * The configuration manager has this setup as Win32.

Once compiled, a file called `AbccSpiAnalyzer.dll` or `AbccSpiAnalyzer64.dll`
in the either `./plugins/Win32/` or `./plugins/Win64` folder, respectively. Copy
this DLL to the user's Saleae Logic software installation in the "Analyzers" folder:

* Example: `C:\Program Files\Saleae LLC\Analyzers\AbccSpiAnalyzer.dll`

The analyzer is now ready to be used and can be found in the same way that
other protocol analyzers are added to the Logic software.

### [GNU/Linux](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script. This script will
  compile the appropriate shared object library based on the host system.

> DEPENDENCIES: **Python**, **G++**

> ADDITIONAL DEPENDENCY: Depending on the system it may also be required to
install `g++-multilib` in order for the python build script to complete without
error. This is so a 64-bit system can cross-compile for 32-bit machines.

Once compiled, a file called `AbccSpiAnalyzer.so` or `AbccSpiAnalyzer64.so`
will reside in the either `./plugins/Linux/` or `./plugins/Linux64` folder,
respectively. Copy this shared object to the user's Saleae Logic software
installation in the "Analyzers" folder.

### [macOS](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script.

> DEPENDENCIES: **Python**, **G++**

Once compiled, a dynamic library called `AbccSpiAnalyzer.dylib` will reside
in the `./plugins/OSX/` folder. Copy this dynamic object to the user's Saleae
Logic software installation in the "Analyzers" folder.

### [Generating Releases](#table-of-contents)

This section is not typically applicable for most users, but is documented here
for completeness. The python script `make_release.py` will generate the ZIP file
used for releases. However to use this without error a user must collect the
compiled library files for each platform. On a Windows 64-bit host with Ubuntu
bash on Windows using WSL (Windows Subsystem for Linux), it is possible to
generate all required library files with the exception to macOS using
`cross_compile.bat`. Obtaining the macOS dynamic library is a separate manual
step, transferring the dylib from the macOS build host to the Windows host.

Windows:

```bash
py.exe -3 -m pip install -r py_requirements.txt
py.exe -3 ./make_release.py
```

macOS/Linux:

```bash
python3 -m pip install -r py_requirements.txt
python3 ./make_release.py
```

## [Documentation](#table-of-contents)

For complete details regarding this plugin's functionality please consult the
[Wiki][link_wiki] found within this repository or [Quick Start Guide][link_qsg]
included in this repository's documentation folder. The Wiki is expected to
contain more up-to-date details on the plugin's functionality in reference to
the master branch whereas the PDF document will typically be updated when a new
tagged release is made and is provided as part of the tagged release's
downloadable assets.

For details on the Logic SDK's API please refer to the Saleae's
[Protocol Analyzer SDK][link_sdk] page.

## [Changelog](#table-of-contents)

Please see [CHANGELOG.md](CHANGELOG.md) provided within this repository for details on the changelog history.

## [Licenses](#table-of-contents)

Please see [LICENSE.md](LICENSE.md) provided within this repository for details on the software licensing.

[mov_overview]: https://github.com/HMSAB/AbccSpiAnalyzer/wiki/overview.gif "Overview of Plugin"
[link_wiki]: https://github.com/HMSAB/AbccSpiAnalyzer/wiki
[link_releases]: https://github.com/HMSAB/AbccSpiAnalyzer/releases
[link_qsg]: doc/AbccSpiAnalyzer_Plugin_Quick_Start_Guide.pdf
[link_sdk]: https://support.saleae.com/saleae-api-and-sdk/protocol-analyzer-sdk
[link_saleae]: https://www.saleae.com/

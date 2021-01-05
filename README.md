# Anybus CompactCom SPI Protocol Analyzer Plugin

## Copyright &copy; 2015-2021 HMS Industrial Networks, Inc.

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
5. [Documentation](#documentation)
6. [Changelog](#changelog)
7. [Licenses](#licenses)

---

## [Description](#table-of-contents)

This plugin, in conjunction with the [Saleae Logic][link_saleae] hardware and
software, provides an easy-to-use interface for analyzing bi-directional (full-duplex)
[Anybus CompactCom (ABCC)](https://www.anybus.com/products/embedded-index) SPI
protocol communication.

Each field within an SPI packet is added as a multi-layered bubble-text
within the Logic software. Basic markup is displayed when zoomed-out, while
more detailed information is displayed when zoomed-in.

The decoded results are indexed and searchable within the Logic software; and
for even more detail, the user can export the message data or process data
to a CSV file to be imported into Excel (or similar) for further analysis. This
functionality makes it a fairly straight-forward task to extract file transfer
data from ABCC object messaging, or to plot a waveform extracted from the
process data data field with accurate local timestamp information and
(if supported by the network) the network time information.

![Overview of Plugin][mov_overview]

## [System Requirements](#table-of-contents)

* Saleae Logic Software
  * __Recommended Version__: 1.2.29
  * Other versions will work so long as **Analyzer SDK version 1.1.32** is
    compatible.
  * __NOTE__: At this time, Saleae Logic V2 is not supported.
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
  * Windows 7 or later
  * Ubuntu 12.04.2+
  * Mac OSX 10.7 Lion+
* USB 2.0 or 3.0 port
  * Using the Saleae Logic on a dedicated USB controller is recommended for
  ensuring the best sampling performance of the hardware will be possible.

## [Precompiled Releases](#table-of-contents)

Precompiled plugins and documentation are available here: [Plugin Releases][link_releases]

## [Compile & Install](#table-of-contents)

While tagged releases are provided which contain pre-compiled libraries and
associated documentation, you may still opt to compile the libraries yourself.

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

> DEPENDENCIES: **Visual Studio 2012** or later *(retargeting project may be necessary)*

> NOTE: **Express** or **Community** versions of Visual Studio may require
additional steps to replace occurrences of `#include "afxres.h"` with `#include "windows.h"`.

When compiling the project, please ensure that the correct library is linked
for your Windows OS. The VS solutions provided are setup with the expectation
that the host system will be x64. The solutions are also setup for batch build
such that an x64 system can compile all supported build configurations with
one request. To access this, right-click the **Solution 'AbccSpiAnalyzer'** item
in the **Solution Explorer** subwindow to access the context menu and select
**Batch Build...**.

#### Cross-compile for GNU/Linux Operating Systems

Also included in the repository is a simple batch file that can be used to quickly
launch an instance of Windows 10 WSL (Windows Subsystem for Linux) and then launch
the build_analyzer.py script. This can be included as part of the post/pre-build
process in a Visual Studio project to support automatic cross-compilation for
GNU/Linux. This functionality requires the same set of dependencies documented below
for GNU/Linux.

#### Custom Visual Studio Projects

If you decide to make your own Visual Studio project (perhaps to use some other
version), ensure that you configure the linker to correctly select the proper
library

* If you are using **Win64**, specify `./sdk/release/Analyzer64.lib` in your linker input.
  * The configuration manager has this setup as x64.
* If you are using **Win32**, specify `./sdk/release/Analyzer.lib` in your linker input.
  * The configuration manager has this setup as Win32.

Once compiled, a file called `AbccSpiAnalyzer.dll` or `AbccSpiAnalyzer64.dll`
in the either `./plugins/Win32/` or `./plugins/Win64` folder, respectively. Copy
this DLL to your Saleae Logic software installation in the "Analyzers" folder:

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
respectively. Copy this shared object to your Saleae Logic software
installation in the "Analyzers" folder.

### [macOS](#table-of-contents)

* To compile simply run the `build_analyzer.py` python script.

> DEPENDENCIES: **Python**, **G++**

Once compiled, a dynamic library called `AbccSpiAnalyzer.dylib` will reside
in the `./plugins/OSX/` folder. Copy this dynamic object to your Saleae Logic
software installation in the "Analyzers" folder.

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

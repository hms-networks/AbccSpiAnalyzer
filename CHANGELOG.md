# Changelog

## Version 2021.1.5.1

### Changes

* Code refactoring
* Updated lookup tables to support latest ABP additions.
* Changed channel names to more closely match ABCC documentation.

### Fixes

* Resolved static analysis issues.


## Version 2019.07.17.1

### Changes

* Updated lookup tables to contain latest ABP header information.
* Migrated from VS2017 to VS2019
* Other minor changes & refactoring

### Fixes

* (Issue #23) Fixed the state logic when the enable-signal toggles and there is
  no clocked data.
* (Issue #24) Fixed the logic behind multi-event markers.
* Fixed build issue for macOS Mojave.

## Version 2018.10.10.1

### Changes

* Significant internal code refactoring
* Updated lookup tables to contain latest ABP header information.
* Added support for network specific error codes for Profinet Diagnostic object
* Significant rework of the plugin's simulation was done to improve highlighting
  the functionality of the plugin. The simulation now performs a more complex
  request-response messaging sequence that targets reading file information from
  the plugin's metadata for the compiled DLL. This doubles to provide built-in
  version information into the plugin for operating systems that do not contain
  such metadata for library objects.
* Significant rework of the CSV file exporting options for ABCC message data and
  process data. These exported files now include state information of the network
  and application. The CSV for message data now provides enumerated results
  making it much more human readable. Error event indicators are also now included
  to help point out CRC error, fragmented SPI packets, and retransmitted packets.
* Changed how the "display base" is used in the plugin. Now, only fields that are
  known to be of a 'character' base-type will be permitted to be displayed as
  "ASCII" or "ASCII & HEX". Frames that are not a "CHAR" base-type will be
  displayed in HEX for both of these options. In general, this means the ASCII
  display options will only apply to the "Message Data" field. This should provide
  a more intuitive user-experience in interpreting the information contained in
  the packet as a whole. Both attribute and command message data have been
  updated to follow these rules. For instance, the File System Interface's
  File_Open message data contains a filename that is naturally understood as a
  string which is classified as a 'character' base-type; whereas the "File Size"
  attribute is naturally understood as a 'numeric' data field. For any field,
  where it may be desirable to permit display in ASCII the BaseType::Character
  should be used. Future adjustments may be made to a particular command or
  attribute BaseType.
* Removed network types from settings that are ABCC 30-series only.
* This changelog has been modified to remove the "known issues" section. These
  are likely to remain as limitations of the software for some time and a separate
  document has been added to track these.
* Added option to limit the number of "clocking alerts" to report to the UI.
  This can be accessed from the advanced settings XML file.

### Fixes

* Fixed lookup logic for the "Object" instance name. Previously, instance 0 was
  being reported as "Unknown" instead of "Object".
* Fixed "compact" indexing MOSI tag. The MOSI channels indexed results were
  mistakenly using the MISO_TAG_STR instead of MOSI_TAG_STR.
* Fixed issue with last packet not being committed. The last packet in a capture
  was not be correctly committed meaning there would be missing information in
  the results. This would affect results observed from both the "chipselect
  marker" behavior along with the exported CSV files.
* Fixed an issue with how extra "clocked data" is detected at the end of an
  SPI transaction.
* Fixed various issues with how advanced settings were being handled.

## Version 2018.1.9.1

### Changes

* Switched plugin's versioning to year.month.day.build notation
* Updated lookup tables to include latest ABP headers including new support for
  OPCUA, PNAM, EME, etc.
* Added advanced settings XML-file option. This file contains additional options
  that may be useful in special cases. Currently, two options are available:
  "4-wire-on-3-channels" and "3-wire-on-4-channels". "4-wire-on-3-channels" can
  be useful when limited on logic analyzer channels and would like to omit the
  SPI enable (chip/slave select) signal and use that channel for some other purpose.
  This provides added flexibility over normal 3-wire analysis since it will ignore
  timing requirements and permit "clock-idle-low" configurations (which the
  ABCC does not support in 3-wire mode and consequently this analyzer).
  "3-wire-on-4-channels" is useful when there is an extra channel on the analyzer
  that could be used to gain the analyzer "markers" that are normally drawn on the
  SPI enable channel. Using this mode will continue to treat the SPI channels in
  3-wire mode and enforcing timing requirements and clock-idle-high configuration.
* Added network specific instance names (i.e. Network configuration object instances)
* Changed decoded result's command extension, source ID, and instance to always
  use a combined decimal and hex notation (when no name is found) for consistency.
* Other minor changes

### Fixes

* Last packet in capture may be missing a frame that will result in an
  uncommitted packet. This means that when exporting messages to a CSV, the last
  packet may be lost.
* Resolved various cases where the plugin would crash the software.

## Revision 1.0.0.0 (RC1)

### Changes

* Data Exporting Options
  * Export individual (all) frame data
    * Each SPI field is exported individually within an CSV file
    * Provides most flexibility in acquisition of raw SPI packet data
    * `NOTE`: Frame data that lacks a valid packet identifier is understood to
      be part of an errored packet.
  * Export message data
    * Only message data within a valid packet will be exported
    * Best option when the only interest is object messaging
  * Export process data
    * On MOSI, only packets that indicate PD_VALID will be exported
    * On MISO, only packets that indicate NEW_PD will be exported
  * Export XML
    * All data is exported in XML format
    * Uses a human readable hierarchical structure to present ABCC SPI packet
      information

## Revision 1.0.0.0 (Beta)

### Changes

* First implementation of the ABCC SPI protocol analyzer
* Supports basic messaging and error indication
* Supports CRC32 computation and indication on invalid checksum.
* Identifies fragmented SPI telegrams (not to be confused with the ABCC SPI
  protocol's message fragmentation or segmentation).
* Supports both 3-wire and 4-wire configurations.
* Supports ABCC SPI fragmentation protocol
  * `NOTE` Fragmentation starting at the message header is currently not
  supported (PLANNED)
* Support toggle-bit monitoring for identification of retransmissions.
* Byte counter to logic-frames that are part of process data.
* Byte counter to logic-frames that are part of message data this will be
  particularly helpful for tracking long segmented messages.
* Markup to aid in distinguishing valid message data from invalid message data.
* Supports bubble-text enumeration for 'most' of the following SPI telegram
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
* Supports filtering options to indexed decoded protocol information. Currently
  the plugin supports indexing:
  * Message Object and Command information (only "new" messages are indexed)
  * Message Source ID (only "new" messages are indexed)
  * Anybus status
  * Application status
  * Network time
  * Protocol/Logical errors (always prefixed by '!')

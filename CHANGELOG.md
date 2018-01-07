# Changelog

## Version 2018.1.9.1

### Known Issues

* A capture that starts in the middle of message fragmentation may result in a
  series of packets being incorrectly interpreted and possibly marked as invalid.
  To workaround this, the user can place a marker past this fragmentation and
  instruct the plugin to start analysis after this point.
* Due to a bug in the Saleae Logic software, the decoded results that indicate
  the packet ID may occasionally display an invalid value of 0xFFFFFFFFFFFFFFFF
  instead of the packet's actual ID.
* Settings window's "Advanced Settings" option is suppose to have a browse
  filesystem button. Saleae has acknowledged that this interface was never
  fully developed and should be resolved in a future release.
* Error markers are not given display priority. This is a limitation of the
  Logic software itself.

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

### Known Issues

* Last packet in capture may be missing a frame that will result in an
  uncommitted packet. This means that when exporting messages to a CSV, the last
  packet may be lost.
* A message's Source ID and Instance tabular text entries do not respect display
  format settings while command extension does. A change should be made to improve
  consistency.
* Opening multiple saved captures will often lead to the currently opened Logic
  windows to crash.
* Assigning severely misbehaving logic signals to the analyzer plugin may result
  in an application crash.
* A capture that starts in the middle of message fragmentation may result in a
  series of packets being incorrectly interpreted and possibly marked as invalid.
  To workaround this, the user can place a marker past this fragmentation and
  instruct the plugin to start analysis after this point.

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

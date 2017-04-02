# Changelog

## Revision 1.0.0.0 (RC1)

### Known Issues

* Last packet in capture may be missing a frame that will result in an
  uncomitted packet. This means that when exporting messages to a CSV, the last
  packet may be lost.
* A message's Source ID and Instance tabular text entries do not respect display
  format settings.
* Opening multiple saved captures will often lead to the currently opened Logic
  windows to crash.
* Assigning severly misbehaving logic signals to the analyzer plugin may result
  in an application crash.
* A capture that starts in the middle of message fragmentation may result in a
  series of packets being incorrectly interpeted and possibly marked as invalid.
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

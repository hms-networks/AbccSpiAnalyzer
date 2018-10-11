### Known Issues/Limitations

Below are a list of issues/Limitations that may be encountered that are dependent on the
Logic software and cannot be resolved by the plugin.

As of Plugin Version 2018.1.9.1:

* Additional Details:
  * Logic software version 1.2.10
  * Logic SDK version 1.1.32
* Issue(s):
  * Settings window's "Advanced Settings" option is suppose to have a browse
    filesystem button. Saleae has acknowledged that this interface was never
    fully developed and should be resolved in a future release.

As of Plugin Revision 1.0.0.0 (RC1):

* Additional Details:
  * Logic software version 1.2.10
  * Logic SDK version 1.1.32
* Issue(s):
  * Due to a bug in the Saleae Logic software, the decoded results that indicate
    the packet ID may occasionally display an invalid value of 0xFFFFFFFFFFFFFFFF
    instead of the packet's actual ID.
  * Error markers are not given display priority. This is a limitation of the
    Logic software itself.

Below are a list of issues/Limitations that may be encountered that could be
resolved by the plugin but would require substantial effort to fix and do not
currently have any plan or priority to resolve.

As of Plugin Revision 1.0.0.0 (RC1):

* Issue(s):
  * A capture that starts in the middle of message fragmentation may result in a
    series of packets being incorrectly interpreted and possibly marked as invalid.
    To workaround this, the user can place a marker past this fragmentation and
    instruct the plugin to start analysis after this point.

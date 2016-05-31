# README #

statsd-proxy, a proxy server that will listen on a configured UDP port (default: 8272) binding to all ip's (0.0.0.0) on the machine.

* Quick summary

Will perform the following operations

* Will intercept all traffic on this port and parse the "c" or count operations.
* After parsing will make a key-value pair where the key will be the property and value will be the counter.
* This key-value (property-counter) will then be published in a redis server running on that machine [TODO: make it configurable from the conf file]
* After this we will pass this request as is to the configured statsd server in the backend.

* Version
0.1

* Admin

Tapas Sharma

tapas.bits@gmail.com
---
layout: main
---
# STATSD_PROXY # {#statsd_proxy-#}

statsd-proxy, a proxy server that listens on a configured UDP port (default: 8272) forwards the requests to statsd server and publishes data to Redis.

# Quick summary {#quick-summary}

The proxy is capabale of performaing the following functions

* Will intercept all traffic on the configured port and parse the "c" or count operations.
+ Other operations are currently not supported, but its in the TODO
* After parsing we will make a key-value pair where the key will be the property and value will be the counter.
* This key-value (property-counter) will then be published to a redis server
* After this we will pass this request as is to the configured statsd server in the backend.

## Dependencies {#dependencies}

For communication with the redis server, we use the standard **hiredis** that can be found here [hiredis github](https://github.com/redis/hiredis)
Other dependices include
* glibc
* pthreads
* hiredis
* redi.sh

## Compiling {#compiling}

To compile the code use the provided make file
~~~~
cd statsd-proxy
make
~~~~
Before running you might need to give permission to the following path, for the user you are running the server as
~~~~
/var/log/
~~~~
The server create a log file under this path with the name
~~~~
/var/log/statsd_proxy.log
~~~~
After compiling the server, and setting the log file you can run the server as
~~~~
bin/statsd_porxy [conf_file]
~~~~


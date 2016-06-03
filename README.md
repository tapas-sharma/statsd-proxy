# README #

statsd-proxy, a proxy server that listens on a configured UDP port (default: 8272) forwards the requests to statsd server and publishes data to Redis.
* Quick summary

The proxy is capabale of performaing the following functions

* Will intercept all traffic on the configured port and parse the "c" or count operations.
+ Other operations are currently not supported, but its in the TODO
* After parsing we will make a key-value pair where the key will be the property and value will be the counter.
* This key-value (property-counter) will then be published to a redis server
* After this we will pass this request as is to the configured statsd server in the backend.

## Dependencies

For communication with the redis server, we use the standard **hiredis** that can be found here [hiredis github](https://github.com/redis/hiredis)
Other dependices include
* glibc
* pthreads
* hiredis

## Compiling

To compile the code use the provided make file
~~~~
cd statsd-proxy
make
~~~~
This will create a bin folder under which there is the server. To run you can do
~~~~
bin/statsd_porxy [conf_file]
~~~~
Configuration file is optional, if not provided the server assumes everything is running locally and on the default ports.
        
## Configuration

There is a sample configuration file, in the conf folder called __statsd_proxy.conf.example__, the configuration properties are self explanatory

~~~~
            # Define the proxy properties here
            #
            proxy_port = 8272
            proxy_ip=localhost
            #
            # To deamonize the proxy write 1 else 0
            # default = 1
            daemonize= 1
            #
            #Log levels, there are 3
            # 1 - INFO
            # 2 - INFO + ERROR
            # 3 - DEBUG
            #default 1
            # log file is in /var/log/startd_proxy.log
            log_level = 1
            #Statds server information
            statsd_server_ip=127.0.0.1
            statsd_server_port = 8125
            #redis server information
            redis_server_ip=localhost
            redis_server_port=6379
~~~~

# Version
0.1

# Contributor(s)

[Tapas Sharma](mailto:tapas.bits@gmail.com)
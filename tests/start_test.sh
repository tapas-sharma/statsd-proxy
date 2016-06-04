#!/bin/bash
#Author: Tapas Sharma
#Version: 0.1
#Used for basic testing of the proxy


PROXY_STATSD_HOST="localhost"
PROXY_STATSD_PORT=8272
REDIS_HOST=127.0.0.1
REDIS_PORT=6379
WAIT_BETWEEN_COMMAND=2
STATDS_OUT="statsd_server.out"
echo "Deleting $STATSD_OUT"
rm -rf $STATSD_OUT
#stop any statsd_proxy server running
pkill -INT statsd_proxy
PIDS=`ps -ef | grep statsd_proxy | grep -v grep | awk '{if ($6=="?") print $2}'`
for p in PIDS
do
    kill -INT $p
done
echo "Compiling the proxy server"
cd ../
make
cd tests
#Start the statsd_proxy using the configuration in 
#test foler, and in daemon mode and debug mode, we will then send
#and look for that in the log file
echo "starting the statsd_proxy..."
../bin/statsd_proxy test.conf
PID=`ps -ef | grep statsd_proxy | grep -v grep | awk '{if ($6=="?") print $2}'`
sleep $WAIT_BETWEEN_COMMAND
echo "Getting the current data from the redis server"
data=`./redi.sh -g statsd_proxy.count -H $REDIS_HOST -P $REDIS_PORT`
echo "Got $data, will increment it by 1"
#start the statsd_server
#assumption is that its installed in /opt/statsd/bin/
#using the configuration.js in the test folder
# the output backend is console in this configuration
#we are grepping for debug since we are just interesetd in the messages
echo "starting the statsd server..."
/opt/statsd/bin/statsd  configuration.js  &> statsd_server.out &
sleep $WAIT_BETWEEN_COMMAND
echo "Sending data..."
#sending test_data
echo -e "statsd_proxy.count:1|c@0.1\nstatsd_proxy.timer:2|ms" | nc -u -w 1 $PROXY_STATSD_HOST $PROXY_STATSD_PORT
# check in Redis for the test data being preset
# we will be using crypt1d's redis.sh from the
# official client's page of redis.
incr_data=`./redi.sh -g statsd_proxy.count -H $REDIS_HOST -P $REDIS_PORT`
err_data=`./redi.sh -g statsd_proxy.timer -H $REDIS_HOST -P $REDIS_PORT 2>&1 /dev/null`
if [ $incr_data > $data ]
then
    echo "PASS"
else
    echo "FAIL"
fi
rm -rf $data
echo "Stopping any servers we started for the test"
PIDS=`ps -ef |grep statsd | grep -v grep | awk '{if ($8=="statsd") print $2}'`
for p in $PIDS
do
    kill -INT $p
done
kill -INT $PID

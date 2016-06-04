/*******************************************************
 * Author  : Tapas Sharma
 * Email   : tapas.bits@gmail.com
 * Project : statsd_proxy
 * Clone   : https://tapassharma@bitbucket.org/tapassharma/statsd-proxy.git
 * Please read the README.md file before proceeding.
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <hiredis.h>
#include <time.h>
#include "log.h"
#include "conf_struct.h"

extern confStruct configuration;

ret_val init_log_file()
{
    configuration.log_file = fopen(LOG_FILE, "a+");
    if(configuration.log_file == NULL)
        return FALSE;
    return TRUE;
}

unsigned long long GetTickCount(void)
{
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)0);
    unsigned long long tmp = (((long long)tv.tv_sec) * 1000000L + (long long)tv.tv_usec);
    return tmp;
}

void get_timestamp(char **timestamp)
{
    (*timestamp) = (char *)malloc(24); // 24 because YYYY/DD/MM HH:MM:SS
    struct tm _ttstr_ = {0};
    time_t _ntt_ = GetTickCount()/1000000;
    bzero((*timestamp), 24);
    localtime_r(&_ntt_, &_ttstr_);
    strftime ( (*timestamp), 24,"%Y/%m/%d %H:%M:%S", &_ttstr_);
    return;    
}

/*******************************************************
 * Author  : Tapas Sharma
 * Email   : tapas.bits@gmail.com
 * Project : statsd_proxy
 * Clone   : https://tapassharma@bitbucket.org/tapassharma/statsd-proxy.git
 * Please read the README.md file before proceeding.
 *********************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include "conf_struct.h"

#define LOG_FILE	"/var/log/statsd_proxy.log"

enum log_levels
{
    LOG_INFO = 1,
    LOG_ERROR,
    LOG_DEBUG
};

#define LEVEL_TO_STRING(l) (l==LOG_INFO || l==LOG_ERROR)?((l==LOG_INFO)?"[INFO]":"[ERROR]"):"[DEBUG]"

/**
 * Keeping this as a macro, since we want to print __FUNCTION__ and __LINE__
 * may be if it crosses certain limit of lines, we can make it a function
 */
#define log(level, msg, ...)                                                                        \
    do                                                                                              \
    {                                                                                               \
     char *timestamp = NULL;                                                                        \
     get_timestamp(&timestamp);                                                                     \
     if(level <= configuration.log_level)                                                           \
     {                                                                                              \
      configuration.log_file?fprintf(configuration.log_file,                                        \
                                         "[%s] %s (%s, %d) "msg"\n",                                \
                                     timestamp,                                                     \
                                     LEVEL_TO_STRING(level),                                        \
                                     __FUNCTION__, __LINE__,                                        \
                                         ##__VA_ARGS__):printf("(%s,%d)LOG FILE NOT INITIALIZED\n", \
                                     __FUNCTION__, __LINE__);                                       \
     }                                                                                              \
     fflush(configuration.log_file);                                                                \
     if(timestamp != NULL)                                                                          \
         free(timestamp);                                                                           \
    }while(0)
         
         
ret_val init_log_file();
void get_timestamp(char **);
#endif

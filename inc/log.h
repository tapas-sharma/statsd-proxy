#ifndef __LOG_H__
#define __LOG_H__

#include "conf_struct.h"

#define LOG_FILE	"/var/log/startd_proxy.log"

enum log_levels
{
    LOG_INFO = 1,
    LOG_ERROR,
    LOG_DEBUG
};

#define LEVEL_TO_STRING(l) (l==LOG_INFO || l==LOG_ERROR)?((l==LOG_INFO)?"[INFO]":"[ERROR]"):"[DEBUG]"

#define log(level,msg,...)                                              \
    do                                                                  \
    {                                                                   \
        if(level <= configuration.log_level)                            \
            configuration.log_file?fprintf(configuration.log_file,      \
                                           "%s (%s, %d) "msg"\n",       \
                                           LEVEL_TO_STRING(level),      \
                                           __FUNCTION__, __LINE__,      \
                                           ##__VA_ARGS__):printf("(%s,%d)LOG FILE NOT INITIALIZED\n", \
                                                                 __FUNCTION__, __LINE__); \
        fflush(configuration.log_file);                                 \
    }while(0)

ret_val init_log_file();

#endif

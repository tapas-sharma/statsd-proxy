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

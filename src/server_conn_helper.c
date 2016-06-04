#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h> 
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
#include "queue.h"

#define TIMEOUT_SEC	3
#define TIMEOUT_MS	500000

extern confStruct configuration;
extern connStruct connections;

/**
 * Open a UDP IPv4 socket and bind to it
 * @return
 * socket return the successfully open socket
 */
int get_socket(int port, char *ip, ret_val bind_flag)
{
  struct sockaddr_in sin;
  int optval,optlen;
  int sock ;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  fcntl(sock, F_SETFD, FD_CLOEXEC);
  if(sock  < 0)
    {
      log(LOG_ERROR,"Opening UDP socket failed :%s\n", strerror(errno) );
      close(sock);
      return FALSE;
    }

  bzero( &sin, sizeof( struct sockaddr_in ));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(ip);
  sin.sin_port = htons(port);
  optval = 1;
  optlen = sizeof(optval);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR ,  &optval, optlen);
  
  if(bind_flag == TRUE)
    {
        if( bind(sock, (struct sockaddr *) &sin, sizeof(sin)) )
        {
            log(LOG_ERROR,"Binding UDP socket failed :%s\n", strerror(errno) );
            close(sock);
            return FALSE;
        }
    }
  log(LOG_INFO, "Opened sock %d for %s:%d bind: %d", sock, ip, port, bind_flag);
  return sock;
}

/**
 * Helper function to open a connection to the redis server
 * using the hiredis library
  * @return
 * TRUE When we were able to open connection to the redis server
 * FALSE when there was some issue, look in the logs
 */
ret_val open_redis_connection()
{
  struct timeval timeout = {TIMEOUT_SEC, TIMEOUT_MS};
  //try to open a connection
  connections.redis_conn = redisConnectWithTimeout(configuration.redis_server_ip,
						   configuration.redis_server_port, timeout);
  if(connections.redis_conn == NULL || connections.redis_conn->err)
    {
      log(LOG_ERROR,"Not able to open redis connection, %s",connections.redis_conn->errstr);
      redisFree(connections.redis_conn);
      return FALSE;
    }
  log(LOG_INFO, "Connection to redis was succesfull on %s:%d",
      configuration.redis_server_ip,configuration.redis_server_port);
  connections.redis_conn_open = TRUE;
  return TRUE;
}

/**
 * Helper function to open a connection to the statsd server configured
  * @return
 * TRUE When we were able to open connection succesfully
 * FALSE when there was some issue, look in the logs
 */
ret_val open_statsd_connection()
{
  connections.statsd_conn = get_socket(configuration.statsd_server_port,
				       configuration.statsd_server_ip, FALSE);
  
  bzero( &connections.send_to, sizeof( struct sockaddr_in ));
  connections.send_to.sin_family = AF_INET;
  connections.send_to.sin_addr.s_addr = inet_addr(configuration.statsd_server_ip);
  connections.send_to.sin_port = htons(configuration.statsd_server_port);
  
  if(connections.statsd_conn == FALSE || connections.statsd_conn < 0)
    {
      log(LOG_ERROR, "Cannot open the socket for statsd, please change the configuration and try again.");
      return FALSE;
    }
  connections.statsd_conn_open = TRUE;  
  return TRUE;
}

/**
 * Helper function to send the data we received to the
 * configured statsd server
 * @return
 * TRUE When we sent the data
 * FALSE when there was some issue, look in the logs
 */

ret_val send_data_to_statsd(char *data,int data_len)
{
  int ret = -1;
  ret = sendto(connections.statsd_conn, data, data_len, 0,
	       (struct sockaddr *) &connections.send_to, sizeof(connections.send_to));
  if(ret < 0)
    {
      log(LOG_ERROR, "Was not able to send data, error %s", strerror(errno));
      return FALSE;
    }
  log(LOG_ERROR, "Send %d bytes", ret);
  return TRUE;
}

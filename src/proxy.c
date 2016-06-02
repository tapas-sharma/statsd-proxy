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
#include "queue.h"

#define TAKE_WRITE_LOCK(x) pthread_rwlock_wrlock(x)
#define TAKE_READ_LOCK(x) pthread_rwlock_rdlock(x)
#define UNLOCK(x) pthread_rwlock_unlock(x)

//global configuration for proxy and connections
confStruct configuration;
connStruct connections;
Queue *req_queue; // all the requests are here

pthread_rwlock_t req_queue_lock; // lock on the queue
pthread_rwlockattr_t rwattr ; //attributes for this lock

/**
 * This will run in a while(1), processing the client UDP requests
 */
void *process_client(void *threadname)
{
  char msg[ MAX_BUF_TO_RECV + 2 ];// 1 for code & 1 for null terminator
  char *all = NULL;
  const char *name = threadname;
  struct sockaddr_in cliAddr;
  socklen_t cliLen = sizeof(cliAddr);
  socklen_t default_len = cliLen;
  int rcv;
  int n = -1;
  if ( open_statsd_connection() == FALSE)
    {
      log(LOG_ERROR, "Cannot open connection to stasd server, please check your configuration.");
      exit(1);
    }
  if (open_redis_connection() == FALSE)
    {
      log(LOG_ERROR, "Cannot open connection to redis server, please check your configuration.");
      exit(1);      
    }
  while (1)
    {
      n = -1;
      cliLen = default_len;
      bzero(&msg, MAX_BUF_TO_RECV+2);
      //peek how much data is on the UDP socket
      n = recvfrom(connections.proxy_fd, msg, MAX_BUF_TO_RECV+1, MSG_PEEK,
		   (struct sockaddr *) &cliAddr, &cliLen);
      
      if(n < 0)
	{
	  log(LOG_ERROR, "[%s] Could not receive data for udp_sock : %s", name,strerror(errno) );
	  continue;
	}
      //malloc the buffer which we will use for reading in.
      all = (char *)malloc(n);
      //read that data
      rcv = recv(connections.proxy_fd, all, n+1, 0);
      if(rcv < 0)
	{
	  log(LOG_ERROR, "[%s] Could not receive %d bytes data from udp_sock : %s",
	      name, n, strerror(errno) );
	  continue;
	}
      //enqueue the data in the request queue
      TAKE_WRITE_LOCK(&req_queue_lock);
      log(LOG_DEBUG, "Data Rcvd is: %s", all);
      enqueue(&req_queue, all, n);
      UNLOCK(&req_queue_lock);
      send_data_to_statsd(all, n);//send data immediately to the statsd server, we can process it later on
      free(all);
    }
  
  log(LOG_ERROR,"[%s] Exiting client thread.", name);
  pthread_exit(NULL);
  
  return (void*)TRUE;
}

/**
 * Will replace the terminator with '\0', thus giving us a string for the first value
 */
static int buffer_after_terminator(char *buf, int buf_len, char terminator,
				   char **after_term, int *after_len)
{
  if(buf == NULL)
    return FALSE;
  // Scan for a terminator
  
  char *term_addr = memchr(buf, terminator, buf_len);
  if (!term_addr)
    {
      *after_term = NULL;
      return -1;
    }

  // Convert the delim to a null-seperator
  *term_addr = '\0';

  // Provide the arg buffer, and arg_len
  *after_term = term_addr+1;
  *after_len = buf_len - (term_addr - buf + 1);
  return TRUE;
}


ret_val extract_key_value_and_insert(char *data)
{
  char *key, *value, *type;
  int len_left ;
  double lvalue=0.0;
  redisReply *reply;
  int ret;
  double current_value = 0;
  key = data;
  ret = buffer_after_terminator(data, strlen(data)+1,':', &value, &len_left);
  if(ret == FALSE)
    return FALSE;
  ret = buffer_after_terminator(value, len_left,'|', &type, &len_left);
  if(ret == FALSE)
    return FALSE;
  
  lvalue = atof(value);
  log(LOG_DEBUG,"Going to insert/increment key: %s with value: %f for type: %s\n", key, lvalue, type);

  switch(type[0])
    {
    case COUNT:
      reply = redisCommand(connections.redis_conn, "GET %s", key);
      current_value = atof(reply->str);
      freeReplyObject(reply);
      
      reply = redisCommand(connections.redis_conn, "SET %s %f", key, lvalue+current_value);
      freeReplyObject(reply);
      
      log(LOG_DEBUG, "Setting value from %f to %f of key: %s",
	  current_value, lvalue+current_value, key);
      break;
    default:
      log(LOG_ERROR, "We do not handle this type right now.");
      break;
    }
  return TRUE;
}

void parse_client_data(Queue *data_node)
{
  if(data_node == NULL || data_node->data == NULL)
    return;
  char *data = data_node->data;
  char *token = strtok(data, "\n");
  char *tmp_arg = NULL;
  while(token!=NULL)
    {
      tmp_arg = (char *)malloc(strlen(token)+1);
      snprintf(tmp_arg, strlen(token)+1,"%s ",token);      
      extract_key_value_and_insert(tmp_arg);
      free(tmp_arg);
      token = strtok(NULL, "\n");
    }
  return;
}


void *process_and_publish_requests(void *threadname)
{
  const char *name = threadname;
  Queue *data_node;
  while(1)
    {
      //Take a read lock an dequeue
      TAKE_READ_LOCK(&req_queue_lock);
      data_node = dequeue(&req_queue);
      UNLOCK(&req_queue_lock);
      if(data_node !=NULL)
	{
	  log(LOG_INFO, "[%s] Going to parse request id: %ud", name, data_node->request_id);
	  parse_client_data(data_node);
	  free_queue_struct(data_node);
	}
    }
}

int main(int argc, char *argv[])
{
  int ret;
  pthread_t threads[2];
      
  init_configuration();
  init_log_file();
  if(argc==2)
    {
      log(LOG_INFO,"Loading configuration from %s.", argv[1]);
      read_config_file(argv[1]);
    }
  print_configuration();
  fflush(configuration.log_file);
  /**
   * initialize the connection object, since there are no default
   * values, we can just bzero it here
   */
  bzero(&connections, sizeof(connections));
  
  connections.proxy_fd = get_socket(configuration.proxy_port, configuration.proxy_ip, TRUE);
  if(connections.proxy_fd == FALSE || connections.proxy_fd < 0)
    {
      log(LOG_ERROR, "Cannot open the socket for proxy, please change the configuration and try again.");
      exit(1);
    }
  //init the queue to NULL
  req_queue = NULL;
  //Init the lock for the queue
  pthread_rwlockattr_init(&rwattr);
  pthread_rwlockattr_setkind_np( &rwattr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP ) ;
  pthread_rwlock_init( &req_queue_lock, &rwattr ) ;

  ret = pthread_create(&threads[0], NULL, process_client, (void *)"Client_Processing_Thread");
  if(ret)
    {
      log(LOG_ERROR, "The return code from the pthread_create was %d,"
	  "for Client_Processing_Thread i.e %s", ret, strerror(errno));
      exit(1);
    }


  ret = pthread_create(&threads[1], NULL, process_and_publish_requests,
		       (void *)"Processing_And_Publishing_Thread");
  if(ret)
    {
      log(LOG_ERROR, "The return code from the pthread_create was %d,"
  	  "for Server_Processing_Thread i.e %s", ret, strerror(errno));
      exit(1);
    }
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_exit(NULL);
  return 0;
}

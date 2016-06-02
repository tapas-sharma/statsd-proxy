#ifndef __CONF_STRUCT__
#define __CONF_STRUCT__

//Default values to assign in the conf structure
#define MAX_BUF_TO_RECV         1024
#define DEFAULT_PROXY_PORT	8272
#define DEFAULT_STATSD_PORT	8125
#define DEFAULT_REDIS_PORT	6379
#define IP_LEN			16  //123.456.789.012

#define DEFAULT_PROXY_IP	"127.0.0.1"
#define DEFAULT_STATSD_IP	"127.0.0.1"
#define DEFAULT_REDIS_IP	"127.0.0.1"

#define DELIM		'='

typedef enum retval
  {
    FALSE=0,
    TRUE
  }ret_val;

typedef struct conf_struct
{  
  int proxy_port;
  int statsd_server_port;
  int redis_server_port;
  
  char *proxy_ip;
  char *statsd_server_ip;
  char *redis_server_ip;

  FILE *config_file;
  FILE *log_file;

  int log_level;
  
  unsigned char daemonize:1;
  unsigned char default_init:1;
  unsigned char log_file_closed:1;
  unsigned char conf_file_closed:1;
  
}confStruct;

typedef struct conn_struct
{
  int statsd_conn;
  int proxy_fd;
  redisContext *redis_conn;
  struct sockaddr_in send_to;
  socklen_t send_sock_len;
  unsigned char statsd_conn_open:1;
  unsigned char redis_conn_open:1;
  unsigned char proxy_conn_open:1;
}connStruct;


ret_val init_configuration();
void print_configuration();
ret_val read_config_file(const char *);
int get_socket(int port, char *ip, ret_val bind_flag);
ret_val open_redis_connection();
ret_val open_statsd_connection();
ret_val send_data_to_statsd(char *data, int data_len);

#endif 

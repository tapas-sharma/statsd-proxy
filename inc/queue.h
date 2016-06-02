#ifndef __QUEUE_H__
#define __QUEUE_H__

#define COUNT		'c'
#define GAUGE		'g'
#define TIMING		'm' //generally ms
#define SETS		's'
/**
 * This will have the request that we received, tokenized on '\n'
 * since can have mutiple requests in the statsd protocol separated by '\n'
 * This will be a linked list of requests and will be a member in the queue
 */
typedef struct request_struct
{
  char *req_data; // the tokenized request data
  char type; // values can be c|g|ms|s
  char *key; // the key to identify in redis
  char *value; // the value to be used in set(key, get(key)+value)
  ret_val (*process_type_request)(); // this will the function we use to process this request type
  struct request_struct *next;
}Request;

typedef struct queue_struct
{
  char *data; // using char* since this is not going to be a generic queue
  int len; //lenght of the data in the buffer
  uint32_t request_id; // to uniquely identify the request, incremental
  //struct request_struct *req; // initially NULL
  unsigned char processed:1; //will say that we have parsed the data and created the request list
  unsigned char in_server_queue:1; // Will say that if we are in the server queue, i.e the thread that is writing to statsd server
  struct queue_struct *next;
}Queue;

ret_val enqueue(Queue **, char *, int );
Queue* dequeue(Queue **);
void free_queue_struct(Queue *);

#endif

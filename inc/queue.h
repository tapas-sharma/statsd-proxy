/*******************************************************
 * Author  : Tapas Sharma
 * Email   : tapas.bits@gmail.com
 * Project : statsd_proxy
 * Clone   : https://tapassharma@bitbucket.org/tapassharma/statsd-proxy.git
 * Please read the README.md file before proceeding.
 *********************************************************/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#define COUNT		'c'
#define GAUGE		'g'
#define TIMING		'm' //generally ms
#define SETS		's'

typedef struct queue_struct
{
    char *data; // using char* since this is not going to be a generic queue
    int len; //lenght of the data in the buffer
    uint32_t request_id; // to uniquely identify the request, incremental
    unsigned char processed:1; //will say that we have parsed the data and created the request list
    unsigned char in_server_queue:1; // Will say that if we are in the server queue, i.e the thread that is writing to statsd server
    struct queue_struct *next;
}Queue;

ret_val enqueue(Queue **, char *, int );
Queue* dequeue(Queue **);
void free_queue_struct(Queue *);

#endif

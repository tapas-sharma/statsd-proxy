#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <hiredis.h>
#include "log.h"
#include "conf_struct.h"
#include "queue.h"

extern confStruct configuration;

/**
 * Free the Queue node and the memory its holding
 */
void free_queue_struct(Queue *node)
{
    if(node->data != NULL)
        free(node->data);
    node->len = 0;
    node->data = NULL;
    memcpy(&node->request_id, "0xFFFFFFFFFFFFFFFF", sizeof(uint32_t));
    node->next=NULL;
    free(node);
    return;
}

/**
 * Enqueue data in the global request queue
 */
ret_val enqueue(Queue **user_q, char *data, int len)
{
    Queue *head = *user_q, *tmp=NULL;

    tmp = (Queue *)malloc(sizeof(Queue));
    bzero(tmp, sizeof(Queue));
    tmp->data = (char *)malloc(len);
    tmp->len = len;
    memcpy(tmp->data, data, len);
    //tmp->req = NULL;
    tmp->processed = 0;
    tmp->in_server_queue = 0;
    tmp->next = head; // would be NULL initially
  
    if( head == NULL )
    {
        log(LOG_DEBUG, "Queue was NULL, creating a new queue and using it.");
        head = tmp;
        *user_q = tmp;
        head->request_id++;
    }
    else
    {
        tmp->request_id = head->request_id;
        tmp->request_id++;
        head = tmp;
        *user_q = tmp;
    }
    return TRUE;
}

/**
 * Always deqeue data from the head of the queue and start processing
 * more like a FIFO queue, since we are in a UDP based protocol system
 */
Queue *dequeue(Queue **user_q)
{
    Queue *head = *user_q, *tmp;
    if( head == NULL )
    {
        //log(LOG_DEBUG, "Queue was NULL");
        return NULL;
    }
    tmp = head;
    head = head->next;
    *user_q = head;
    return tmp;
}

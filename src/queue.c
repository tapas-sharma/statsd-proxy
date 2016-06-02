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
#include <hiredis.h>
#include "log.h"
#include "conf_struct.h"
#include "queue.h"

extern confStruct configuration;


void free_request_struct(Request *node)
{
  Request *tmp;
  while(node != NULL)
    {
      if(node->req_data != NULL)
	free(node->req_data);
      if(node->key != NULL)
	free(node->key);
      if(node->value != NULL)
	free(node->value);
      node->req_data = NULL;
      node->key = NULL;
      node->value = NULL;
      node->process_type_request = NULL;
      tmp = node;
      node = node->next;
      free(tmp);
    }
  return;
}

void free_queue_struct(Queue *node)
{
  if(node->data != NULL)
    free(node->data);
  node->len = 0;
  node->data = NULL;
  memcpy(&node->request_id, "0xFFFFFFFFFFFFFFFF", sizeof(uint32_t));
  //free_request_struct(node->req);
  node->next=NULL;
  free(node);
  return;
}


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


/* ret_val insert_in_req(Request **req, char *key, char *value, char type) */
/* { */
/*   Request *head = *req; */
/*   Request *tmp = (Request *)malloc(sizeof(Request)); */
/*   tmp->key = (char *)malloc(strlen(key)); */
/*   snprintf(tmp->key, strlen(key), "%s", key); */
/*   tmp->value = (char *)malloc(strlen(value)); */
/*   snprintf(tmp->value, strlen(value), "%s", value); */
/*   tmp->type = type; */
/*   tmp->next = head; */

/*   if(head == NULL) */
/*     { */
/*       *req = tmp; */
/*     } */
/*   else */
/*     { */
      
/*     } */
/* } */

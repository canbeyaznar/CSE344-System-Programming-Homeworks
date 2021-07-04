#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdio.h>
#include <stdlib.h>


typedef struct _myQueue
{
    int* values;

    int maxsize;
    int current_size;

    int front;
    int rear;

}myQueue;

myQueue* initialize_queue(int size);
int isEmpty(myQueue* q);
int front(myQueue* q);
void push(myQueue* q, int val);
int pop(myQueue* q);

typedef struct _edge{
    int src;
    int dest;
}edge;



#endif
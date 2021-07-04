#include "graph.h"


myQueue* initialize_queue(int size)
{
    myQueue* temp = (myQueue*) malloc(sizeof(myQueue));
    temp->values = (int*) malloc(sizeof(int)*size);
    temp->maxsize = size;
    temp->current_size = 0;
    temp->front = 0;
    temp->rear = -1;

    return temp;
}
int isEmpty(myQueue* q)
{
    if(q->current_size == 0)
        return 1;
    return 0;
}

int front(myQueue* q)
{
    if(isEmpty(q) == 1)
    {
        printf("Something went wrong in queue\n");
        exit(EXIT_FAILURE);
    }
    return q->values[q->front];
}
void push(myQueue* q, int val)
{
    if(q->current_size == q->maxsize)
    {
        printf("Something went wrong in queue\n");
        exit(EXIT_FAILURE);
    }

    q->rear = (q->rear+1) % (q->maxsize);
    q->values[q->rear] = val;
    q->current_size += 1;
}
int pop(myQueue* q)
{
    if(isEmpty(q) == 1)
    {
        printf("Queue is empty\n");
        return -1;
    }
    int temp = front(q);
    q->front = (q->front + 1) % (q->maxsize);
    q->current_size = q->current_size - 1;

    return temp;
}

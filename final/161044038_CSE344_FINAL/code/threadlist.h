#ifndef _THREADLIST_H_
#define _THREADLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _thread_inf{

	int id;
	//int thread_type;
	int socket_fd;
	int isRunning;
	pthread_t graph_thread;
	//pthread_mutex_t graph_mutex;
	//pthread_cond_t graph_cond_var;

}thread_inf;

struct thread_list{
	thread_inf thread_val;

	struct thread_list* next_thread;
};

struct thread_list* createThreadNode(thread_inf val);
void addThreadNode(struct thread_list* head, thread_inf val, int* is_First );
void changeThreadNode(struct thread_list* head,thread_inf val, int index );
int getSize_threadlist(struct thread_list* head);

void print_thread_inf(thread_inf val);
void free_thread_list(struct thread_list* head);
void copy_thread_inf(thread_inf* v1, thread_inf v2);
int countRunningThread(struct thread_list* head);





#endif
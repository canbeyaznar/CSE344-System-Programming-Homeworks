#include "threadlist.h"

// thread_list functions
struct thread_list *createThreadNode(thread_inf val)
{
    struct thread_list *temp = (struct thread_list *)malloc(sizeof(struct thread_list) * 1);
    copy_thread_inf(&(temp->thread_val),val);
    temp->next_thread = NULL;
    return temp;
}

void addThreadNode(struct thread_list *head, thread_inf val, int* is_First)
{

    if(*is_First == 0)
    {
        *is_First = 1;
        head = createThreadNode(val);
        printf("VAl :%d\n",head->thread_val.id);
        //copy_thread_inf(&(head->thread_val),val);
        //head->next_thread = NULL;
    }
    else
    {
        struct thread_list *traverse_list = head;
        while (traverse_list->next_thread != NULL)
        {
            traverse_list = traverse_list->next_thread;
        }
        traverse_list->next_thread = createThreadNode(val);
    }
    
}
void changeThreadNode(struct thread_list* head,thread_inf val, int index )
{
    if(head == NULL)
    {
        return;
    }
    if(index == 0)
    {
        copy_thread_inf(&(head->thread_val),val);
        return;
    }

    struct thread_list *traverse_list = head;
    int i=0;
    while (traverse_list->next_thread != NULL)
    {
        if(i == index)
            break;
        traverse_list = traverse_list->next_thread;
        i++;
    }
    copy_thread_inf(&(traverse_list->thread_val),val);
    //printf("Aha : %d %d\n",i,traverse_list->next_thread->thread_val.id);;

}

void copy_thread_inf(thread_inf* v1, thread_inf v2)
{
    v1->id = v2.id;
    v1->isRunning = v2.isRunning;
    v1->socket_fd = v2.socket_fd;
    //v1->thread_type = v2.thread_type;
}

int getSize_threadlist(struct thread_list *head)
{
    struct thread_list *temp = head;
    int size = 0;
    while (temp != NULL)
    {
        size++;
        //print_thread_inf(temp->thread_val);
        temp = temp->next_thread;
    }
    return size;
}

void free_thread_list(struct thread_list *head)
{
    struct thread_list *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next_thread;
        free(temp);
    }
}

int countRunningThread(struct thread_list* head)
{
    struct thread_list *temp = head;
    int size = 0;
    while (temp != NULL)
    {
        if(temp->thread_val.isRunning == 1)
            size++;
        //print_thread_inf(temp->thread_val);
        temp = temp->next_thread;
    }
    return size;
}

void print_thread_inf(thread_inf val)
{
    printf("\n-----print_thread_inf------\n");
    printf("ID -> %d\n", val.id);
    printf("isRunning -> %d\n", val.isRunning);
    printf("socket_fd -> %d\n", val.socket_fd);
    //printf("thread_type -> %d\n", val.thread_type);
    printf("------------------------------\n");
}

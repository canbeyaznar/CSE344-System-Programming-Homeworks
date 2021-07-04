// Can BEYAZNAR 
// 161044038
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#include "graph.h"

void usage();
int isNUM(char *str_input);
void get_timestamp(char *time_str);
char* _return_timestamp();

int main(int argc, char *argv[])
{ 

    int a_counter=0, p_counter=0, s_counter=0, d_counter=0;
    char* a_input_str=NULL, *p_input_str=NULL,*s_input_str=NULL, *d_input_str=NULL;

    int source=0, dest=0, PORT=0;
    int opt;
    int process_id = getpid();

	clock_t start, end;
	double load_time = 0.0;

    //char* request = NULL;
    char* response=NULL;
    int* request = NULL;
	char* time_stamp = NULL;


    while ((opt = getopt(argc, argv, "a:p:s:d:")) != -1)
	{
		switch (opt)
		{
		case 'a':
			a_counter++;
			a_input_str = optarg;
			break;

		case 'p':
			p_counter++;
			p_input_str = optarg;
			break;

		case 's':
			s_counter++;
			s_input_str = optarg;
			break;

		case 'd':
			d_counter++;
			d_input_str = optarg;
			break;

		case '?':
			usage();
			errno = 5;
			printf("unknown option: %c\n", optopt);
			return 1;

		default:
			usage();
			errno = EIO;
			printf("Error no: %d\n", errno);
			return 1;
		}
	}


    if(a_counter != 1 || p_counter != 1 || 
        s_counter != 1 || d_counter != 1 )
    {
        usage();
		errno = EIO;
		printf("Error no: %d (inputs are not enough)\n", errno);
		exit(EXIT_FAILURE);
    }
    if(isNUM(p_input_str) != 1 || isNUM(s_input_str) != 1 
        || isNUM(d_input_str) != 1 )
    {
        usage();
		errno = EIO;
		printf("Error no: %d\n", errno);
		exit(EXIT_FAILURE);
    }
    PORT = atoi(p_input_str);
    source = atoi(s_input_str);
    dest = atoi(d_input_str);

    if(PORT < 0 || source < 0 || dest < 0)
    {
        usage();
		errno = EIO;
		printf("Error no: %d\n", errno);
		exit(EXIT_FAILURE);
    }

    
    int sock = 0; 
	struct sockaddr_in serv_addr; 

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, a_input_str, &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	time_stamp = _return_timestamp();
	printf("%sClient (%d) connecting to %s\n",time_stamp,process_id, a_input_str);
	free(time_stamp);
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 

    request = (int*) malloc(sizeof(int)*2*sizeof(int));
    response = (char*) malloc(sizeof(char)*40960);


    request[0] = source;
    request[1] = dest;

	time_stamp = _return_timestamp();
	printf("%sClient (%d) connected and requesting a path from node %d to %d\n",time_stamp,process_id, source, dest);
	free(time_stamp);

	start = clock();
	//send(sock,request,strlen(request),0);
	send(sock , request, sizeof(int) * 2,0 ); 
	//valread = read( sock , response, 40960); 
	recv(sock, response, 40960,0);

	end = clock();
	load_time = ((double)(end - start)) / CLOCKS_PER_SEC;

	time_stamp = _return_timestamp();
	if(strcmp(response,"NO PATH") == 0)
	{
		printf("%sServer’s response (%d): %s, arrived in %.1lf seconds, shutting down\n",
		time_stamp,process_id,response,load_time);
	}
	else
	{
		printf("%sServer’s response to (%d): %s, arrived in %.1lf seconds.\n",
		time_stamp,process_id,response,load_time);
	}
	free(time_stamp);
    free(request);
    free(response);
	
	close(sock);

    return 0;
} 

int isNUM(char *str_input)
{
	if (str_input == NULL)
		return -1;
	int length = strlen(str_input);
	int i = 0;
	for (i = 0; i < length; i++)
	{
		if (!(str_input[i] >= '0' && str_input[i] <= '9'))
			return -1;
	}
	return 1;
}

char* _return_timestamp()
{
	char *timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);
	return timestamp;
}

void get_timestamp(char *time_str)
{
	int hour, minute, second, day, month, year;
	time_t t = time(NULL);
	struct tm *timePtr = localtime(&t);

	hour = timePtr->tm_hour;
	minute = timePtr->tm_min;
	second = timePtr->tm_sec;

	day = timePtr->tm_mday;
	month = timePtr->tm_mon + 1;
	year = timePtr->tm_year + 1900;

	sprintf(time_str, "%02d:%02d:%02d %02d/%02d/%02d ",
			hour, minute, second, day, month, year);
}

void usage()
{
    printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
    printf("-a IP Address\n");
    printf("-p PORT\n");
    printf("-s Source of edge\n");
    printf("-a Destination of edge\n");
    printf("Source or destination must be positive value\n");

    printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
}

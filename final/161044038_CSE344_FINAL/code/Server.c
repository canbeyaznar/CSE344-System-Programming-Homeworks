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
#include "threadlist.h"
#include "cache.h"

int currentIndex_graph = 0;
int **myGraph = NULL;
int NodeSize = 0, EdgeSize = 0;
int *EdgeSize_EachNode = NULL;

int count_start_thread = 0, count_max_thread = 0;
int count_current_thread = 0;
int priority_type = 0;

int isThreadCreated = 0;
int isResizeEnd = 0;

int SIGINT_CATCHED = 0;

double load_ratio = 0.0;

pthread_t resize_thread;
pthread_mutex_t resize_mutex;
pthread_cond_t resize_cond_var;
int resize_pipe[2];
int resize_goback_pipe[2];

pthread_mutex_t log_mutex;
pthread_mutex_t reader_mutex;

//pthread_mutex_t writer_mutex;
pthread_cond_t okToRead;
pthread_cond_t okToWrite;

int AR = 0, AW = 0; // active readers and writers
int WR = 0, WW = 0; // waiting readers and writers

pthread_mutex_t writer_priority_mutex;
pthread_mutex_t reader_priority_mutex;

pthread_mutex_t isThreadFinish_control_mutex;
pthread_cond_t isThreadFinish;

int control_val = 0;
int isFirst = 0;
int isCacheInserted = 0;

struct thread_list *thread_pool = NULL;
struct Cache_tree *cache_database;
int **thread_pipes = NULL;

FILE *log_file = NULL;
FILE* control_server_file = NULL;

void prepare_server(FILE *user_input);
void signalhandler(int signum);

void usage();
void usage_tolog();
int isNUM(char *str_input);
void get_timestamp(char *time_str);
void print_timestamp(FILE *fp);
char *return_timestamp();

// thread functions
void *thread_calculate_request(void *index_thread);
void *resize_threads(void *val);
int findAvailableThread();

// graph functions
int getNodeSizeFromFile(FILE *file_input);
void getEdgeSize_EachNode(FILE *file_input);

int readFileAndAddNode(FILE *file_input, int ***graph_input);
void initialize_graph(int ***graph_input, int size);
void printGraphArr(int **graph_input, int size);
void printAllNodePaths(int **graph_input, int size);
int *BFS(int **graph_input, int src, int dest, int size, int line, int *path_result_length);
int BFS_recursive(int **graph_input, int src, int dest, int **visited, int **path, int level, int size);
int generateRandNumber(int l, int u);

int main(int argc, char *argv[])
{
	struct sigaction sigaction_mask;
	sigaction_mask.sa_handler = signalhandler;
	sigaction_mask.sa_flags = 0;
	sigemptyset(&sigaction_mask.sa_mask);
	sigaction(SIGINT, &sigaction_mask, NULL);

	int opt;
	int i_counter = 0, p_counter = 0, o_counter = 0, s_counter = 0, x_counter = 0, r_counter = 0;

	char *PORT_s = 0;
	char *pathToLogFile = NULL;
	char *count_start_thread_s = NULL, *count_max_thread_s = NULL;
	char *priority_type_s = NULL;
	int PORT = 0;
	int i, temp;
	char *fileName = NULL;
	FILE *inputFile = NULL;
	int *send_pipe = NULL;
	srand(time(NULL));

	

	int server_fd, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// for daemon
	//  ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r 2
	while ((opt = getopt(argc, argv, "i:p:o:s:x:r:")) != -1)
	{
		switch (opt)
		{
		case 'i':
			i_counter++;
			fileName = optarg;
			break;

		case 'p':
			p_counter++;
			PORT_s = optarg;
			break;

		case 'o':
			o_counter++;
			pathToLogFile = optarg;
			break;

		case 's':
			s_counter++;
			count_start_thread_s = optarg;
			break;

		case 'x':
			x_counter++;
			count_max_thread_s = optarg;
			break;

		case 'r':
			r_counter++;
			priority_type_s = optarg;
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

	if (i_counter == 0 || p_counter == 0 || o_counter == 0 ||
		s_counter == 0 || x_counter == 0 || r_counter == 0)
	{
		usage();
		errno = EIO;
		printf("Error no: %d (inputs are not enough)\n", errno);
		exit(EXIT_FAILURE);
	}

	inputFile = fopen(fileName, "r+");
	if (inputFile == NULL)
	{
		usage();
		errno = EBADF;
		printf("Error no: %d (error while opening file)\n", errno);
		exit(EXIT_FAILURE);
	}

	if (isNUM(PORT_s) == -1 || isNUM(count_start_thread_s) == -1 ||
		isNUM(count_max_thread_s) == -1 || isNUM(priority_type_s) == -1)
	{
		usage();
		errno = EIO;
		printf("Error no: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	PORT = atoi(PORT_s);
	count_start_thread = atoi(count_start_thread_s);
	count_max_thread = atoi(count_max_thread_s);
	count_current_thread = count_start_thread;
	priority_type = atoi(priority_type_s);
	if (PORT <= 0 || count_start_thread < 2 || count_start_thread > count_max_thread ||
		priority_type < 0 || priority_type > 2)
	{
		usage();
		errno = EIO;
		printf("Error no: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	// this will control is server working or not
	if( access("_ISSERVER_WORKING_", F_OK) != -1 )
	{
		//if the file exists
		printf("You cannot run the server more than once \n");
		exit(EXIT_FAILURE);
	}
	else
	{ 	// if file does not exist create it
		control_server_file = fopen("_ISSERVER_WORKING_","w");
		if(control_server_file == NULL)
		{
			printf("_ISSERVER_WORKING_ could not created please try again\n");
			exit(EXIT_FAILURE);
		}
	}
	
	//***** DAEMON PART *****//
	pid_t p_id = 0;
	pid_t s_sid = 0;

	p_id = fork();
	if (p_id < 0)
	{
		usage();
		errno = EIO;
		printf("Unexpected error at fork()\n");
		fclose(inputFile);
		exit(EXIT_FAILURE);
	}
	if (p_id > 0)
	{
		//printf("\n\nprocess id : %d kill this process for SIGINT\n\n", p_id);

		fclose(inputFile);
		fclose(control_server_file);
		exit(EXIT_SUCCESS); // kill parent process
	}

	umask(0); // change mode
	log_file = fopen(pathToLogFile, "a+");
	if (log_file == NULL)
	{
		errno = EBADF;
		printf("Error no: %d(Opening log_file failed)\n", errno);
		fclose(inputFile);
		fclose(control_server_file);
		unlink("_ISSERVER_WORKING_");
		exit(EXIT_FAILURE);
	}

	s_sid = setsid();
	if (s_sid < 0)
	{
		errno = EBADF;
		printf("Error no: %d(setsid failed)\n", errno);
		fclose(inputFile);
		fclose(log_file);
		fclose(control_server_file);
		unlink("_ISSERVER_WORKING_");
		exit(EXIT_FAILURE);
	}

	if (SIGINT_CATCHED == 1)
	{
		fclose(inputFile);
		fclose(log_file);
		fclose(control_server_file);
		unlink("_ISSERVER_WORKING_");
		return 0;
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	fflush(log_file);

	char *timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);
	//pthread_mutex_lock(&log_mutex);
	fprintf(log_file, "%sExecuting with parameters:\n", timestamp);
	fprintf(log_file, "%s-i %s\n", timestamp, fileName);
	fprintf(log_file, "%s-p %d\n", timestamp, PORT);
	fprintf(log_file, "%s-o %s\n", timestamp, pathToLogFile);
	fprintf(log_file, "%s-s %d\n", timestamp, count_start_thread);
	fprintf(log_file, "%s-x %d\n", timestamp, count_max_thread);
	//pthread_mutex_unlock(&log_mutex);
	free(timestamp);

	fflush(log_file);

	prepare_server(inputFile);

	thread_inf temp_thread_inf;
	temp_thread_inf.id = 0;
	temp_thread_inf.isRunning = 0;
	temp_thread_inf.socket_fd = -1;
	//temp_thread_inf.thread_type = 0;

	isFirst = 1;
	//thread_pool = (struct thread_list*) malloc(sizeof(struct thread_list)*1);
	copy_thread_inf(&(thread_pool->thread_val), temp_thread_inf);
	thread_pool->next_thread = NULL;

	for (i = 1; i < count_current_thread; i++)
	{
		thread_inf temp_thread_inf;
		temp_thread_inf.id = i;
		temp_thread_inf.isRunning = 0;
		temp_thread_inf.socket_fd = -1;
		//temp_thread_inf.thread_type = 0;

		addThreadNode(thread_pool, temp_thread_inf, &isFirst);
	}

	//pthread_mutex_init(&log_mutex, NULL);

	struct thread_list *temp_thread_list = thread_pool;
	while (temp_thread_list != NULL)
	{
		if (pthread_create(&(temp_thread_list->thread_val.graph_thread), NULL, &thread_calculate_request, &(temp_thread_list->thread_val)))
		{
			usage_tolog();
			errno = EIO;
			timestamp = return_timestamp();
			fprintf(log_file, "%sError no: %d(error creating thread)\n", timestamp, errno);
			free(timestamp);
			fflush(log_file);
		}
		temp_thread_list = temp_thread_list->next_thread;
	}
	
	timestamp = return_timestamp();
	fprintf(log_file, "%sA pool of %d threads has been created\n",timestamp,count_current_thread);
	free(timestamp);
	fflush(log_file);

	if (pthread_create(&resize_thread, NULL, &resize_threads, NULL))
	{
		usage_tolog();
		errno = EIO;
		timestamp = return_timestamp();
		fprintf(log_file, "%sError no: %d(error creating thread)\n", timestamp, errno);
		free(timestamp);
		fflush(log_file);
	}

	i = 0;
	int running_thread;

	// Socket Part

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		timestamp = return_timestamp();
		fprintf(log_file, "%sSocket failed %d\n", timestamp, errno);
		free(timestamp);
		fflush(log_file);
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   &opt, sizeof(opt)))
	{
		timestamp = return_timestamp();
		fprintf(log_file, "%ssetsockopt failed %d\n", timestamp, errno);
		free(timestamp);
		fflush(log_file);
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		timestamp = return_timestamp();
		fprintf(log_file, "%slisten failed %d\n", timestamp, errno);
		free(timestamp);
		fflush(log_file);
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		if (SIGINT_CATCHED == 1)
		{
			close(server_fd);
			break;
		}
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
								 (socklen_t *)&addrlen)) < 0)
		{
			close(server_fd);
			//perror("error close2 : ");
			break;
		}
		else
		{

			temp = countRunningThread(thread_pool);
			load_ratio = (double)100 * ((double)temp / (double)count_current_thread);
			
			// look server load and if it is bigger than 75
			// call resize thread
			if (load_ratio >= 75 && isResizeEnd == 0)
			{
				char resize = 'r';
				if (count_current_thread < count_max_thread)
				{
					// send the condition to resize the server 
					write(resize_pipe[1], &resize, 1);
				}

				else
				{
					isResizeEnd = 1;
					resize = 'e';
					// send the condition to exit the server 
					write(resize_pipe[1], &resize, 1);
					if (pthread_join(resize_thread, NULL))
					{
						usage();
						errno = EIO;
						printf("Error no: %d(error waiting thread)\n", errno);
					}
					close(resize_pipe[1]);
					close(resize_pipe[0]);
				}
			}

			// look for available thread
			// and send your socket information to it
			running_thread = findAvailableThread();
			if (running_thread == -1)
			{
				timestamp = return_timestamp();
				fprintf(log_file, "%sNo thread is available! Waiting for one.\n", timestamp);
				free(timestamp);
				fflush(log_file);
				while (running_thread == -1)
					running_thread = findAvailableThread();
			}

			thread_inf temp_thread_inf;
			temp_thread_inf.id = running_thread;
			temp_thread_inf.isRunning = 1;
			temp_thread_inf.socket_fd = 0;
			send_pipe = (int *)malloc(sizeof(int) * 3 * sizeof(int));
			send_pipe[0] = 1;
			send_pipe[1] = new_socket; // send socket file descriptor
			send_pipe[2] = new_socket;

			changeThreadNode(thread_pool, temp_thread_inf, running_thread);

			temp = countRunningThread(thread_pool);
			load_ratio = (double)100 * ((double)temp / (double)count_current_thread);

			timestamp = return_timestamp();
			fprintf(log_file, "%sA connection has been delegated to thread id #%d, system load %.1lf%%\n",
					timestamp, running_thread, load_ratio);
			free(timestamp);

			write(thread_pipes[running_thread][1], send_pipe, sizeof(int) * 3);
			free(send_pipe);

			fflush(log_file);
		}
	}

	// Socket Part

	if (isResizeEnd != 1)
	{
		isResizeEnd = 1;
		char resize = 'e';
		write(resize_pipe[1], &resize, 1);
		if (pthread_join(resize_thread, NULL))
		{
			usage();
			errno = EIO;
			printf("Error no: %d(error waiting thread)\n", errno);
		}
		close(resize_pipe[1]);
		close(resize_pipe[0]);
	}

	temp_thread_list = thread_pool;
	i = 0;

	send_pipe = (int *)malloc(sizeof(int) * 3 * sizeof(int));
	send_pipe[0] = -1;
	send_pipe[1] = -1;
	send_pipe[2] = -1;

	while (i < count_current_thread)
	{
		// send exit condition to exit the threads gracefully
		write(thread_pipes[temp_thread_list->thread_val.id][1], send_pipe, 3 * sizeof(int));

		// and wait...
		if (pthread_join((temp_thread_list->thread_val.graph_thread), NULL))
		{
			usage_tolog();
			errno = EIO;
			timestamp = return_timestamp();
			fprintf(log_file, "%sError no: %d(error waiting thread)\n",timestamp, errno);
			free(timestamp);
			fflush(log_file);
			close(server_fd);
			exit(EXIT_FAILURE);
		}

		temp_thread_list = temp_thread_list->next_thread;
		i++;
	}
	timestamp = return_timestamp();
	fprintf(log_file, "%sAll threads have terminated, server shutting down.\n",timestamp);
	fflush(log_file);
	free(timestamp);

	free_thread_list(thread_pool);

	for (i = 0; i < count_max_thread; i++)
	{
		close(thread_pipes[i][0]);
		close(thread_pipes[i][1]);
		free(thread_pipes[i]);
	}
	free(send_pipe);
	free(thread_pipes);

	free_cache_tree(cache_database);
	for (i = 0; i < NodeSize; i++)
	{
		free(myGraph[i]);
	}
	free(myGraph);

	free(EdgeSize_EachNode);

	fclose(log_file);
	fclose(control_server_file);
	unlink("_ISSERVER_WORKING_"); // delete the control file so another server can work after that

	return 0;
}

void prepare_server(FILE *user_input)
{

	char *timestamp = NULL;
	clock_t start, end;
	double time_used;
	int i, pipe_control;

	//		Graph initialize part		//
	// find size and allocate 2d array  //
	NodeSize = getNodeSizeFromFile(user_input) + 1;
	if (NodeSize == 0) // if there is no input in file exit...
	{
		usage_tolog();
		timestamp = return_timestamp();

		fprintf(log_file, "%sYour Nodes in file is wrong\n", timestamp);
		free(timestamp);
		fclose(user_input);
		fclose(log_file);
		exit(EXIT_FAILURE);
	}

	// count the size of all edges special to FromNodeID
	// For example 0->1, 0->2, 0->3 =>  myGraph[0]'s size is 3
	EdgeSize_EachNode = (int *)malloc(sizeof(int) * NodeSize);
	getEdgeSize_EachNode(user_input);
	

	timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);
	fprintf(log_file, "%sLoading graph...\n", timestamp);
	free(timestamp);

	fflush(log_file);

	start = clock();

	// Allocate the 2D int graph
	// myGraph[NodeSize][EdgeSize_EachNode] 
	// EdgeSize_EachNode is unique size for each NodeID
	initialize_graph(&myGraph, NodeSize);

	readFileAndAddNode(user_input, &myGraph);
	end = clock();

	time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

	fclose(user_input);

	timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);
	//pthread_mutex_lock(&log_mutex);
	fprintf(log_file, "%sGraph loaded in %.1lf seconds with %d nodes and %d edges.\n",
			timestamp, time_used, NodeSize, EdgeSize);
	//pthread_mutex_unlock(&log_mutex);
	free(timestamp);

	//initalize pipes
	thread_pipes = (int **)malloc(sizeof(int *) * count_max_thread);
	for (i = 0; i < count_max_thread; i++)
	{
		thread_pipes[i] = (int *)malloc(sizeof(int) * 2);
		pipe_control = pipe(thread_pipes[i]);
		if (pipe_control < 0)
		{
			timestamp = (char *)malloc(sizeof(char) * 50);
			fprintf(log_file, "%sError creating pipe!!\n", timestamp);
			fflush(log_file);
			free(timestamp);
			int j;
			if (thread_pipes != NULL)
			{
				for (j = 0; j < i; j++)
				{
					if (thread_pipes[i] != NULL)
					{
						close(thread_pipes[i][0]);
						close(thread_pipes[i][1]);
						free(thread_pipes[i]);
					}
				}
				free(thread_pipes);
			}
			
			exit(EXIT_FAILURE);
		}
	}
	pipe_control = pipe(resize_pipe);
	if (pipe_control < 0)
	{
		timestamp = (char *)malloc(sizeof(char) * 50);
		fprintf(log_file, "%sError creating pipe!!\n", timestamp);
		fflush(log_file);
		free(timestamp);
		if (thread_pipes != NULL)
		{
			for (i = 0; i < count_max_thread; i++)
			{
				if (thread_pipes[i] != NULL)
				{
					close(thread_pipes[i][0]);
					close(thread_pipes[i][1]);
					free(thread_pipes[i]);
				}
				free(thread_pipes);
			}
		}

		exit(EXIT_FAILURE);
	}

	int *root_temp_path = (int *)malloc(sizeof(int) * 2);
	root_temp_path[0] = -1; //NodeSize / 2;
	root_temp_path[1] = -1; //NodeSize / 2;

	cache_database = insert_path(cache_database, root_temp_path, 2);
	cache_database->isHead = 1;
	free(root_temp_path);

	thread_pool = (struct thread_list *)malloc(sizeof(struct thread_list));

	if (priority_type == 0)
	{
		pthread_mutex_init(&reader_priority_mutex, NULL);
		pthread_cond_init(&okToRead, NULL);
		pthread_cond_init(&okToWrite, NULL);
	}
	else if (priority_type == 1)
	{
		pthread_mutex_init(&writer_priority_mutex, NULL);
		pthread_cond_init(&okToRead, NULL);
		pthread_cond_init(&okToWrite, NULL);
	}
	else
	{
		pthread_mutex_init(&reader_mutex, NULL);
		//pthread_mutex_init(&writer_mutex, NULL);
	}

	pthread_mutex_init(&isThreadFinish_control_mutex, NULL);
	//pthread_cond_init(&isThreadFinish,NULL);
}


// thread function that calculates bfs and sends response to the client
void *thread_calculate_request(void *current_thread)
{

	thread_inf *current_thread_inf = ((thread_inf *)current_thread);
	int id = current_thread_inf->id;
	char *timestamp = NULL;
	struct Cache_tree *temp_cachenode = NULL;
	char *printMessage = NULL;
	char *sendToClient = NULL;
	//int* read_request = NULL;

	timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);

	fprintf(log_file, "%sThread #%d: waiting for connection\n", timestamp, id);

	free(timestamp);
	fflush(log_file);

	while (1)
	{
		if (SIGINT_CATCHED == 1)
			return NULL;
		current_thread_inf->isRunning = 0;
		
		// get request and read socket_fd
		int *read_request = (int *)malloc(sizeof(int) * 3 * sizeof(int));
		read(thread_pipes[id][0], read_request, 3 * sizeof(int));

		if (SIGINT_CATCHED == 1)
		{
			free(read_request);
			return NULL;
		}

		current_thread_inf->isRunning = 1;

		int isRunning_thread = read_request[0];
		int socket_fd = read_request[1];

		free(read_request);

		// if thread must exit, break and exit from thread
		if (isRunning_thread == -1)
		{
			current_thread_inf->isRunning = -1;
			break;
		}
		else if (isRunning_thread == 1)
		{
			current_thread_inf->isRunning = 1;

			sendToClient = (char *)malloc(sizeof(char) * 40960);

			int *buf = (int *)malloc(sizeof(int) * 2 * sizeof(int));
			recv(socket_fd, buf, (2 * sizeof(int)),0);

			int src = buf[0];
			int dest = buf[1];

			free(buf);

			int *path_result = NULL;
			int pathSize, i;

			if (priority_type == 0) // -r 0 : reader prioritization
			{
				// it will be opposite of reader, writer problem
				// because the reader has higher priority
				// This solution was made as described in the lesson.

				pthread_mutex_lock(&reader_priority_mutex);
				while ((AW + AR) > 0)
				{
					WR++;
					pthread_cond_wait(&okToRead, &reader_priority_mutex);
					WR--;
				}

				AR++;
				pthread_mutex_unlock(&reader_priority_mutex);

				//pthread_mutex_lock(&log_mutex);
				//print_timestamp(log_file);
				timestamp = return_timestamp();
				fprintf(log_file, "%sThread #%d: searching database for a path from node %d to node %d\n",
						timestamp, id, src, dest);

				free(timestamp);

				fflush(log_file);
				//pthread_mutex_unlock(&log_mutex);

				temp_cachenode = find_path(cache_database, src, dest);

				pthread_mutex_lock(&reader_priority_mutex);
				AR--;

				if (WR > 0)
					pthread_cond_signal(&okToRead);
				else if (WW > 0)
					pthread_cond_broadcast(&okToWrite);
				pthread_mutex_unlock(&reader_priority_mutex);

				if (temp_cachenode == NULL || temp_cachenode->isHead == 1) //not found in cache
				{
					//pthread_mutex_lock(&log_mutex);
					timestamp = return_timestamp();
					fprintf(log_file, "%sThread #%d: no path in database, calculating %d->%d\n",
							timestamp, id, src, dest);
					free(timestamp);

					//pthread_mutex_unlock(&log_mutex);
					fflush(log_file);

					path_result = BFS(myGraph, src, dest, NodeSize, 0, &pathSize);

					if (path_result == NULL)
					{
						//pthread_mutex_lock(&log_mutex);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: path not possible from node %d to %d\n",
								timestamp, id, src, dest);
						free(timestamp);
						fflush(log_file);

						strcpy(sendToClient, "NO PATH");
						send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
						free(sendToClient);

						//pthread_mutex_unlock(&log_mutex);
					}

					else
					{

						//pthread_mutex_lock(&log_mutex);

						printMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));
						char *copyMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));

						//fprintf(log_file, "Thread #%d: path calculated: ", id);
						sprintf(printMessage, "%d->", path_result[pathSize - 1]);
						for (i = pathSize - 2; i >= 1; i--)
						{
							strcpy(copyMessage, printMessage);
							sprintf(printMessage, "%s%d->", copyMessage, path_result[i]);
						}
						strcpy(copyMessage, printMessage);
						sprintf(printMessage, "%s%d", copyMessage, path_result[0]);

						strcpy(sendToClient, printMessage);
						send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
						free(sendToClient);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: path calculated:%s\n",
								timestamp, id, printMessage);
						free(timestamp);

						free(printMessage);
						free(copyMessage);
						fflush(log_file);

						//pthread_mutex_unlock(&log_mutex);

						pthread_mutex_lock(&reader_priority_mutex);
						while ((AR + WR) > 0)
						{
							WW++;
							pthread_cond_wait(&okToWrite, &reader_priority_mutex);
							WW--;
						}
						AW++;
						pthread_mutex_unlock(&reader_priority_mutex);

						//pthread_mutex_lock(&log_mutex);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: responding to client and adding path to database\n",
								timestamp, id);
						free(timestamp);

						fflush(log_file);
						//pthread_mutex_unlock(&log_mutex);

						cache_database = insert_path(cache_database, path_result, pathSize);

						free(path_result);

						pthread_mutex_lock(&reader_priority_mutex);
						AW--;
						if (AW == 0 && WR > 0)
							pthread_cond_signal(&okToRead);
						pthread_mutex_unlock(&reader_priority_mutex);
					}
				}
				else
				{

					//pthread_mutex_lock(&log_mutex);

					printMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));
					char *copyMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));

					timestamp = (char *)malloc(sizeof(char) * 50);
					get_timestamp(timestamp);

					sprintf(printMessage, "%d->", temp_cachenode->path_arr[temp_cachenode->size - 1]);
					for (i = temp_cachenode->size - 2; i >= 1; i--)
					{
						strcpy(copyMessage, printMessage);
						sprintf(printMessage, "%s%d->", copyMessage, temp_cachenode->path_arr[i]);
					}
					strcpy(copyMessage, printMessage);
					sprintf(printMessage, "%s%d", copyMessage, temp_cachenode->path_arr[0]);

					strcpy(sendToClient, printMessage);
					send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
					free(sendToClient);

					fprintf(log_file, "%sThread #%d: path found in database: %s\n",
							timestamp, id, printMessage);

					fflush(log_file);
					free(printMessage);
					free(copyMessage);
					free(timestamp);
				}
			}
			else if (priority_type == 1) // -r 1 : writer prioritization
			{
				// it will be reader, writer problem
				// because the writer has higher priority
				// This solution was made as described in the lesson.

				pthread_mutex_lock(&writer_priority_mutex);
				while ((AW + WW) > 0)
				{
					WR++;
					pthread_cond_wait(&okToRead, &writer_priority_mutex);
					WR--;
				}

				AR++;
				pthread_mutex_unlock(&writer_priority_mutex);
				// access DB
				//pthread_mutex_lock(&log_mutex);

				timestamp = return_timestamp();
				fprintf(log_file, "%sThread #%d: searching database for a path from node %d to node %d\n",
						timestamp, id, src, dest);

				free(timestamp);

				fflush(log_file);
				//pthread_mutex_unlock(&log_mutex);

				temp_cachenode = find_path(cache_database, src, dest);

				pthread_mutex_lock(&writer_priority_mutex);
				AR--;

				if (AR == 0 && WW > 0)
					pthread_cond_signal(&okToWrite);
				pthread_mutex_unlock(&writer_priority_mutex);

				if (temp_cachenode == NULL || temp_cachenode->isHead == 1) //not found in cache
				{
					//pthread_mutex_lock(&log_mutex);

					timestamp = return_timestamp();
					fprintf(log_file, "%sThread #%d: no path in database, calculating %d->%d\n",
							timestamp, id, src, dest);
					free(timestamp);

					//pthread_mutex_unlock(&log_mutex);
					fflush(log_file);

					path_result = BFS(myGraph, src, dest, NodeSize, 0, &pathSize);

					if (path_result == NULL)
					{
						//pthread_mutex_lock(&log_mutex);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: path not possible from node %d to %d\n",
								timestamp, id, src, dest);
						free(timestamp);

						strcpy(sendToClient, "NO PATH");
						send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
						free(sendToClient);

						fflush(log_file);
						//pthread_mutex_unlock(&log_mutex);
					}

					else
					{
						pthread_mutex_lock(&writer_priority_mutex);
						while ((AW + AR) > 0)
						{
							WW++;
							pthread_cond_wait(&okToWrite, &writer_priority_mutex);
							WW--;
						}
						AW++;
						pthread_mutex_unlock(&writer_priority_mutex);
						//Access DB

						//pthread_mutex_lock(&log_mutex);

						printMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));
						char *copyMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));

						//fprintf(log_file, "Thread #%d: path calculated: ", id);
						sprintf(printMessage, "%d->", path_result[pathSize - 1]);
						for (i = pathSize - 2; i >= 1; i--)
						{
							strcpy(copyMessage, printMessage);
							sprintf(printMessage, "%s%d->", copyMessage, path_result[i]);
						}
						strcpy(copyMessage, printMessage);
						sprintf(printMessage, "%s%d", copyMessage, path_result[0]);

						strcpy(sendToClient, printMessage);
						send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
						free(sendToClient);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: path calculated:%s\n",
								timestamp, id, printMessage);
						free(timestamp);

						free(printMessage);
						free(copyMessage);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: responding to client and adding path to database\n",
								timestamp, id);
						free(timestamp);

						fflush(log_file);
						//pthread_mutex_unlock(&log_mutex);

						cache_database = insert_path(cache_database, path_result, pathSize);

						free(path_result);

						pthread_mutex_lock(&writer_priority_mutex);
						AW--;
						if (WW > 0)
							pthread_cond_signal(&okToWrite);
						else if (WR > 0)
							pthread_cond_broadcast(&okToRead);
						pthread_mutex_unlock(&writer_priority_mutex);
					}
				}
				else
				{
					
					//pthread_mutex_lock(&log_mutex);

					printMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));
					char *copyMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));

					timestamp = (char *)malloc(sizeof(char) * 50);
					get_timestamp(timestamp);

					sprintf(printMessage, "%d->", temp_cachenode->path_arr[temp_cachenode->size - 1]);
					for (i = temp_cachenode->size - 2; i >= 1; i--)
					{
						strcpy(copyMessage, printMessage);
						sprintf(printMessage, "%s%d->", copyMessage, temp_cachenode->path_arr[i]);
					}
					strcpy(copyMessage, printMessage);
					sprintf(printMessage, "%s%d", copyMessage, temp_cachenode->path_arr[0]);

					strcpy(sendToClient, printMessage);
					send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
					free(sendToClient);

					fprintf(log_file, "%sThread #%d: path found in database: %s\n",
							timestamp, id, printMessage);
					//

					fflush(log_file);
					free(printMessage);
					free(copyMessage);
					free(timestamp);
					//pthread_mutex_unlock(&log_mutex);
				}
			}
			else	// -r 2 : equal priorities to readers and writers 
			{
				pthread_mutex_lock(&reader_mutex);
				//pthread_mutex_lock(&writer_mutex);
				//pthread_mutex_lock(&log_mutex);

				timestamp = return_timestamp();
				fprintf(log_file, "%sThread #%d: searching database for a path from node %d to node %d\n",
						timestamp, id, src, dest);

				free(timestamp);

				fflush(log_file);

				//pthread_mutex_unlock(&log_mutex);

				temp_cachenode = find_path(cache_database, src, dest);
				if (temp_cachenode == NULL || temp_cachenode->isHead == 1) //not found in cache
				{
					//pthread_mutex_lock(&log_mutex);
					timestamp = (char *)malloc(sizeof(char) * 50);
					get_timestamp(timestamp);
					fprintf(log_file, "%sThread #%d: no path in database, calculating %d->%d\n",
							timestamp, id, src, dest);
					free(timestamp);
					//pthread_mutex_unlock(&log_mutex);
					fflush(log_file);

					path_result = BFS(myGraph, src, dest, NodeSize, 0, &pathSize);

					if (path_result == NULL)
					{
						//pthread_mutex_lock(&log_mutex);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: path not possible from node %d to %d\n",
								timestamp, id, src, dest);
						free(timestamp);
						fflush(log_file);

						strcpy(sendToClient, "NO PATH");
						send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
						free(sendToClient);

						//pthread_mutex_unlock(&log_mutex);
						pthread_mutex_unlock(&reader_mutex);
					}

					else
					{
						//pthread_mutex_lock(&log_mutex);

						printMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));
						char *copyMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));

						//fprintf(log_file, "Thread #%d: path calculated: ", id);
						sprintf(printMessage, "%d->", path_result[pathSize - 1]);
						for (i = pathSize - 2; i >= 1; i--)
						{
							strcpy(copyMessage, printMessage);
							sprintf(printMessage, "%s%d->", copyMessage, path_result[i]);
						}
						strcpy(copyMessage, printMessage);
						sprintf(printMessage, "%s%d", copyMessage, path_result[0]);

						strcpy(sendToClient, printMessage);
						send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
						free(sendToClient);

						timestamp = return_timestamp();
						fprintf(log_file, "%sThread #%d: path calculated:%s\n",
								timestamp, id, printMessage);
						free(timestamp);

						free(printMessage);
						free(copyMessage);
						fflush(log_file);

						//pthread_mutex_unlock(&log_mutex);

						//pthread_mutex_lock(&log_mutex);
						print_timestamp(log_file);
						fprintf(log_file, "Thread %d: responding to client and adding path to database\n", id);

						//pthread_mutex_unlock(&log_mutex);

						cache_database = insert_path(cache_database, path_result, pathSize);

						free(path_result);
						pthread_mutex_unlock(&reader_mutex);
					}
				}
				else
				{

					pthread_mutex_unlock(&reader_mutex);
					//pthread_mutex_lock(&log_mutex);

					printMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));
					char *copyMessage = (char *)malloc(sizeof(char) * ((pathSize * 15) + 300));

					timestamp = (char *)malloc(sizeof(char) * 50);
					get_timestamp(timestamp);

					sprintf(printMessage, "%d->", temp_cachenode->path_arr[temp_cachenode->size - 1]);
					for (i = temp_cachenode->size - 2; i >= 1; i--)
					{
						strcpy(copyMessage, printMessage);
						sprintf(printMessage, "%s%d->", copyMessage, temp_cachenode->path_arr[i]);
					}
					strcpy(copyMessage, printMessage);
					sprintf(printMessage, "%s%d", copyMessage, temp_cachenode->path_arr[0]);

					strcpy(sendToClient, printMessage);
					send(socket_fd, sendToClient, sizeof(char) * 40960, 0);
					free(sendToClient);

					fprintf(log_file, "%sThread #%d: path found in database: %s\n",
							timestamp, id, printMessage);
					//

					fflush(log_file);
					free(printMessage);
					free(copyMessage);
					free(timestamp);
					//pthread_mutex_unlock(&log_mutex);
				}
			}

			current_thread_inf->isRunning = 0;
			isCacheInserted = 1;
			//pthread_cond_signal(&isThreadFinish);
			close(socket_fd);
			//break;
		}
	}

	return NULL;
}

// thread function that resizes the thread pool
void *resize_threads(void *val)
{
	int numberOfNewThread = 0, i,temp;
	char request;
	double _load_ratio = 0.0;
	while (1)
	{
		if (SIGINT_CATCHED == 1)
			return NULL;

		read(resize_pipe[0], &request, 1);

		if (SIGINT_CATCHED == 1)
			return NULL;

		// exit from thread
		if (request == 'e')
		{
			isResizeEnd = 1;
			return NULL;
		}
		//resize
		else if (request == 'r')
		{
			//Calculate how many threads to create
			numberOfNewThread = count_current_thread / 4;
			if ((count_current_thread + numberOfNewThread) > count_max_thread)
			{
				numberOfNewThread = count_max_thread - count_current_thread;
			}

			else if (numberOfNewThread == 0)
				numberOfNewThread = 1;

			for (i = 0; i < numberOfNewThread; i++)
			{
				thread_inf temp;
				temp.id = count_current_thread + i;
				temp.isRunning = 0;

				addThreadNode(thread_pool, temp, &isFirst);
			}

			struct thread_list *travers_list = thread_pool;
			for (i = 0; i < count_current_thread; i++)
				travers_list = travers_list->next_thread;

			// call pthread_create for newly created threads
			for (i = 0; i < numberOfNewThread; i++)
			{
				pthread_create(&travers_list->thread_val.graph_thread, NULL, &thread_calculate_request, &(travers_list->thread_val));
				travers_list = travers_list->next_thread;
			}


			//update thread count
			count_current_thread += numberOfNewThread;

			// calculate server load after resize
			temp = countRunningThread(thread_pool);
			_load_ratio = (double)100 * ((double)temp / (double)count_current_thread);

			char *timestamp = return_timestamp();
			fprintf(log_file, "%sSystem load %.1lf%%, pool extended to %d threads\n",
					timestamp, _load_ratio, count_current_thread);
			free(timestamp);
			fflush(log_file);
		}
	}
}

void signalhandler(int signum)
{
	switch (signum)
	{
	case SIGINT:
		SIGINT_CATCHED = 1;
		char* time_stamp = return_timestamp();
		fprintf(log_file, "%sTermination signal received, waiting for ongoing threads to complete.\n",time_stamp);
		fflush(log_file);
		free(time_stamp);
		break;

	default:
		break;
	}
}

// this function returns id of first available thread...
int findAvailableThread()
{
	int i = 0;
	struct thread_list *traverseList = thread_pool;
	while (traverseList != NULL)
	{
		if (traverseList->thread_val.isRunning == 0)
			return i;
		traverseList = traverseList->next_thread;
		i++;
	}
	return -1;
}

// this function assigns the values read from the file to the graph array
int readFileAndAddNode(FILE *file_input, int ***graph_input)
{
	char control_char;
	char *firstVal = NULL;
	char *secondVal = NULL;
	int firstIndex = 0;
	int control = 0;

	edge tempEdge;
	tempEdge.dest = -1;
	tempEdge.src = -1;

	int current_id_index = 0;
	int current_id = 0;

	int inputCount = 1,i;
	//this will help while adding edge
	int* currentIndexes = (int*) malloc(sizeof(int)*NodeSize);
	for(i=0; i<NodeSize; i++)
	{
		currentIndexes[i] = 0; 
	}

	fseek(file_input, 0, SEEK_SET);
	while (control == 0)
	{
		control_char = fgetc(file_input);
		if (control_char == EOF)
			return -1;
		if (control_char == '#') // if it is command line
		{
			while (fgetc(file_input) != '\n')
			{
			}
		}
		else
		{
			if (control_char >= '0' && control_char <= '9')
				break; // Node part came break and read in another loop
			else
			{
				//printf("Your file input is wrong this line will be ignored\n");
				while (fgetc(file_input) != '\n')
				{
				}
			}
		}
	}

	firstVal = (char *)malloc(sizeof(char) * 100);
	firstVal[0] = control_char;
	firstIndex++;
	control = 0;
	while (control == 0)
	{
		control_char = fgetc(file_input);
		if (control_char == '\t')
		{
			control = 1;
			firstVal[firstIndex] = '\0';
			break;
		}
		else if (control_char == '\n')
			break;
		else
		{
			firstVal[firstIndex] = control_char;
			firstIndex++;
		}
	}
	secondVal = (char *)malloc(sizeof(char) * 100);
	if (isNUM(firstVal) == -1)
	{
		//printf("Your first value of first input is wrong\n");
	}
		
	else
	{
		tempEdge.src = atoi(firstVal);
		fscanf(file_input, "%s\n", secondVal);
		if (isNUM(secondVal) == -1)
		{
			//printf("Your second value of first input is wrong\n");
		}
			
		else
		{
			current_id = tempEdge.src;
			tempEdge.dest = atoi(secondVal);
			(*graph_input)[tempEdge.src][currentIndexes[current_id]] = tempEdge.dest;
			current_id_index++;
			currentIndexes[current_id]++;
		}
	}

	while (1)
	{
		
		if (fscanf(file_input, "%s\t%s\n", firstVal, secondVal) == EOF)
			break;
		if (isNUM(firstVal) == -1)
		{
			//printf("Your first value of %d. input is wrong\n", inputCount);
		}
		else if (isNUM(secondVal) == -1)
		{
			//printf("Your second value of %d. input is wrong\n", inputCount);
		}
		else
		{
			tempEdge.src = atoi(firstVal);
			tempEdge.dest = atoi(secondVal);

			if (tempEdge.src < 0)
			{
				//printf("Your first value of %d. input is wrong\n", inputCount);
			}
				
			else if (tempEdge.dest < 0)
			{
				//printf("Your second value of %d. input is wrong\n", inputCount);
			}
				
			else
			{
				(*graph_input)[tempEdge.src][currentIndexes[tempEdge.src]] = tempEdge.dest;
				currentIndexes[tempEdge.src]++;
			}
			inputCount++;
		}
	}

	free(firstVal);
	free(secondVal);
	free(currentIndexes);
	return 0;
}


/*
	This function finds the maximum NodeID in the file.
	Important note: If even a number is a negative value,
					the program ends with an error message !!
*/
int getNodeSizeFromFile(FILE *file_input)
{
	char control_char;
	char *firstVal = NULL;
	char *secondVal = NULL;
	int firstIndex = 0;
	int control = 0;

	int inputCount = 0;
	int maxNodeID = -1;
	int tempNodeID = -1;

	while (control == 0)
	{
		control_char = fgetc(file_input);
		if (control_char == EOF)
			return -1;
		if (control_char == '#') // if it is command line
		{
			while (fgetc(file_input) != '\n')
			{
			}
		}
		else
		{
			if (control_char >= '0' && control_char <= '9')
				break; // Node part came break and read in another loop
			else
			{
				fprintf(log_file, "Your inputs in file are wrong\n");
				fflush(log_file);
				return -1;
			}
		}
	}

	firstVal = (char *)malloc(sizeof(char) * 100);
	firstVal[0] = control_char;
	firstIndex++;
	control = 0;
	while (control == 0)
	{
		control_char = fgetc(file_input);
		if (control_char == '\t')
		{
			control = 1;
			firstVal[firstIndex] = '\0';
			break;
		}
		else if (control_char == '\n')
			break;
		else
		{
			firstVal[firstIndex] = control_char;
			firstIndex++;
		}
	}
	secondVal = (char *)malloc(sizeof(char) * 100);
	if (isNUM(firstVal) == -1)
	{
		fprintf(log_file, "Your first value of first input is wrong\n");
		fflush(log_file);
		free(firstVal);
		free(secondVal);
		return -1;
	}

	else
	{
		maxNodeID = atoi(firstVal);
		fscanf(file_input, "%s\n", secondVal);
		if (isNUM(secondVal) == -1)
		{
			fprintf(log_file, "Your second value of first input is wrong\n");
			fflush(log_file);
			free(firstVal);
			free(secondVal);
			return -1;
		}

		else
		{
			tempNodeID = atoi(secondVal);
			if (maxNodeID < tempNodeID)
				maxNodeID = tempNodeID;
			inputCount++;
		}
	}

	while (1)
	{
		if (fscanf(file_input, "%s\t%s\n", firstVal, secondVal) == EOF)
			break;
		if (isNUM(firstVal) == -1)
		{
			fprintf(log_file, "Your first value of %d. input is wrong\n", inputCount);
			fflush(log_file);
			free(firstVal);
			free(secondVal);
			return -1;
		}
		else if (isNUM(secondVal) == -1)
		{
			fprintf(log_file, "Your second value of %d. input is wrong\n", inputCount);
			fflush(log_file);
			free(firstVal);
			free(secondVal);
			return -1;
		}
		else
		{
			tempNodeID = atoi(firstVal);
			if (tempNodeID < 0)
			{
				fprintf(log_file, "Your first value of %d. input is wrong\n", inputCount);
				fflush(log_file);
				free(firstVal);
				free(secondVal);
				return -1;
			}

			if (maxNodeID < tempNodeID)
				maxNodeID = tempNodeID;
			tempNodeID = atoi(secondVal);

			if (tempNodeID < 0)
			{
				fprintf(log_file, "Your second value of %d. input is wrong\n", inputCount);
				fflush(log_file);
				free(firstVal);
				free(secondVal);
				return -1;
			}

			if (maxNodeID < tempNodeID)
				maxNodeID = tempNodeID;

			inputCount++;
		}
	}

	free(firstVal);
	free(secondVal);

	if (maxNodeID == -1)
	{
		fprintf(log_file, "\nSomething is wrong in your input file\n");
		fflush(log_file);

		return -1;
	}

	EdgeSize = inputCount;
	return maxNodeID;
}

//This function reads the file again and counts how many of each Node.
void getEdgeSize_EachNode(FILE *file_input)
{
	fseek(file_input, 0, SEEK_SET);

	char control_char;
	char *firstVal = NULL;
	char *secondVal = NULL;
	int firstIndex = 0;
	int control = 0;

	int inputCount = 0;
	int tempNodeID = -1;

	int current_id = 0;

	while (control == 0)
	{
		control_char = fgetc(file_input);
		if (control_char == EOF)
			return;
		if (control_char == '#') // if it is command line
		{
			while (fgetc(file_input) != '\n')
			{
			}
		}
		else
		{
			if (control_char >= '0' && control_char <= '9')
				break; // Node part came break and read in another loop
			else
			{
				//printf("Your file input is wrong this line will be ignored\n");
				while (fgetc(file_input) != '\n')
				{
				}
			}
		}
	}

	firstVal = (char *)malloc(sizeof(char) * 100);
	firstVal[0] = control_char;
	firstIndex++;
	control = 0;
	while (control == 0)
	{
		control_char = fgetc(file_input);
		if (control_char == '\t')
		{
			control = 1;
			firstVal[firstIndex] = '\0';
			break;
		}
		else if (control_char == '\n')
			break;
		else
		{
			firstVal[firstIndex] = control_char;
			firstIndex++;
		}
	}
	secondVal = (char *)malloc(sizeof(char) * 100);
	if (isNUM(firstVal) == -1)
	{
		//printf("Your first value of first input is wrong\n");
	}

	else
	{
		current_id = atoi(firstVal);

		fscanf(file_input, "%s\n", secondVal);
		/*if (isNUM(secondVal) == -1)
			printf("Your second value of first input is wrong\n");
		
		else
		{
			tempNodeID = atoi(secondVal);
			//EdgeSize_EachNode[current_id]++;
			//current_size++;
		}*/
	}

	while (1)
	{
		if (fscanf(file_input, "%s\t%s\n", firstVal, secondVal) == EOF)
		{
			EdgeSize_EachNode[current_id]++;
			break;
		}

		if (isNUM(firstVal) == -1)
		{
			//printf("Your first value of %d. input is wrong\n", inputCount);
		}
		else if (isNUM(secondVal) == -1)
		{
			//printf("Your second value of %d. input is wrong\n", inputCount);
		}
		else
		{
			tempNodeID = atoi(firstVal);
			EdgeSize_EachNode[tempNodeID]++;
			/*
			if (current_id < tempNodeID)
			{
				EdgeSize_EachNode[current_id]++;
				//EdgeSize_EachNode[current_id] = current_size+1;
				current_id = tempNodeID;
				current_size = 0;
			}
			else
			{
				EdgeSize_EachNode[current_id]++;
				current_size++;
			}*/
			inputCount++;
		}
	}

	free(firstVal);
	free(secondVal);
}


// This function reserves space for the graph.
// Note: Each NodeID has its own size. 
// This size is determined by the EdgeSize_EachNode array
void initialize_graph(int ***graph_input, int size)
{
	int i, j;
	(*graph_input) = (int **)malloc(sizeof(int *) * ((int)size));
	if ((*graph_input) == NULL)
	{
		//printf("Malloc is out of memory!!\n");
		return;
	}
	for (i = 0; i < ((int)size); i++)
	{
		//(*graph_input)[i] = (int *) calloc(size,sizeof(int))
		//(*graph_input)[i] = (int *)malloc(sizeof(int) * ((int)size));

		(*graph_input)[i] = (int *)malloc(sizeof(int) * (EdgeSize_EachNode[i] + 1));
		if ((*graph_input)[i] == NULL)
		{
			//printf("Malloc is out of memory!!\n");
			return;
		}
		if (EdgeSize_EachNode[i] == 0)
		{
			(*graph_input)[i][0] = -999;
		}
		else
		{
			for (j = 0; j < EdgeSize_EachNode[i]; j++)
				(*graph_input)[i][j] = 0;
		}
	}
}

// This function uses simple BFS algorithm
int *BFS(int **graph_input, int src, int dest, int size, int line, int *path_result_length)
{
	int *visited_arr = (int *)malloc(sizeof(int) * size);
	int *visited_values = (int *)malloc(sizeof(int) * size); //this will keep visited values paths
	myQueue *bfs_queue = initialize_queue(size); // queue structure with array
	int i, temp_val, isLoopEnd = 0;
	for (i = 0; i < size; i++)
	{
		visited_arr[i] = 0;
		visited_values[i] = -1;
	}

	visited_arr[src] = 1;
	push(bfs_queue, src);
	while (isEmpty(bfs_queue) != 1)
	{
		temp_val = front(bfs_queue); // take new parent
		pop(bfs_queue);

		if (EdgeSize_EachNode[temp_val] != 0 || graph_input[temp_val][0] != -999)
		{
			for (i = 0; i < EdgeSize_EachNode[temp_val]; i++)
			{
				if (visited_arr[graph_input[temp_val][i]] != 1)
				{
					visited_arr[graph_input[temp_val][i]] = 1;

					//graph_input [temp_val] Assign the parent of that value to [i]. 
					// So this route is followed when the destination is found.
					visited_values[graph_input[temp_val][i]] = temp_val;
					push(bfs_queue, graph_input[temp_val][i]);

					// if destination is found break...
					if (graph_input[temp_val][i] == dest)
					{
						isLoopEnd = 1;
						break;
					}
				}
				else
				{
					// if destination is found break...
					// this condition is for when src = dest
					if (graph_input[temp_val][i] == dest)
					{
						if (src == dest)
						{
							visited_values[dest] = temp_val;
							isLoopEnd = 1;
							break;
						}
					}
				}
			}
			if (isLoopEnd == 1)
				break;
		}
	}

	// if path could not found return NULL
	if (isLoopEnd == 0)
	{
		free(bfs_queue->values);
		free(bfs_queue);
		free(visited_arr);
		free(visited_values);
		return NULL;
	}

	if (isLoopEnd != 0)
	{
		int *path_result = (int *)malloc(sizeof(int) * size);
		int currentindex = 0;
		int curr_path_index = dest; 
		
		// start from found dest
		// and assign each traversed value to the path array
		path_result[currentindex] = curr_path_index;
		currentindex++;
		while (visited_values[curr_path_index] != -1)
		{
			// this condition is for when src == dest
			// for example finding path 5 to 5
			if (visited_values[curr_path_index] == dest)
			{
				// append curr_path_index's parent
				path_result[currentindex] = dest;
				currentindex++;
				break;
			}

			// append curr_path_index's parent
			path_result[currentindex] = visited_values[curr_path_index];
			curr_path_index = visited_values[curr_path_index]; // update new dest for finding its parent
			currentindex++;
		}
		
		// assign its length
		*path_result_length = currentindex;

		free(bfs_queue->values);
		free(bfs_queue);
		free(visited_arr);
		free(visited_values);

		return path_result;
		//free(path_result);
	}

	return NULL;

	/* yedek kod
	int* visited = (int*) malloc(sizeof(int)*size);
	int* path = (int*) malloc(sizeof(int)*size);
	int i, control;
	for(i=0; i<size; i++)
		visited[i] = 0;
	for(i=0; i<size; i++)
		path[i] = -1;

	int level=0;
	path[level] = src;

	int counter=0;
	control=BFS_recursive(graph_input,src, dest,&visited ,&path ,level+1,size );
	if(control == 1)
	{

		i =0;
		while(path[i] != -1)
		{
			printf(" %d ->",path[i]);
			i++;
			counter++;
		}
		printf("\n");
		
	}

	free(visited);
	free(path);*/
	/*
		for(i=0; i<size; i++)
	{
		if( visited[i] != 1 && graph_input[src][i] == 1 )
		{
			visited[i] = 1;
			BFS_recursive(graph_input,src, dest,&visited ,&path ,level+1,size );
		}
		
	}
	*/
}
int BFS_recursive(int **graph_input, int src, int dest, int **visited, int **path, int level, int size)
{
	if (src == dest)
	{
		(*path)[level] = dest;
		return 1;
	}
	(*visited)[src] = 1;
	int i;

	if (graph_input[src][dest] == 1)
	{
		(*path)[level] = dest;
		return 1;
	}

	for (i = 0; i < size; i++)
	{
		if ((*visited)[i] != 1 && graph_input[src][i] == 1)
		{
			(*visited)[i] = 1;
			if (BFS_recursive(graph_input, i, dest, visited, path, level + 1, size) == 1)
			{
				(*path)[level] = i;
				return 1;
			}
		}
	}

	return 0;
}

void printGraphArr(int **graph_input, int size)
{
	int i, j;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
			printf(" %d ", graph_input[i][j]);
		printf("\n");
	}
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

void usage()
{

	printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
	printf("-i -> pathToFile (must be valid path)\n");
	printf("-p -> PORT (must be positive integer)\n");
	printf("-o -> pathToLogFile (must be valid path)\n");
	printf("-s -> number of threads in start ( s >=2 )\n");
	printf("-x -> number of maximum threads (x > s)\n");
	printf("-r -> prioritization type (r >= 0 && r <= 2)\n");
	printf("r==0 -> reader prioritization\n");
	printf("r==1 -> writer prioritization\n");
	printf("r==2 -> equal priorities to readers and writers \n");
	printf("NodeID's must not be negative number\n");
	printf("There should be a TAB between the values FromNodeId and ToNodeId in the file\n");
	printf("If there is a file \"_ISSERVER_WORKING_\", please delete it and run the program correctly.\n");
	printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
}

void usage_tolog()
{
	char *timestamp = NULL;
	timestamp = return_timestamp();
	fprintf(log_file, "%s-i -> pathToFile (must be valid path)\n", timestamp);
	fprintf(log_file, "%s-p -> PORT (must be positive integer)\n", timestamp);
	fprintf(log_file, "%s-o -> pathToLogFile (must be valid path)\n", timestamp);
	fprintf(log_file, "%s-s -> number of threads in start ( s >=2 )\n", timestamp);
	fprintf(log_file, "%s-x -> number of maximum threads (x > s)\n", timestamp);
	fprintf(log_file, "%s-r -> prioritization type (r >= 0 && r <= 2)\n", timestamp);
	fprintf(log_file, "%sr==0 -> reader prioritization\n", timestamp);
	fprintf(log_file, "%sr==1 -> writer prioritization\n", timestamp);
	fprintf(log_file, "%sr==2 -> equal priorities to readers and writers \n", timestamp);
	fprintf(log_file, "%sNodeID's must not be negative number\n", timestamp);
	fprintf(log_file, "%sThere should be a TAB between the values FromNodeId and ToNodeId in the file\n",
			timestamp);
	fprintf(log_file,"%sIf there is a file \"_ISSERVER_WORKING_\", please delete it and run the program correctly.\n",
			timestamp);
	fflush(log_file);
	free(timestamp);
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

void print_timestamp(FILE *fp)
{
	char *timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);
	fprintf(fp, "%s", timestamp);
	fflush(log_file);
	free(timestamp);
}

char *return_timestamp()
{
	char *timestamp = (char *)malloc(sizeof(char) * 50);
	get_timestamp(timestamp);
	return timestamp;
}

int generateRandNumber(int l, int u)
{
	int num = (rand() % (u - l + 1)) + l;
	return num;
}


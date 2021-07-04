//  Can BEYAZNAR
//  161044038

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

typedef struct _florist
{
    char *floristName;
    int id;
    double x;
    double y;
    double ms;
    char **flower_name_arr;
    int numberOfFlowers;
    int current_delivery;
    int total_delivery;

    int total_time;
} florist;

typedef struct _client
{
    char *clientName;
    int id;
    double x;
    double y;
    char *flowerName;
} client;

typedef struct _stat
{
    int total_delivery;
    int total_time;
}_stat;

int isThreadCreated = 0;

florist *florist_arr;
client *client_arr;
int **client_requestqueue = NULL;
sig_atomic_t exitThread = 0;

pthread_mutex_t *florist_mutex_arr = NULL;
pthread_cond_t *myCondVar_arr = NULL;

int numberOfFlorist = 0;
int numberOfClient = 0;



void usage()
{
    printf("-o-o-o-o-o-o-o-o-o-\n\n");

    printf("./program -i filePath\n");
    printf("-i -> input file\n");

    printf("Florists must have at least one flower name\n");
    printf("Clients must have only one flower\n");

    printf("-o-o-o-o-o-o-o-o-o-\n\n");
}

void signalhandler(int signum, siginfo_t *info, void *ptr)
{

    //int i, j;
    if (signum == SIGINT)
    {

        printf("\n -o-o-o-o-o-o-o-o-o-o-o-o-o- \n");
        printf("\n\n-SIGINT CAUGHT-\n\n");
        printf("Program will shut down gracefully\n\n");
        printf("\n -o-o-o-o-o-o-o-o-o-o-o-o-o- \n");

        if (exitThread != -1)
        {
            exitThread = -1;
        }
    }
}

double returnChebyshevDist(double x1, double y1, double x2, double y2)
{
    double x = x1 - x2;
    double y = y1 - y2;
    if (x < 0.0)
        x = x * (-1);
    if (y < 0.0)
        y = y * (-1);

    if (x >= y)
        return x;
    else
        return y;
}


// Values in index_arr are florist indexes
// In the loop, the x and y values of these indexes are taken and the distance 
// between the client's x and y values is calculated (Chebyshev Distance).
// And the index of the florist with the shortest distance between them is returned
int returnIndexofClosestFlorist(int *index_arr, int arr_size, int clientIndex)
{
    int minIndex = index_arr[0];
    double minResult;
    double currentResult = 0;
    int i = 0;
    double x1, y1, x2, y2;

    x2 = client_arr[clientIndex].x;
    y2 = client_arr[clientIndex].y;

    x1 = florist_arr[index_arr[i]].x;
    y1 = florist_arr[index_arr[i]].y;

    minResult = returnChebyshevDist(x1, y1, x2, y2);

    for (i = 1; i < arr_size; i++)
    {
        x1 = florist_arr[index_arr[i]].x;
        y1 = florist_arr[index_arr[i]].y;

        currentResult = returnChebyshevDist(x1, y1, x2, y2);
        if (currentResult < minResult)
        {
            minIndex = florist_arr[index_arr[i]].id;
            minResult = currentResult;
        }
    }

    return minIndex;
}

//this function generates a random number between l and u
int generateRandNumber(int l, int u)
{
    int num = (rand() % (u - l + 1)) + l;
    return num;
}
// In the _str string, starting from the location start_index 
// returns the index where the user first finds the character they are looking for
int returnIndexofCharacter(char *_str, int start_index, char character, int size_str)
{
    int i = start_index;
    while (i < size_str && _str[i] != character)
        i++;
    return i;
}

void print_florist(int index)
{
    int i;

    printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
    printf("florist name : %s\n", florist_arr[index].floristName);
    printf("id : %d\n", florist_arr[index].id);
    printf("x : %lf\n", florist_arr[index].x);
    printf("y : %lf\n", florist_arr[index].y);
    printf("ms : %lf\n", florist_arr[index].ms);

    printf("\nflower names : \n");
    for (i = 0; i < florist_arr[index].numberOfFlowers; i++)
        printf("%d : %s\n", (i + 1), florist_arr[index].flower_name_arr[i]);
    printf("Current delivery index: %d\n", florist_arr[index].current_delivery);
    printf("Total delivery : %d\n", florist_arr[index].total_delivery);

    printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
}

void print_client(int index)
{
    printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
    printf("client name : %s\n", client_arr[index].clientName);
    printf("id : %d\n", client_arr[index].id);
    printf("x : %lf\n", client_arr[index].x);
    printf("y : %lf\n", client_arr[index].y);
    printf("flower name : %s\n", client_arr[index].flowerName);
    printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
}

// define the florist information in the given line in the florist struct
void read_florist_str(char *line, int index)
{
    int i = 0, j, k;
    char *florist_name = NULL;
    char *x_str;
    char *y_str;
    char *ms_str;
    char **flowerNames;
    int len_floristname = 0;

    //int numberOfFlowerName=0;
    int start = 0, end = 0;
    int size_line = strlen(line);

    len_floristname = returnIndexofCharacter(line, 0, ' ', size_line);
    start += len_floristname;

    florist_name = (char *)malloc(sizeof(char) * len_floristname);
    for (j = 0; j < len_floristname; j++)
        florist_name[j] = line[j];

    florist_arr[index].floristName = florist_name;

    start = returnIndexofCharacter(line, start, '(', size_line) + 1;
    end = returnIndexofCharacter(line, start, ',', size_line);

    x_str = (char *)malloc(sizeof(char) * (end - start));

    i = 0;
    for (j = start; j < end; j++)
    {
        x_str[i] = line[j];
        i++;
    }
    florist_arr[index].x = atof(x_str);
    free(x_str);

    start = end + 1;
    end = returnIndexofCharacter(line, start, ' ', size_line);

    i = 0;
    y_str = (char *)malloc(sizeof(char) * (end - start));
    for (j = start; j < end; j++)
    {
        y_str[i] = line[j];
        i++;
    }
    florist_arr[index].y = atof(y_str);
    free(y_str);

    start = end + 1;
    end = returnIndexofCharacter(line, start, ')', size_line);

    i = 0;
    ms_str = (char *)malloc(sizeof(char) * (end - start));
    for (j = start; j < end; j++)
    {
        ms_str[i] = line[j];
        i++;
    }
    florist_arr[index].ms = atof(ms_str);
    free(ms_str);

    start = end + 1;
    end = returnIndexofCharacter(line, start, ':', size_line) + 1;
    i = end;
    while (line[i] == ' ')
        i++;
    int numberOfFlowerName = 0;
    while (i < size_line && line[i] != '\n')
    {
        if (line[i] == ',')
            numberOfFlowerName++;
        i++;
    }
    numberOfFlowerName++;

    start = end + 1;
    flowerNames = (char **)malloc(sizeof(char *) * numberOfFlowerName);

    florist_arr[index].numberOfFlowers = numberOfFlowerName;
    for (j = 0; j < numberOfFlowerName; j++)
    {
        start = end + 1;
        end = returnIndexofCharacter(line, start, ',', size_line);

        if (j == numberOfFlowerName - 1)
        {
            flowerNames[j] = (char *)malloc(sizeof(char) * (end - start - 1));
            end -= 1;
        }

        else
            flowerNames[j] = (char *)malloc(sizeof(char) * (end - start));
        i = 0;
        for (k = start; k < end; k++)
        {
            flowerNames[j][i] = line[k];
            i++;
        }
        end = returnIndexofCharacter(line, end + 1, ' ', size_line);
    }
    florist_arr[index].flower_name_arr = flowerNames;
    florist_arr[index].id = index;
    florist_arr[index].current_delivery = 0;
    florist_arr[index].total_delivery = 0;
    florist_arr[index].total_time = 0.0;
}

// define the client information in the given line in the client struct
void read_client_str(char *line, int index)
{
    //client_arr[index]
    char *clientName = NULL;
    char *x_str;
    char *y_str;
    char *flowerName = NULL;
    int start = 0, end = 0;
    int i, j;
    int size_line = strlen(line);

    end = returnIndexofCharacter(line, start, ' ', size_line);
    clientName = (char *)malloc(sizeof(char) * (end));
    for (i = 0; i < end; i++)
        clientName[i] = line[i];
    client_arr[index].clientName = clientName;

    start = end + 1;
    start = returnIndexofCharacter(line, start, '(', size_line) + 1;
    end = returnIndexofCharacter(line, start, ',', size_line);

    x_str = (char *)malloc(sizeof(char) * (end - start));
    i = 0;
    for (j = start; j < end; j++)
    {
        x_str[i] = line[j];
        i++;
    }

    client_arr[index].x = atof(x_str);
    free(x_str);

    start = end + 1;
    end = returnIndexofCharacter(line, start, ')', size_line);

    y_str = (char *)malloc(sizeof(char) * (end - start));
    i = 0;
    for (j = start; j < end; j++)
    {
        y_str[i] = line[j];
        i++;
    }
    client_arr[index].y = atof(y_str);
    free(y_str);

    start = end + 1;
    start = returnIndexofCharacter(line, start, ':', size_line) + 1;

    while (line[start] == ' ')
        start++;
    end = returnIndexofCharacter(line, start, '\n', size_line);

    flowerName = (char *)malloc(sizeof(char) * (end - start));
    i = 0;
    for (j = start; j < end; j++)
    {
        flowerName[i] = line[j];
        i++;
    }
    client_arr[index].flowerName = flowerName;
    client_arr[index].id = index;
}

//Florists and clients are read in the file and defined in the array in the program.
void readFile_init_param(FILE *file)
{
    char *line = NULL;
    size_t bufsize = 0;

    int isfloristRead_end = 0;

    int cur_florist = 0;
    int cur_client = 0;

    while (getline(&line, &bufsize, file) != -1)
    {
        if (strlen(line) == 1)
            isfloristRead_end = 1;
        else
        {
            if (isfloristRead_end == 0)
                numberOfFlorist++;
            else
                numberOfClient++;
        }
        //printf("%s\n",line);
    }
    //if (line)
    //free(line);

    florist_arr = (florist *)malloc(sizeof(florist) * numberOfFlorist);
    client_arr = (client *)malloc(sizeof(client) * numberOfClient);

    fseek(file, 0, SEEK_SET);

    isfloristRead_end = 0;
    while (getline(&line, &bufsize, file) != -1)
    {

        if (line[0] == '\n')
            isfloristRead_end = 1;
        else
        {
            if (isfloristRead_end == 0)
            {
                read_florist_str(line, cur_florist);
                cur_florist++;
            }

            else
            {
                read_client_str(line, cur_client);
                cur_client++;
            }
        }
        
    }

    free(line);
}

// this is the thread that the florists work
// It looks at the request queue according to the index it is 
// located on and if there is an order, a message is printed on the screen.
// If there is no new order, it waits.
// If exitThread is equal to 1, orders are finished.
// If exitThread -1, the user sent the SIGINT signal. And the threads must end.
void *service_flower(void *index_florist)
{
    int index = *((int *)index_florist);
    int i, randomNum, current_distance = 0;
    double current_time = 0.0;
    _stat *myStats = (_stat*) malloc(sizeof(_stat));
    myStats->total_delivery = 0;
    myStats->total_time = 0;

    while (exitThread == 0)
    {

        if (exitThread != 0)
        {
            if (exitThread == -1)
                return myStats;

            for (i = florist_arr[index].current_delivery; i < florist_arr[index].total_delivery; i++)
            {
                if (exitThread == -1)
                    return myStats;

                int servicedClient = client_requestqueue[index][florist_arr[index].current_delivery];

                randomNum = generateRandNumber(1, 250);
                current_distance = returnChebyshevDist(florist_arr[index].x, florist_arr[index].y,
                                                       client_arr[servicedClient].x, client_arr[servicedClient].y);
                current_time = (double)current_distance / florist_arr[index].ms;

                current_time += randomNum;

                florist_arr[index].total_time += current_time;
                if (exitThread != -1)
                {
                    usleep(randomNum * 1000);
                    printf("Florist %s has delivered a ", florist_arr[index].floristName);
                    printf("%s to %s in %dms\n", client_arr[servicedClient].flowerName,
                           client_arr[servicedClient].clientName, (int)current_time);
                }

                florist_arr[index].current_delivery++;
                myStats->total_delivery++;
                myStats->total_time += current_time;
            }
            //pthread_mutex_unlock(&florist_mutex_arr[index]);
            break;
        }

        pthread_mutex_lock(&florist_mutex_arr[index]);
        while (florist_arr[index].current_delivery >= florist_arr[index].total_delivery && exitThread == 0)
        {
            if (exitThread != 0)
            {
                if (exitThread == -1)
                {
                    pthread_mutex_unlock(&florist_mutex_arr[index]);
                    return myStats;
                }

                for (i = florist_arr[index].current_delivery; i < florist_arr[index].total_delivery; i++)
                {
                    if (exitThread == -1)
                    {
                        pthread_mutex_unlock(&florist_mutex_arr[index]);
                        return myStats;
                    }

                    int servicedClient = client_requestqueue[index][florist_arr[index].current_delivery];

                    randomNum = generateRandNumber(1, 250);
                    current_distance = returnChebyshevDist(florist_arr[index].x, florist_arr[index].y,
                                                           client_arr[servicedClient].x, client_arr[servicedClient].y);
                    current_time = (double)current_distance / florist_arr[index].ms;

                    current_time += randomNum;

                    florist_arr[index].total_time += current_time;

                    if (exitThread != -1)
                    {
                        usleep(randomNum * 1000);
                        printf("Florist %s has delivered a ", florist_arr[index].floristName);
                        printf("%s to %s in %dms\n", client_arr[servicedClient].flowerName,
                               client_arr[servicedClient].clientName, (int)current_time);
                    }

                    florist_arr[index].current_delivery++;
                    myStats->total_delivery++;
                    myStats->total_time += current_time;
                }
                pthread_mutex_unlock(&florist_mutex_arr[index]);
                printf("%s is closing shop\n", florist_arr[index].floristName);
                return myStats;
            }

            pthread_cond_wait(&myCondVar_arr[index], &florist_mutex_arr[index]);
        }

        if (exitThread != 0)
        {
            if (exitThread == -1)
            {
                pthread_mutex_unlock(&florist_mutex_arr[index]);
                return myStats;
            }

            for (i = florist_arr[index].current_delivery; i < florist_arr[index].total_delivery; i++)
            {
                if (exitThread == -1)
                {
                    pthread_mutex_unlock(&florist_mutex_arr[index]);
                    return myStats;
                }
                int servicedClient = client_requestqueue[index][florist_arr[index].current_delivery];

                randomNum = generateRandNumber(1, 250);
                current_distance = returnChebyshevDist(florist_arr[index].x, florist_arr[index].y,
                                                       client_arr[servicedClient].x, client_arr[servicedClient].y);
                current_time = (double)current_distance / florist_arr[index].ms;

                current_time += randomNum;

                florist_arr[index].total_time += current_time;

                if (exitThread != -1)
                {
                    usleep(randomNum * 1000);
                    printf("Florist %s has delivered a ", florist_arr[index].floristName);
                    printf("%s to %s in %dms\n", client_arr[servicedClient].flowerName,
                           client_arr[servicedClient].clientName, (int)current_time);
                }

                florist_arr[index].current_delivery++;
                myStats->total_delivery++;
                myStats->total_time += current_time;
            }
            pthread_mutex_unlock(&florist_mutex_arr[index]);
            printf("%s is closing shop\n", florist_arr[index].floristName);
            return myStats;
        }
        if (exitThread == -1)
        {
            pthread_mutex_unlock(&florist_mutex_arr[index]);
            return myStats;
        }
        randomNum = generateRandNumber(1, 250);

        int servicedClient = client_requestqueue[index][florist_arr[index].current_delivery];

        current_distance = returnChebyshevDist(florist_arr[index].x, florist_arr[index].y,
                                               client_arr[servicedClient].x, client_arr[servicedClient].y);
        current_time = (double)current_distance / florist_arr[index].ms;

        current_time += randomNum;

        florist_arr[index].total_time += current_time;

        if (exitThread != -1)
        {
            usleep(randomNum * 1000);
            printf("Florist %s has delivered a ", florist_arr[index].floristName);
            printf("%s to %s in %dms\n", client_arr[servicedClient].flowerName,
                   client_arr[servicedClient].clientName, (int)current_time);
        }

        florist_arr[index].current_delivery++;

        myStats->total_delivery++;
        myStats->total_time += current_time;

        pthread_mutex_unlock(&florist_mutex_arr[index]);
    }

    for (i = florist_arr[index].current_delivery; i < florist_arr[index].total_delivery; i++)
    {
        if (exitThread == -1)
            return myStats;
        int servicedClient = client_requestqueue[index][florist_arr[index].current_delivery];

        randomNum = generateRandNumber(1, 250);
        current_distance = returnChebyshevDist(florist_arr[index].x, florist_arr[index].y,
                                               client_arr[servicedClient].x, client_arr[servicedClient].y);
        current_time = (double)current_distance / florist_arr[index].ms;

        current_time += randomNum;

        florist_arr[index].total_time += current_time;

        if (exitThread != -1)
        {
            usleep(randomNum * 1000);
            printf("Florist %s has delivered a ", florist_arr[index].floristName);
            printf("%s to %s in %dms\n", client_arr[servicedClient].flowerName,
                   client_arr[servicedClient].clientName, (int)current_time);
        }

        florist_arr[index].current_delivery++;

        myStats->total_delivery++;
        myStats->total_time += current_time;
    }
    if (exitThread != -1)
        printf("%s is closing shop\n", florist_arr[index].floristName);
    return myStats;
}

// Aranan cicegin hangi cicekcilerde oldugunu belirler ve onun arrayini dondurur
void find_linked_florist_indexes(int *input_arr, int sizeOfArr, int index_client)
{
    int i, j;
    int count = 0;

    count = 0;
    for (i = 0; i < numberOfFlorist; i++)
    {
        for (j = 0; j < florist_arr[i].numberOfFlowers; j++)
        {
            if (strcmp(client_arr[index_client].flowerName, florist_arr[i].flower_name_arr[j]) == 0)
            {
                input_arr[count] = i;
                count++;
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{

    //signal(SIGINT, signalhandler);
    srand(time(NULL));
    int i, j;
    int opt;
    int i_counter = 0;
    int floristID = 0;

    char *fileName = NULL;
    pthread_t *thread_florist = NULL;
    int *linked_florist_indexes = NULL;
    int *florist_indexes = NULL;

    FILE *inputFile;

    while ((opt = getopt(argc, argv, "i:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            i_counter++;
            fileName = optarg;
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

    if (i_counter != 1)
    {
        usage();
        errno = EIO;
        printf("Error no: %d (inputs are not enough)\n", errno);
        return 1;
    }

    inputFile = fopen(fileName, "r");
    if (inputFile == NULL)
    {
        errno = EBADF;
        printf("Error no: %d (File could not read)\n", errno);
        exit(EXIT_FAILURE);
    }

    readFile_init_param(inputFile);

    fclose(inputFile);

    client_requestqueue = (int **)malloc(sizeof(int *) * numberOfFlorist);
    for (i = 0; i < numberOfFlorist; i++)
    {
        client_requestqueue[i] = (int *)malloc(sizeof(int) * numberOfClient);
        for (j = 0; j < numberOfClient; j++)
            client_requestqueue[i][j] = -1;
    }

    int numberOfLinkedFlorist = 0;
    int index_client = 0;

    florist_indexes = (int *)malloc(sizeof(int) * numberOfFlorist);
    for (i = 0; i < numberOfFlorist; i++)
        florist_indexes[i] = i;

    florist_mutex_arr = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * numberOfFlorist);
    myCondVar_arr = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * numberOfFlorist);

    for (i = 0; i < numberOfFlorist; i++)
    {
        if (pthread_cond_init(&myCondVar_arr[i], NULL) != 0)
        {
            printf("Error creating condition variable\n");
            exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&florist_mutex_arr[i], NULL) != 0)
        {
            printf("Error creating thread mutex\n");
            exit(EXIT_FAILURE);
        }
    }

    int send_delivery_florist_index = 0;

    struct sigaction new_action;
    new_action.sa_sigaction = signalhandler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;

    sigaction(SIGINT, &new_action, NULL);

    //thread part
    isThreadCreated = 1;
    thread_florist = (pthread_t *)malloc(sizeof(pthread_t) * numberOfFlorist);
    while (floristID < numberOfFlorist)
    {

        if (pthread_create(&(thread_florist[floristID]), NULL, &service_flower, &florist_indexes[floristID]))
        {
            usage();
            errno = EIO;
            printf("Error no: %d(error creating thread)\n", errno);
        }
        floristID++;
    }

    // main thread
    
    if (exitThread != -1)
    {
        for (index_client = 0; index_client < numberOfClient; index_client++)
        {
            if (exitThread == -1)
                break;

            numberOfLinkedFlorist = 0;
            for (i = 0; i < numberOfFlorist; i++)
            {
                for (j = 0; j < florist_arr[i].numberOfFlowers; j++)
                {
                    if (strcmp(client_arr[index_client].flowerName, florist_arr[i].flower_name_arr[j]) == 0)
                    {
                        numberOfLinkedFlorist++;
                        break;
                    }
                }
            }

            if (numberOfLinkedFlorist == 0)
            {
                printf("There is no florist selling this flower!!\n");
                exit(EXIT_FAILURE);
            }

            linked_florist_indexes = (int *)malloc(sizeof(int) * numberOfLinkedFlorist);
            find_linked_florist_indexes(linked_florist_indexes, numberOfLinkedFlorist, index_client);

            send_delivery_florist_index = returnIndexofClosestFlorist(linked_florist_indexes, numberOfLinkedFlorist, index_client);
            free(linked_florist_indexes);

            if (pthread_mutex_lock(&florist_mutex_arr[send_delivery_florist_index]) != 0)
            {
                printf("Error while making mutex lock!\n");
                exit(EXIT_FAILURE);
            }

            client_requestqueue[send_delivery_florist_index][florist_arr[send_delivery_florist_index].total_delivery] = index_client;
            florist_arr[send_delivery_florist_index].total_delivery++;

            if (pthread_cond_signal(&myCondVar_arr[send_delivery_florist_index]) != 0)
            {
                printf("Error while sending condition signal\n");
                exit(EXIT_FAILURE);
            }

            if (pthread_mutex_unlock(&florist_mutex_arr[send_delivery_florist_index]) != 0)
            {
                printf("Error while making mutex unlock\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (exitThread != -1)
    {
        for (i = 0; i < numberOfFlorist; i++)
        {
            if (pthread_mutex_lock(&florist_mutex_arr[i]) != 0)
            {
                printf("Error while making mutex lock!\n");
                exit(EXIT_FAILURE);
            }
        }

        exitThread = 1;
        printf("All requests processed.\n");

        for (i = 0; i < numberOfFlorist; i++)
        {
            if (pthread_mutex_unlock(&florist_mutex_arr[i]) != 0)
            {
                printf("Error while making mutex unlock!\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    
    void *tempStat = NULL;
    _stat* mystat_arr = (_stat*) malloc(sizeof(_stat)* numberOfFlorist);
    for (i = 0; i < numberOfFlorist; i++)
    {       
        if (pthread_cond_signal(&myCondVar_arr[i]) != 0)
        {
            printf("Error while sending condition signal\n");
            exit(EXIT_FAILURE);
        }
        
        if (pthread_join(thread_florist[i], &tempStat))
        {
            usage();
            errno = EIO;
            printf("Error no: %d(Error joining thread)\n", errno);
            return 1;
        }
        
        if(tempStat != NULL)
        {
            mystat_arr[i].total_delivery = ((_stat*) tempStat)->total_delivery;
            mystat_arr[i].total_time = ((_stat*) tempStat)->total_time;
        }
        
        
        if (pthread_mutex_destroy(&florist_mutex_arr[i]) != 0)
        {
            printf("Error while making mutex destroy\n");
            exit(EXIT_FAILURE);
        }

        if (pthread_cond_destroy(&myCondVar_arr[i]) != 0)
        {
            printf("Error while making condition variable destroy\n");
            exit(EXIT_FAILURE);
        }
        free(tempStat);
        //printf("%s closing shop\n",florist_arr[i].floristName);
    }
    free(thread_florist);
    
    if (exitThread != -1)
    {
        printf("\n-------------------------------------------------\n");
        printf("Florist\t\t # of sales\t\t Total time");
        printf("\n-------------------------------------------------\n");
        for (i = 0; i < numberOfFlorist; i++)
        {
            printf("%s\t\t%d\t\t %15dms\n", florist_arr[i].floristName, mystat_arr[i].total_delivery,
                   ((int)mystat_arr[i].total_time));
        }
        printf("\n-------------------------------------------------\n");
    }
    free(mystat_arr);
    for (i = 0; i < numberOfFlorist; i++)
        free(client_requestqueue[i]);
    free(client_requestqueue);
    for (i = 0; i < numberOfFlorist; i++)
    {
        free(florist_arr[i].floristName);
        for (j = 0; j < florist_arr[i].numberOfFlowers; j++)
            free(florist_arr[i].flower_name_arr[j]);
        free(florist_arr[i].flower_name_arr);
    }

    for (i = 0; i < numberOfClient; i++)
    {
        free(client_arr[i].clientName);
        free(client_arr[i].flowerName);
    }

    for (i = 0; i < numberOfFlorist; i++)
    {
        pthread_cond_destroy(&myCondVar_arr[i]);
        pthread_mutex_destroy(&florist_mutex_arr[i]);
    }
    if (myCondVar_arr != NULL)
        free(myCondVar_arr);

    if (florist_mutex_arr != NULL)
        free(florist_mutex_arr);

    if (florist_arr != NULL)
        free(florist_arr);

    if (client_arr != NULL)
        free(client_arr);

    if (florist_indexes != NULL)
        free(florist_indexes);

    //pthread_exit(0);
}
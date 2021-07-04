//Can BEYAZNAR 161044038

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
#include <signal.h>
//#include <time.h>

void signal_handler(int signal)
{
    switch (signal)
    {
    case SIGINT:
        raise(SIGTERM);

    default:
        break;
    }
}

void usage()
{
    char writeString[500];
    strcat(writeString, "\n-o-o-o-o-o-o-o-o-o-\n\n");

    strcat(writeString, "N -> number of cooks\n");
    strcat(writeString, "M -> number of students\n");
    strcat(writeString, "T -> number of tables\n");
    strcat(writeString, "S -> counter of size\n");
    strcat(writeString, "K -> kitchen of size\n\n");

    strcat(writeString, "P -> soup\n");
    strcat(writeString, "C -> main course\n");
    strcat(writeString, "D -> desert\n");

    strcat(writeString, "\n-o-o-o-o-o-o-o-o-o-\n\n");
    strcat(writeString, "Constraints (all are integers)\n");
    strcat(writeString, "M > N > 2\n");
    strcat(writeString, "S > 3\n");
    strcat(writeString, "M > T >= 1\n");
    strcat(writeString, "L >= 3\n");
    strcat(writeString, "K = 2LM + 1\n");

    strcat(writeString, "\nFor bonus part: \n");
    strcat(writeString, "M = U+G > 3\n");
    strcat(writeString, "M = U > G >= 1\n");
    strcat(writeString, "\n-o-o-o-o-o-o-o-o-o-\n");

    write(STDOUT_FILENO, writeString, strlen(writeString));
}

int searchValueInArr(int *arr, int size, int val)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i] == val)
            return i;
    }
    return -1;
}

int getMinValue(int v1, int v2, int v3)
{

    if (v1 <= v2)
    {
        if (v1 <= v3)
            return 1;
        else
            return 3;
    }

    else
    {
        if (v2 <= v3)
            return 2;
        else
            return 3;
    }
}

int generateRandomNumber(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

int main(int argc, char *argv[])
{
    int N_Counter = 0;
    int T_Counter = 0;
    int S_Counter = 0;
    int L_Counter = 0;
    int U_Counter = 0;
    int G_Counter = 0;
    int F_Counter = 0;

    char *N_Arr = NULL;
    char *T_Arr = NULL;
    char *S_Arr = NULL;
    char *L_Arr = NULL;
    char *U_Arr = NULL;
    char *G_Arr = NULL;

    char writeString[500];

    mode_t User_ReadOnly = S_IRUSR;
    mode_t User_WriteOnly = S_IWUSR;
    mode_t User_ReadWrite = S_IRUSR | S_IWUSR;

    char *inputFile = NULL;
    int _inputfileSignal;
    char buffer[1];
    size_t bytes_read;

    int N, M, T, S, L, K, U, G;
    int i, j, k;
    int mainProcessID = getpid();

    int *ProcessID_Array;

    int cooksIndexStart, cooksIndexEnd;
    int studentsIndexStart, studentsIndexEnd;
    int supplierIndex;

    struct stat stats;

    int opt;

    //srand(time(0));

    // ./program -N 2 -M 10 -T 5 -S 4 -L 13
    while ((opt = getopt(argc, argv, "N:T:S:L:U:G:F:")) != -1)
    {
        switch (opt)
        {
        case 'N':
            N_Counter = N_Counter + 1;
            N_Arr = optarg;
            break;
            /*
        case 'M':
            M_Counter = M_Counter + 1;
            M_Arr = optarg;
            break;
            */

        case 'T':
            T_Counter = T_Counter + 1;
            T_Arr = optarg;
            break;

        case 'S':
            S_Counter = S_Counter + 1;
            S_Arr = optarg;
            break;
        case 'L':
            L_Counter = L_Counter + 1;
            L_Arr = optarg;
            break;
        case 'U':
            U_Counter++;
            U_Arr = optarg;
            break;
        case 'G':
            G_Counter++;
            G_Arr = optarg;
            break;
        case 'F':
            F_Counter++;
            inputFile = optarg;
            break;

        case '?':
            usage();
            errno = 5;
            sprintf(writeString, "unknown option: %c\n", optopt);
            write(STDOUT_FILENO, writeString, strlen(writeString));

            return 1;

        default:
            usage();
            errno = EIO;
            sprintf(writeString, "Error no: %d\n", errno);
            write(STDOUT_FILENO, writeString, strlen(writeString));

            return 1;
            break;
        }
    }
    if (N_Counter != 1 || T_Counter != 1 || S_Counter != 1 ||
        L_Counter != 1 || U_Counter != 1 || G_Counter != 1 || F_Counter != 1)
    {
        usage();
        errno = EIO;
        sprintf(writeString, "Error no: %d (inputs are not enough)\n", errno);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        return 1;
    }

    signal(SIGINT, signal_handler);

    N = atoi(N_Arr);
    T = atoi(T_Arr);
    S = atoi(S_Arr);
    L = atoi(L_Arr);
    U = atoi(U_Arr);
    G = atoi(G_Arr);

    //if one of these parameter is NaN exit
    if (N != N || T != T || S != S || L != L || U != U || G != G)
    {
        usage();
        errno = EIO;
        sprintf(writeString, "Error no: %d (program read one of these inputs total_SoupControl NaN)\n", errno);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        return 1;
    }

    M = U + G;
    if (!(M > 3) || !((U > G) && (G >= 1)))
    {
        usage();
        errno = EIO;
        sprintf(writeString, "Error no: %d (does not meet the requirements)\n", errno);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        return 1;
    }

    if (!(M > N && N > 2) || !(S > 3) || !((M > T) && (T >= 1)) || !(L >= 3))
    {
        usage();
        errno = EIO;
        sprintf(writeString, "Error no: %d (does not meet the requirements)\n", errno);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        return 1;
    }

    _inputfileSignal = open(inputFile, O_RDONLY, User_ReadWrite);

    if (_inputfileSignal == -1)
    {
        usage();
        errno = EBADF;
        sprintf(writeString, "Error no: %d(Could not open file)\n", errno);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        return 1;
    }

    stat(inputFile, &stats);
    if (stats.st_size < 3 * L * M)
    {
        usage();
        errno = EBADF;
        sprintf(writeString, "Error no : %d(%s file must contain EXACTLY 3LM characters!!)\n", errno, inputFile);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        exit(1);
    }

    lseek(_inputfileSignal, 0, SEEK_SET);

    K = 2 * L * M + 1;
    sprintf(writeString, "Kitchen Size : %d\n", K);
    write(STDOUT_FILENO, writeString, strlen(writeString));
    cooksIndexStart = 0;
    cooksIndexEnd = N - 1;

    studentsIndexStart = N;
    studentsIndexEnd = N + M - 1;

    supplierIndex = N + M;

    int numberOfEachFood = L * M;
    int numberOfChildren = N + M + 1;

    sem_t *sem_plate_Soup, *sem_plate_MainCourse, *sem_plate_Desert;
    sem_t *sem_kitchen_empty, *supply_lock;
    sem_t *numberOfSentSoup, *numberOfSentMainCourse, *numberOfSentDesert;
    sem_t *cook_kitchen_lock;
    sem_t *student_lock, *table_empty;

    sem_t *numberOfSoup_ReplacedCounter, *numberOfMain_ReplacedCounter, *numberOfDesert_ReplacedCounter;

    sem_t *counter_kitchen_lock, *counter_empty, *counter_soupSize,
        *counter_mainSize, *counter_desertSize;
    sem_t *numberOfStudentsAtCounter, *numberOfStudentsLeftCounter;

    sem_t *graduate_lock, *undergraduate_lock;

    numberOfStudentsLeftCounter = mmap(NULL, sizeof(*numberOfStudentsLeftCounter),
                                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                       -1, 0);
    numberOfStudentsAtCounter = mmap(NULL, sizeof(*numberOfStudentsAtCounter),
                                     PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                     -1, 0);

    undergraduate_lock = mmap(NULL, sizeof(*undergraduate_lock),
                              PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                              -1, 0);
    graduate_lock = mmap(NULL, sizeof(*graduate_lock),
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                         -1, 0);
    numberOfSoup_ReplacedCounter = mmap(NULL, sizeof(*numberOfSoup_ReplacedCounter),
                                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                        -1, 0);
    numberOfMain_ReplacedCounter = mmap(NULL, sizeof(*numberOfMain_ReplacedCounter),
                                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                        -1, 0);
    numberOfDesert_ReplacedCounter = mmap(NULL, sizeof(*numberOfDesert_ReplacedCounter),
                                          PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                          -1, 0);

    student_lock = mmap(NULL, sizeof(*student_lock),
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                        -1, 0);
    table_empty = mmap(NULL, sizeof(*table_empty),
                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);
    counter_kitchen_lock = mmap(NULL, sizeof(*counter_kitchen_lock),
                                PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                -1, 0);
    counter_empty = mmap(NULL, sizeof(*counter_empty),
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                         -1, 0);
    counter_soupSize = mmap(NULL, sizeof(*counter_soupSize),
                            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                            -1, 0);
    counter_mainSize = mmap(NULL, sizeof(*counter_mainSize),
                            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                            -1, 0);
    counter_desertSize = mmap(NULL, sizeof(*counter_desertSize),
                              PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                              -1, 0);

    cook_kitchen_lock = mmap(NULL, sizeof(*cook_kitchen_lock),
                             PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                             -1, 0);
    numberOfSentSoup = mmap(NULL, sizeof(*numberOfSentSoup),
                            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                            -1, 0);
    numberOfSentMainCourse = mmap(NULL, sizeof(*numberOfSentMainCourse),
                                  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                  -1, 0);
    numberOfSentDesert = mmap(NULL, sizeof(*numberOfSentDesert),
                              PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                              -1, 0);
    sem_kitchen_empty = mmap(NULL, sizeof(*sem_kitchen_empty),
                             PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                             -1, 0);
    sem_plate_Soup = mmap(NULL, sizeof(*sem_plate_Soup),
                          PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                          -1, 0);
    sem_plate_MainCourse = mmap(NULL, sizeof(*sem_plate_MainCourse),
                                PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                -1, 0);
    sem_plate_Desert = mmap(NULL, sizeof(*sem_plate_Desert),
                            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                            -1, 0);
    supply_lock = mmap(NULL, sizeof(*supply_lock),
                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);

    if (sem_kitchen_empty == MAP_FAILED || sem_plate_Soup == MAP_FAILED ||
        sem_plate_MainCourse == MAP_FAILED || sem_plate_Desert == MAP_FAILED || supply_lock == MAP_FAILED)
    {
        perror("MMAP FAILED!!\n");
        exit(1);
    }

    if (sem_init(numberOfStudentsLeftCounter, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(numberOfStudentsAtCounter, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(undergraduate_lock, 1, 1) == -1)
        perror("sem_init");
    if (sem_init(graduate_lock, 1, 1) == -1)
        perror("sem_init");
    if (sem_init(numberOfSoup_ReplacedCounter, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(numberOfMain_ReplacedCounter, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(numberOfDesert_ReplacedCounter, 1, 0) == -1)
        perror("sem_init");

    if (sem_init(sem_plate_Soup, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(sem_plate_MainCourse, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(sem_plate_Desert, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(sem_kitchen_empty, 1, K) == -1)
        perror("sem_init");
    if (sem_init(supply_lock, 1, 1) == -1)
        perror("sem_init");
    if (sem_init(numberOfSentSoup, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(numberOfSentMainCourse, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(numberOfSentDesert, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(cook_kitchen_lock, 1, 1) == -1)
        perror("sem_init");

    if (sem_init(counter_kitchen_lock, 1, 1) == -1)
        perror("sem_init");
    if (sem_init(counter_empty, 1, S) == -1)
        perror("sem_init");
    if (sem_init(counter_soupSize, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(counter_mainSize, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(counter_desertSize, 1, 0) == -1)
        perror("sem_init");
    if (sem_init(student_lock, 1, 1) == -1)
        perror("sem_init");
    if (sem_init(table_empty, 1, T) == -1)
        perror("sem_init");

    ProcessID_Array = (int *)malloc(sizeof(int) * (N + M + 1));

    for (i = 0; i < N + M + 1; i++)
    {
        if (mainProcessID == getpid() && fork() == 0)
            ProcessID_Array[i] = getpid();
    }

    if (mainProcessID != getpid())
    {
        int indexOfProcess = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
        sprintf(writeString, "current pid : %d, index : %d\n", getpid(), indexOfProcess);
        write(STDOUT_FILENO, writeString, strlen(writeString));
        // Supplier Part
        if (indexOfProcess == supplierIndex)
        {

            int index = 0;
            //This loop continues 3 * L * M times since the supplier has one process.
            for (i = 0; i < 3 * L * M; i++)
            {
                int soupCount, mainCount, desertCount;
                int rs, rm, rd;
                int numberOfEmptyKitchen;
                char buffer[1];

                sem_getvalue(sem_kitchen_empty, &numberOfEmptyKitchen);

                

                // is kitchen empty?
                /*
                if (numberOfEmptyKitchen == 0)
                {
                    sem_getvalue(sem_plate_Soup, &soupCount);
                    sem_getvalue(sem_plate_MainCourse, &mainCount);
                    sem_getvalue(sem_plate_Desert, &desertCount);
                    sprintf(writeString,"\nThe supplier is waiting for kitchen room to free - # of items at kitchen: %d\n",
                           (soupCount + mainCount + desertCount));
                    write(STDOUT_FILENO,writeString,strlen(writeString));
                }
                */
                sem_wait(sem_kitchen_empty);
                sem_wait(supply_lock); // this will lock the kitchen so while supplier send a plate to kitchen, cooks will wait.


                int rand_index = 0;
                //read a character from file
                read(_inputfileSignal, buffer, 1);

                if (buffer[0] == 'P' || buffer[0] == 'p')
                    rand_index = 0;

                else if (buffer[0] == 'C' || buffer[0] == 'c')
                    rand_index = 1;

                else if (buffer[0] == 'D' || buffer[0] == 'd')
                    rand_index = 2;

                //if the character is unknown, the program will be terminated...
                else
                {
                    usage();
                    errno = EBADF;
                    sprintf(writeString, "errno :%d Unknown parameter in file\n", errno);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    raise(SIGTERM);
                }

                sem_getvalue(numberOfSentSoup, &rs);
                sem_getvalue(numberOfSentMainCourse, &rm);
                sem_getvalue(numberOfSentDesert, &rd);
                /*
                int controlFood = 0;
                int rand_index = generateRandomNumber(0, 2);

                sem_getvalue(numberOfSentSoup, &rs);
                sem_getvalue(numberOfSentMainCourse, &rm);
                sem_getvalue(numberOfSentDesert, &rd);
                if (rand_index == 0 && rs < L * M)
                    controlFood = 1;
                else if (rand_index == 1 && rm < L * M)
                    controlFood = 1;
                else if (rand_index == 2 && rd < L * M)
                    controlFood = 1;
                */
                /*
                while (controlFood == 0)
                {
                    rand_index = generateRandomNumber(0, 2);
                    sem_getvalue(numberOfSentSoup, &rs);
                    sem_getvalue(numberOfSentMainCourse, &rm);
                    sem_getvalue(numberOfSentDesert, &rd);

                    if (rand_index == 0 && rs < L * M)
                        controlFood = 1;
                    else if (rand_index == 1 && rm < L * M)
                        controlFood = 1;
                    else if (rand_index == 2 && rd < L * M)
                        controlFood = 1;
                }
                */
                sem_getvalue(sem_plate_Soup, &soupCount);
                sem_getvalue(sem_plate_MainCourse, &mainCount);
                sem_getvalue(sem_plate_Desert, &desertCount);

                if (rand_index == 0) //send soup
                {
                    sprintf(writeString, "The supplier is going to the kitchen to deliver soup: ");
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "    kitchen items P:%d, C:%d, D:%d = %d\n",
                            soupCount, mainCount, desertCount, (soupCount + mainCount + desertCount));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sem_post(numberOfSentSoup);
                    sem_post(sem_plate_Soup);

                    sprintf(writeString, "The Supplier delivered soup - ");
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                }

                else if (rand_index == 1) // send main course
                {
                    sprintf(writeString, "The supplier is going to the kitchen to deliver main course: ");
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "    kitchen items P:%d, C:%d, D:%d = %d\n",
                            soupCount, mainCount, desertCount, (soupCount + mainCount + desertCount));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sem_post(numberOfSentMainCourse);
                    sem_post(sem_plate_MainCourse);
                    sprintf(writeString, "The Supplier delivered main course - ");
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                }

                else if (rand_index == 2) // send desert
                {
                    sprintf(writeString, "The supplier is going to the kitchen to deliver desert: ");
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "    kitchen items P:%d, C:%d, D:%d = %d\n",
                            soupCount, mainCount, desertCount, (soupCount + mainCount + desertCount));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sem_post(numberOfSentDesert);
                    sem_post(sem_plate_Desert);

                    sprintf(writeString, "The Supplier delivered desert - ");
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                }

                sem_getvalue(sem_plate_Soup, &soupCount);
                sem_getvalue(sem_plate_MainCourse, &mainCount);
                sem_getvalue(sem_plate_Desert, &desertCount);

                sprintf(writeString, "- after delivery: kitchen items P:%d, C:%d, D:%d\n", soupCount, mainCount, desertCount);
                write(STDOUT_FILENO, writeString, strlen(writeString));
                sprintf(writeString, "\nSupplier finished supplying - GOODBYE!\n");
                write(STDOUT_FILENO, writeString, strlen(writeString));

                sem_getvalue(sem_plate_Soup, &soupCount);
                sem_getvalue(sem_plate_MainCourse, &mainCount);
                sem_getvalue(sem_plate_Desert, &desertCount);

                sem_post(supply_lock);
            }

            exit(1);
        }

        // Cook Part
        else if (indexOfProcess >= cooksIndexStart && indexOfProcess <= cooksIndexEnd)
        {
            sem_wait(cook_kitchen_lock);
            int a, total_SoupControl, total_MainControl, total_DesertControl;
            int plateSoup, plateMain, plateDesert;

            sem_getvalue(numberOfSentSoup, &total_SoupControl);
            sem_getvalue(numberOfSentMainCourse, &total_MainControl);
            sem_getvalue(numberOfSentDesert, &total_DesertControl);

            sem_getvalue(sem_plate_Soup, &plateSoup);
            sem_getvalue(sem_plate_MainCourse, &plateMain);
            sem_getvalue(sem_plate_Desert, &plateDesert);

            sem_post(cook_kitchen_lock);
            // continue until the supplier sends all the plates.
            while (total_SoupControl != L * M && total_MainControl != L * M && total_DesertControl != L * M)
            {
                sem_wait(cook_kitchen_lock);

                sem_getvalue(sem_plate_Soup, &plateSoup);
                sem_getvalue(sem_plate_MainCourse, &plateMain);
                sem_getvalue(sem_plate_Desert, &plateDesert);

                sem_getvalue(numberOfSentSoup, &total_SoupControl);
                sem_getvalue(numberOfSentMainCourse, &total_MainControl);
                sem_getvalue(numberOfSentDesert, &total_DesertControl);

                if (total_SoupControl == L * M && total_MainControl == L * M && total_DesertControl == L * M)
                {
                    sem_getvalue(sem_plate_Soup, &plateSoup);
                    sem_getvalue(sem_plate_MainCourse, &plateMain);
                    sem_getvalue(sem_plate_Desert, &plateDesert);
                    if (plateSoup == 0 && plateMain == 0 && plateDesert == 0)
                    {
                        sem_post(cook_kitchen_lock);
                        break;
                    }
                }

                sem_getvalue(sem_plate_Soup, &plateSoup);
                sem_getvalue(sem_plate_MainCourse, &plateMain);
                sem_getvalue(sem_plate_Desert, &plateDesert);

                int _rs, _rm, _rd;
                sem_getvalue(numberOfSoup_ReplacedCounter, &_rs);
                sem_getvalue(numberOfMain_ReplacedCounter, &_rm);
                sem_getvalue(numberOfDesert_ReplacedCounter, &_rd);

                if ((_rs + _rm + _rd) + 1 == 3 * L * M)
                {
                    int temp, _processIndex, remaining_food;
                    int counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert;

                    sem_getvalue(sem_plate_Soup, &plateSoup);
                    sem_getvalue(sem_plate_MainCourse, &plateMain);
                    sem_getvalue(sem_plate_Desert, &plateDesert);

                    temp = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());

                    sprintf(writeString, "Cook %d is going to the kitchen to wait for/get a plate - ", temp);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "kitchen items P:%d, C:%d, D:%d = %d\n",
                            plateSoup, plateMain, plateDesert, (plateSoup + plateMain + plateDesert));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    temp = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
                    sprintf(writeString, "Cook %d is going to the counter to deliver ", temp);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sem_getvalue(counter_soupSize, &counter_sizeOfSoup);
                    sem_getvalue(counter_mainSize, &counter_sizeOfMain);
                    sem_getvalue(counter_desertSize, &counter_sizeOfDesert);
                    if (_rs + 1 == L * M)
                    {
                        sprintf(writeString, "soup - counter items P:%d, C:%d, D:%d = %d\n",
                                counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_wait(sem_plate_Soup);

                        sem_wait(counter_empty);
                        sem_wait(counter_kitchen_lock);

                        sem_post(counter_soupSize);

                        sem_getvalue(counter_soupSize, &counter_sizeOfSoup);
                        sem_getvalue(counter_mainSize, &counter_sizeOfMain);
                        sem_getvalue(counter_desertSize, &counter_sizeOfDesert);

                        sprintf(writeString, "Cook %d is placed soup on the counter - ", temp);
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sprintf(writeString, "counter items P:%d, C:%d, D:%d = %d\n", counter_sizeOfSoup,
                                counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(sem_kitchen_empty);
                        sem_getvalue(sem_kitchen_empty, &remaining_food);
                        sprintf(writeString, "Cook %d finished serving - items at kitchen: %d - going home - GOODBYE!!!\n",
                                temp, (K - remaining_food));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(counter_kitchen_lock);
                    }

                    else if (_rm + 1 == L * M)
                    {
                        sprintf(writeString, "main course - counter items P:%d, C:%d, D:%d = %d\n",
                                counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_wait(sem_plate_MainCourse);

                        sem_wait(counter_empty);
                        sem_wait(counter_kitchen_lock);

                        sem_post(counter_mainSize);

                        sem_getvalue(counter_soupSize, &counter_sizeOfSoup);
                        sem_getvalue(counter_mainSize, &counter_sizeOfMain);
                        sem_getvalue(counter_desertSize, &counter_sizeOfDesert);

                        sprintf(writeString, "Cook %d is placed main course on the counter - ", temp);
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sprintf(writeString, "counter items P:%d, C:%d, D:%d = %d\n", counter_sizeOfSoup,
                                counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(sem_kitchen_empty);
                        sem_getvalue(sem_kitchen_empty, &remaining_food);
                        sprintf(writeString, "Cook %d finished serving - items at kitchen: %d - going home - GOODBYE!!!\n",
                                temp, (K - remaining_food));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(counter_kitchen_lock);
                    }

                    else if (_rd + 1 == L * M)
                    {
                        sprintf(writeString, "desert - counter items P:%d, C:%d, D:%d = %d\n",
                                counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_wait(sem_plate_Desert);

                        sem_wait(counter_empty);
                        sem_wait(counter_kitchen_lock);

                        sem_post(counter_desertSize);

                        sem_getvalue(counter_soupSize, &counter_sizeOfSoup);
                        sem_getvalue(counter_mainSize, &counter_sizeOfMain);
                        sem_getvalue(counter_desertSize, &counter_sizeOfDesert);

                        sprintf(writeString, "Cook %d is placed desert on the counter - ", temp);
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sprintf(writeString, "counter items P:%d, C:%d, D:%d = %d\n", counter_sizeOfSoup,
                                counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(sem_kitchen_empty);
                        sem_getvalue(sem_kitchen_empty, &remaining_food);
                        sprintf(writeString, "Cook %d finished serving - items at kitchen: %d - going home - GOODBYE!!!\n",
                                temp, (K - remaining_food));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                    }

                    sem_post(counter_kitchen_lock);
                    sem_post(cook_kitchen_lock);
                    exit(1);
                }

                sem_post(cook_kitchen_lock);
                // continue until the all plates sent to the counter.
                while (plateSoup != 0 && plateMain != 0 && plateDesert != 0)
                {

                    sem_wait(cook_kitchen_lock);
                    int temp;
                    sem_getvalue(sem_plate_Soup, &plateSoup);
                    sem_getvalue(sem_plate_MainCourse, &plateMain);
                    sem_getvalue(sem_plate_Desert, &plateDesert);

                    if (plateSoup == 0 && plateMain == 0 && plateDesert == 0)
                    {
                        sem_post(cook_kitchen_lock);
                        break;
                    }
                    temp = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());

                    sprintf(writeString, "Cook %d is going to the kitchen to wait for/get a plate - ", temp);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "kitchen items P:%d, C:%d, D:%d = %d\n",
                            plateSoup, plateMain, plateDesert, (plateSoup + plateMain + plateDesert));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    int counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert;
                    int minFoodIndex, emptySizeofCounter;
                    int remaining_food;
                    int plate_P, plate_C, plate_D;
                    int itemsAtKitchen;

                    sem_getvalue(sem_kitchen_empty, &itemsAtKitchen);
                    /*
                    if (itemsAtKitchen == K && total_SoupControl != L * M &&
                        total_MainControl != L * M && total_SoupControl != L * M)
                    {
                        temp = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
                        sprintf(writeString,"\nCook %d is waiting at the kitchen - ", temp);
                        write(STDOUT_FILENO,writeString,strlen(writeString));
                        sprintf(writeString,"items at kitchen: %d\n", (K - itemsAtKitchen));
                        write(STDOUT_FILENO,writeString,strlen(writeString));
                    }
                    */

                    sem_getvalue(counter_soupSize, &counter_sizeOfSoup);
                    sem_getvalue(counter_mainSize, &counter_sizeOfMain);
                    sem_getvalue(counter_desertSize, &counter_sizeOfDesert);

                    /*
                    sem_getvalue(counter_empty, &emptySizeofCounter);
                    if (emptySizeofCounter == 0)
                    {
                        temp = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
                        sem_getvalue(sem_kitchen_empty, &remaining_food);
                        sprintf(writeString,"Cook %d is waiting for counter to free - ", temp);
                        write(STDOUT_FILENO,writeString,strlen(writeString));
                        sprintf(writeString,"items on counter P:%d, C:%d, D:%d, at kitchen:%d\n", counter_sizeOfSoup,
                               counter_sizeOfMain, counter_sizeOfDesert, (K - remaining_food));
                        write(STDOUT_FILENO,writeString,strlen(writeString));
                    }
                    */

                    // Find the least available food on the counter and send that food.
                    minFoodIndex = getMinValue(counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert);
                    temp = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
                    sprintf(writeString, "Cook %d is going to the counter to deliver ", temp);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    if (minFoodIndex == 1) // send soup
                        sem_wait(sem_plate_Soup);

                    else if (minFoodIndex == 2) // send main course
                        sem_wait(sem_plate_MainCourse);

                    else // send desert
                        sem_wait(sem_plate_Desert);

                    sem_wait(supply_lock);
                    sem_wait(counter_empty);
                    sem_wait(counter_kitchen_lock);
                    char *foodStr;

                    if (minFoodIndex == 1) // send soup to counter
                    {
                        sprintf(writeString, "soup - counter items P:%d, C:%d, D:%d = %d\n",
                                counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(counter_soupSize);
                        sem_post(numberOfSoup_ReplacedCounter);
                        foodStr = "soup";
                    }

                    else if (minFoodIndex == 2) // send main to counter
                    {
                        sprintf(writeString, "main course - counter items P:%d, C:%d, D:%d = %d\n",
                                counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(counter_mainSize);
                        sem_post(numberOfMain_ReplacedCounter);
                        foodStr = "main course";
                    }
                    else // send desert to counter
                    {
                        sprintf(writeString, "desert - counter items P:%d, C:%d, D: = %d\n",
                                counter_sizeOfSoup, counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                        sem_post(counter_desertSize);
                        sem_post(numberOfDesert_ReplacedCounter);
                        foodStr = "desert";
                    }

                    sem_getvalue(counter_soupSize, &counter_sizeOfSoup);
                    sem_getvalue(counter_mainSize, &counter_sizeOfMain);
                    sem_getvalue(counter_desertSize, &counter_sizeOfDesert);

                    int _processIndex;
                    _processIndex = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());

                    sem_post(counter_kitchen_lock);

                    sem_post(supply_lock);
                    sem_post(sem_kitchen_empty);

                    sem_getvalue(sem_kitchen_empty, &remaining_food);
                    sprintf(writeString, "Cook %d is placed %s on the counter - ", _processIndex, foodStr);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "counter items P:%d, C:%d, D:%d = %d\n", counter_sizeOfSoup,
                            counter_sizeOfMain, counter_sizeOfDesert, (counter_sizeOfSoup + counter_sizeOfMain + counter_sizeOfDesert));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sem_getvalue(sem_kitchen_empty, &remaining_food);
                    sprintf(writeString, "Cook %d finished serving - items at kitchen: %d - going home - GOODBYE!!!\n",
                            _processIndex, (K - remaining_food));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sem_post(cook_kitchen_lock);
                }
            }
        }

        // Student Part
        else
        {

            int limitOfStudent = 1;
            while (limitOfStudent <= L)
            {
                sem_post(numberOfStudentsAtCounter);
                sem_wait(student_lock);

                int studentId = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
                studentId = studentId - N;

                sem_post(student_lock);

                if (studentId < G)
                {
                    sem_wait(student_lock);
                    sem_wait(graduate_lock);
                }
                else
                {
                    sem_wait(student_lock);
                    sem_wait(undergraduate_lock);
                }

                int sizeOfEmptyTable;
                int counterSoup, counterMain, counterDesert;
                int studentVal, studentLeft;

                sem_getvalue(counter_soupSize, &counterSoup);
                sem_getvalue(counter_mainSize, &counterMain);
                sem_getvalue(counter_desertSize, &counterDesert);

                sem_getvalue(numberOfStudentsAtCounter, &studentVal);
                sem_getvalue(numberOfStudentsLeftCounter, &studentLeft);

                /*
                if (counterSoup == 0 || counterMain == 0 || counterDesert == 0)
                {
                    sprintf(writeString,"Student %d is waiting at the counter (round %d) - ",
                           studentId, limitOfStudent);
                    write(STDOUT_FILENO,writeString,strlen(writeString));
                    sprintf(writeString,"# of students at counter: %d\n", 1);
                    write(STDOUT_FILENO,writeString,strlen(writeString));
                }
                */

                //if one of these foods is zero at counter, dont change the value of limitOfStudents.
                if (counterSoup > 0 && counterMain > 0 && counterDesert > 0)
                {
                    sem_getvalue(numberOfStudentsAtCounter, &studentVal);
                    sem_getvalue(numberOfStudentsLeftCounter, &studentLeft);

                    if (studentId < G)
                    {
                        sprintf(writeString, "Graduate ");
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                    }

                    sprintf(writeString, "Student %d is going to the counter (round %d) - ",
                            studentId, limitOfStudent);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "# of students at counter: %d and counter items P:%d, C:%d, D:%d = %d\n",
                            (studentVal - studentLeft), counterSoup, counterMain, counterDesert, (counterSoup + counterMain + counterDesert));
                    write(STDOUT_FILENO, writeString, strlen(writeString));

                    int tempTableId;
                    sem_getvalue(table_empty, &sizeOfEmptyTable);

                    if (studentId < G)
                    {
                        sprintf(writeString, "Graduate ");
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                    }
                    sprintf(writeString, "Student %d got food and is going to get a table (round %d) - ",
                            studentId, limitOfStudent);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "# of empty tables: %d\n", sizeOfEmptyTable);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    /*
                    if (sizeOfEmptyTable == 0)
                    {
                        sprintf(writeString,"Student %d is waiting for a table (round %d) - ", studentId, limitOfStudent);
                        write(STDOUT_FILENO,writeString,strlen(writeString));
                        sprintf(writeString,"# of empty table : %d\n", sizeOfEmptyTable);
                        write(STDOUT_FILENO,writeString,strlen(writeString));
                    }
                    */

                    sem_wait(counter_soupSize);
                    sem_wait(counter_mainSize);
                    sem_wait(counter_desertSize);
                    sem_wait(counter_kitchen_lock);
                    sem_wait(table_empty);

                    sem_getvalue(counter_soupSize, &counterSoup);
                    sem_getvalue(counter_mainSize, &counterMain);
                    sem_getvalue(counter_desertSize, &counterDesert);
                    sem_getvalue(table_empty, &sizeOfEmptyTable);
                    if (studentId < G)
                    {
                        sprintf(writeString, "Graduate ");
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                    }
                    sprintf(writeString, "Student %d sat at table to eat (round %d) - ",
                            studentId, limitOfStudent);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    sprintf(writeString, "on counter P:%d, C:%d, D:%d, empty tables: %d\n",
                            counterSoup, counterMain, counterDesert, sizeOfEmptyTable);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    tempTableId = sizeOfEmptyTable;

                    sem_post(table_empty);

                    sem_getvalue(table_empty, &sizeOfEmptyTable);

                    if (studentId < G)
                    {
                        sprintf(writeString, "Graduate ");
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                    }
                    sprintf(writeString, "Student %d left table %d to eat again (round %d) - empty tables: %d\n",
                            studentId, tempTableId, limitOfStudent, sizeOfEmptyTable);
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                    //studentId = searchValueInArr(ProcessID_Array, numberOfChildren, getpid());
                    //studentId = studentId - N;

                    sem_post(counter_empty);
                    sem_post(counter_empty);
                    sem_post(counter_empty);
                    sem_post(numberOfStudentsLeftCounter);
                    sem_post(counter_kitchen_lock);

                    if (studentId < G)
                    {
                        sem_post(student_lock);
                        sem_post(graduate_lock);
                    }
                    else
                    {
                        sem_post(student_lock);
                        sem_post(undergraduate_lock);
                    }
                }
                else
                {
                    limitOfStudent--;
                    sem_post(numberOfStudentsLeftCounter);
                    if (studentId < G)
                    {
                        sem_post(student_lock);
                        sem_post(graduate_lock);
                    }
                    else
                    {
                        sem_post(undergraduate_lock);
                        sem_post(student_lock);
                    }
                }
                limitOfStudent++;
                if (limitOfStudent > L)
                {
                    if (studentId < G)
                    {
                        sprintf(writeString, "Graduate ");
                        write(STDOUT_FILENO, writeString, strlen(writeString));
                    }
                    sprintf(writeString, "Student %d is done eating L=%d times - going home - GOODBYE!!\n",
                            studentId, (limitOfStudent - 1));
                    write(STDOUT_FILENO, writeString, strlen(writeString));
                }
            }

            //exit(1);
        }
    }

    for (i = 0; i < M + N + 1; i++)
        ProcessID_Array[i] = wait(NULL);

    sem_destroy(sem_plate_Soup);
    sem_destroy(sem_plate_MainCourse);
    sem_destroy(sem_plate_Desert);

    sem_destroy(sem_kitchen_empty);
    sem_destroy(supply_lock);

    sem_destroy(numberOfSentSoup);
    sem_destroy(numberOfSentMainCourse);
    sem_destroy(numberOfSentDesert);

    sem_destroy(cook_kitchen_lock);
    sem_destroy(student_lock);
    sem_destroy(table_empty);

    sem_destroy(counter_kitchen_lock);
    sem_destroy(counter_empty);
    sem_destroy(counter_soupSize);
    sem_destroy(counter_mainSize);
    sem_destroy(counter_desertSize);

    sem_destroy(numberOfSoup_ReplacedCounter);
    sem_destroy(numberOfMain_ReplacedCounter);
    sem_destroy(numberOfDesert_ReplacedCounter);

    sem_destroy(undergraduate_lock);
    sem_destroy(graduate_lock);
    sem_destroy(numberOfStudentsAtCounter);
    sem_destroy(numberOfStudentsLeftCounter);
    free(ProcessID_Array);
    
    close(_inputfileSignal);

    exit(1);
}

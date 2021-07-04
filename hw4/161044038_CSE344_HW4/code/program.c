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
#include <pthread.h>

sem_t *chef_mutex, *supplier_mutex;

// this array will keep count of M, F, W and S
int m = 0, f = 1, w = 2, s = 3;
char ingredientNames[4][10] = {"milk", "flour", "walnuts", "sugar"};
int ingredientCount[4] = {0, 0, 0, 0};
int totalDoubleIngredient = 0;
int currentDoubleIngredient = 0;

void usage()
{
    printf("-o-o-o-o-o-o-o-o-o-\n\n");

    printf("./program -i filePath\n");
    printf("-i -> input file\n");

    printf("The file have to has valid content and at least 10 rows.\n");

    printf("-o-o-o-o-o-o-o-o-o-\n\n");
}

int generateRandNumber(int l, int u) 
{ 
    int num = (rand() % (u-l + 1)) + l;
    return num;
} 

int controlTheFile(int fd)
{
    char buffer[1];
    size_t _byteRead;
    int rowCounter = 0, doubleIngredient = 0;
    do
    {
        _byteRead = read(fd, buffer, sizeof(buffer));
        if (_byteRead != 1)
            break;

        if (buffer[0] == 'M' || buffer[0] == 'F' || buffer[0] == 'W' || buffer[0] == 'S')
        {
            doubleIngredient++;
            if (doubleIngredient == 2)
            {
                totalDoubleIngredient++;
                doubleIngredient = 0;
                rowCounter++;
            }
        }

        else if (buffer[0] == '\n')
        {
        }
        else
            return 0;

    } while (_byteRead == sizeof(buffer));

    lseek(fd, 0, SEEK_SET);

    // file have to has at least 10 rows
    if (rowCounter < 10)
        return 0;
    return 1;
}

void print_ingredientArr()
{
    printf("\n-o-o-o-o-o-o-o-o-o-o-o-\n");

    printf("M -> %d\n", ingredientCount[0]);
    printf("F -> %d\n", ingredientCount[1]);
    printf("W -> %d\n", ingredientCount[2]);
    printf("S -> %d\n", ingredientCount[3]);

    printf("\n-o-o-o-o-o-o-o-o-o-o-o-\n");
}

void *make_gullac(void *index_chef)
{

    while (1)
    {
        sem_wait(chef_mutex);

        if (currentDoubleIngredient == totalDoubleIngredient)
        {   
            sem_post(supplier_mutex);
            sem_post(chef_mutex);
            break;
        }

        int indexChef = *((int *)index_chef);

        //chef 1 needs W S
        if (indexChef == 1)
        {
            
            if (ingredientCount[w] > 0 && ingredientCount[s] > 0)
            {
                printf("chef%d is waiting for ", indexChef);
                printf("walnuts and sugar\n");
                ingredientCount[w]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[w]);
                ingredientCount[s]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[s]);
                sleep(generateRandNumber(1,5));
                printf("chef%d has delivered the dessert to the wholesaler\n", indexChef);
                sem_post(supplier_mutex);
            }
        }

        //chef 2 needs F S
        else if (indexChef == 2)
        {
            
            if (ingredientCount[f] > 0 && ingredientCount[s] > 0)
            {
                printf("chef%d is waiting for ", indexChef);
                printf("flour and sugar\n");
                ingredientCount[f]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[f]);
                ingredientCount[s]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[s]);
                sleep(generateRandNumber(1,5));
                printf("chef%d has delivered the dessert to the wholesaler\n", indexChef);
                sem_post(supplier_mutex);
            }
        }

        //chef 3 needs F W
        else if (indexChef == 3)
        {
            
            if (ingredientCount[f] > 0 && ingredientCount[w] > 0)
            {     
                printf("chef%d is waiting for ", indexChef);
                printf("flour and walnuts\n");
                ingredientCount[f]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[f]);
                ingredientCount[w]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[w]);
                sleep(generateRandNumber(1,5));
                printf("chef%d has delivered the dessert to the wholesaler\n", indexChef);
                sem_post(supplier_mutex);
            }
        }

        //chef 4 needs M S
        else if (indexChef == 4)
        {
            
            if (ingredientCount[m] > 0 && ingredientCount[s] > 0)
            {
                printf("chef%d is waiting for ", indexChef);
                printf("milk and sugar\n");
                ingredientCount[m]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[m]);
                ingredientCount[s]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[s]);
                sleep(generateRandNumber(1,5));
                printf("chef%d has delivered the dessert to the wholesaler\n", indexChef);
                sem_post(supplier_mutex);
            }
        }

        //chef 5 needs M W
        else if (indexChef == 5)
        {
            
            if (ingredientCount[m] > 0 && ingredientCount[w] > 0)
            {
                printf("chef%d is waiting for ", indexChef);
                printf("milk and walnuts\n");
                ingredientCount[m]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[m]);
                ingredientCount[w]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[w]);
                sleep(generateRandNumber(1,5));
                printf("chef%d has delivered the dessert to the wholesaler\n", indexChef);
                sem_post(supplier_mutex);
            }
        }

        //chef 6 needs M F
        else if (indexChef == 6)
        {
            
            if (ingredientCount[m] > 0 && ingredientCount[f] > 0)
            {
                printf("chef%d is waiting for ", indexChef);
                printf("milk and flour\n");
                ingredientCount[m]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[m]);
                ingredientCount[f]--;
                printf("chef%d has taken the %s\n", indexChef, ingredientNames[f]);

                sleep(generateRandNumber(1,5));
                printf("chef%d has delivered the dessert to the wholesaler\n", indexChef);
                sem_post(supplier_mutex);
            }
        }

        sem_post(chef_mutex);
    }
    
    return NULL;
}

int main(int argc, char *argv[])
{
    int i_counter=0;

    mode_t User_ReadWrite = S_IRUSR | S_IWUSR;

    char *inputFile = NULL;
    int _inputFileSignal;
    int isFileTrue = 0;
    int opt;
    int chefID = 0, i;

    pthread_t thread_chef[6];

    char *buffer;
    size_t byteread;

    int chefIndexes[6] = {1, 2, 3, 4, 5, 6};

    srand(time(0));

    while ((opt = getopt(argc, argv, "i:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            i_counter++;
            inputFile = optarg;
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

    _inputFileSignal = open(inputFile, O_RDONLY, User_ReadWrite);

    if (_inputFileSignal == -1)
    {
        usage();
        errno = EBADF;
        printf("Error no: %d(Could not open file)\n", errno);
        return 1;
    }

    isFileTrue = controlTheFile(_inputFileSignal);
    if (isFileTrue == 0)
    {
        usage();
        errno = EBADFD;
        printf("Error no: %d\n", errno);
        return 1;
    }

    chef_mutex = mmap(NULL, sizeof(*chef_mutex),
                      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                      -1, 0);
    supplier_mutex = mmap(NULL, sizeof(*supplier_mutex),
                          PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                          -1, 0);

    if (sem_init(chef_mutex, 0, 1) == -1)
        perror("sem_init");
    if (sem_init(supplier_mutex, 0, 0) == -1)
        perror("sem_init");

    while (chefID < 6)
    {
        if (pthread_create(&(thread_chef[chefID]), NULL, &make_gullac, &chefIndexes[chefID]))
        {
            usage();
            errno = EIO;
            printf("Error no: %d(error creating thread)\n", errno);
        }
        chefID++;
    }
    
    do
    {
        buffer = (char*) malloc(sizeof(char)*3);
        byteread = read(_inputFileSignal, buffer, 3);
        if (byteread != 3)
        {
            if(byteread != 2)
            {
                free(buffer);
                break;
            }
                
            if (buffer[0] == 'M')
                ingredientCount[m]++;
            else if (buffer[0] == 'F')
                ingredientCount[f]++;
            else if (buffer[0] == 'W')
                ingredientCount[w]++;
            else if (buffer[0] == 'S')
                ingredientCount[s]++;
            else
            {
                free(buffer);
                break;
            }
            
            if (buffer[1] == 'M')
                ingredientCount[m]++;
            else if (buffer[1] == 'F')
                ingredientCount[f]++;
            else if (buffer[1] == 'W')
                ingredientCount[w]++;
            else if (buffer[1] == 'S')
                ingredientCount[s]++;
            else
            {
                free(buffer);
                break;
            }
            
            sem_wait(supplier_mutex);
            currentDoubleIngredient++;
            printf("the wholesaler has obtained the dessert and left to sell it\n");
            free(buffer);
            break;
        }

            if (buffer[0] == 'M')
                ingredientCount[m]++;
            else if (buffer[0] == 'F')
                ingredientCount[f]++;
            else if (buffer[0] == 'W')
                ingredientCount[w]++;
            else if (buffer[0] == 'S')
                ingredientCount[s]++;
            else
            {
                free(buffer);
                break;
            }
            
            
            if (buffer[1] == 'M')
                ingredientCount[m]++;
            else if (buffer[1] == 'F')
                ingredientCount[f]++;
            else if (buffer[1] == 'W')
                ingredientCount[w]++;
            else if (buffer[1] == 'S')
                ingredientCount[s]++;
            else
            {
                free(buffer);
                break;
            }

            //print_ingredientArr();
            free(buffer);
            //it will wait for desert
            printf("the wholesaler is waiting for the dessert\n");
            
            sem_wait(supplier_mutex);
            currentDoubleIngredient++;
            printf("the wholesaler has obtained the dessert and left to sell it\n");
            
            
    } while (byteread == 3);

    for (i = 0; i < 6; i++)
    {
        if (pthread_join(thread_chef[i], NULL))
        {
            usage();
            errno = EIO;
            printf("Error no: %d(Error joining thread)\n", errno);
            return 1;
        }
    }

    sem_destroy(supplier_mutex);
    sem_destroy(chef_mutex);
    close(_inputFileSignal);
    
}
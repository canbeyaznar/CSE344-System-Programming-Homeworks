//Can BEYAZNAR
//161044038

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>

#include <sys/types.h>
#include <sys/wait.h>

char nameOfTempFile[] = "temp.txtXXXXXX";
char* inputfileName;
double *MeanResults;
double *SDResults;
int _resultCounter = 0;

int inputfilesignal;
int outputfilesignal;
int tempfilesignal;

char SignalsString[][30] = { 
"SIGHUP","SIGINT","SIGQUIT","SIGILL", "SIGTRAP",
"SIGABRT","SIGIOT","SIGBUS","SIGFPE","SIGKILL","SIGUSR1",
"SIGSEGV","SIGUSR2","SIGALRM","SIGCHLD","SIGCONT","SIGTSTP",
"SIGTTIN","SIGTTOU","SIGURG","SIGXCPU","SIGXFSZ","SIGVTALRM",
"SIGPROF","SIGWINCH","SIGIO","SIGPOLL"
};
int signalsQueue[100];
int signalCount = 0;
int isSignalPrintEnd = 0;


typedef struct ReturnInformation
{
    int numberOfReadBytes;
    int numberOfLineEquation;
    int *SignalInformations;
} _ReturnInformation;

typedef struct struct_2DCoord
{
    int x_coord;
    int y_coord;
} _struct_2DCoord;

typedef struct _equation
{
    double a;
    double b;
} equation;

void signalcounter(int signum)
{
    switch (signum)
    {
        case SIGHUP:
            signalsQueue[signalCount] = 0;
            signalCount++;
            break;
        case SIGINT:
            signalsQueue[signalCount] = 1;
            signalCount++;
            break;
        case SIGQUIT:
            signalsQueue[signalCount] = 2;
            signalCount++;
            break;
        case SIGILL:
            signalsQueue[signalCount] = 3;
            signalCount++;
            break;
        case SIGABRT:
            signalsQueue[signalCount] = 4;
            signalCount++;
            break;
            
        
        case SIGBUS:
            signalsQueue[signalCount] = 6;
            signalCount++;
            break;
        case SIGFPE:
            signalsQueue[signalCount] = 7;
            signalCount++;
            break;
    
    
        case SIGKILL:
            signalsQueue[signalCount] = 8;
            signalCount++;
            break;
        case SIGUSR1:
            signalsQueue[signalCount] = 9;
            signalCount++;
            break;
        case SIGSEGV:
            signalsQueue[signalCount] = 10;
            signalCount++;
            break;
        case SIGUSR2:
            signalsQueue[signalCount] = 11;
            signalCount++;
            break;
        case SIGALRM:
            signalsQueue[signalCount] = 12;
            signalCount++;
            break;
        case SIGCHLD:
            signalsQueue[signalCount] = 13;
            signalCount++;
            break;
        case SIGCONT:
            signalsQueue[signalCount] = 14;
            signalCount++;
            break;
        case SIGTSTP:
            signalsQueue[signalCount] = 15;
            signalCount++;
            break;
        


        case SIGTTIN:
            signalsQueue[signalCount] = 16;
            signalCount++;
            break;
        case SIGTTOU:
            signalsQueue[signalCount] = 17;
            signalCount++;
            break;
        case SIGURG:
            signalsQueue[signalCount] = 18;
            signalCount++;
            break;
        case SIGXCPU:
            signalsQueue[signalCount] = 19;
            signalCount++;
            break;
        case SIGXFSZ:
            signalsQueue[signalCount] = 20;
            signalCount++;
            break;
        case SIGVTALRM:
            signalsQueue[signalCount] = 21;
            signalCount++;
            break;
        case SIGPROF:
            signalsQueue[signalCount] = 22;
            signalCount++;
            break;
        case SIGWINCH:
            signalsQueue[signalCount] = 23;
            signalCount++;
            break;

        case SIGIO:
            signalsQueue[signalCount] = 24;
            signalCount++;
            break;
        
        
        default:
            break;
    }

   

}


void _counter()
{
    signal(SIGHUP,signalcounter);       
    signal(SIGINT,signalcounter);
    signal(SIGQUIT,signalcounter);
    signal(SIGILL,signalcounter);
    signal(SIGTRAP,signalcounter);
    signal(SIGABRT,signalcounter);
    signal(SIGIOT,signalcounter);
    signal(SIGBUS,signalcounter);
    signal(SIGFPE,signalcounter);
    signal(SIGKILL,signalcounter);
    signal(SIGUSR1,signalcounter);

    signal(SIGSEGV,signalcounter);       
    signal(SIGUSR2,signalcounter);
    signal(SIGPIPE,signalcounter);
    signal(SIGALRM,signalcounter);
    signal(SIGSTKFLT,signalcounter);
    signal(SIGCHLD,signalcounter);
    signal(SIGCONT,signalcounter);
    signal(SIGTSTP,signalcounter);
    signal(SIGTTIN,signalcounter);
    signal(SIGTTOU,signalcounter);

    signal(SIGURG,signalcounter);       
    signal(SIGXCPU,signalcounter);
    signal(SIGXFSZ,signalcounter);
    signal(SIGVTALRM,signalcounter);
    signal(SIGPROF,signalcounter);
    signal(SIGWINCH,signalcounter);
    signal(SIGIO,signalcounter);
    signal(SIGPOLL,signalcounter);

    

}


void printAllSignals()
{
    int i;
    printf("Sent signals :\n");
    for(i=0; i<signalCount; i++)
        printf(" -%s- ", SignalsString[ signalsQueue[i] ] );
    printf("\n");
    printf("Number of signals : %d\n",signalCount);
}
void catcher(int signum)
{
    switch (signum)
    {
    case SIGUSR1:

        break;
    case SIGUSR2:
        break;

    case SIGTERM:
    {   
        printf("Caught SIGTERM\n");
        close(inputfilesignal);
        close(tempfilesignal);
        close(outputfilesignal);

        unlink(nameOfTempFile);
        unlink(inputfileName);

        exit(0);
    }
    default:
        break;
    }
}

void printString_char(char *str, int len)
{
    int i;
    for (i = 0; i < len; i++)
        printf("%c", str[i]);
    printf("\n");
    return;
}

void printString_unsigned_int(unsigned int *arr, int len)
{
    int i;
    for (i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");
    return;
}

double StandardDeviation(double data[],int size) {
    double sum = 0.0;
    double mean;
    double result = 0.0;
    int i;
    for (i = 0; i < size; ++i) {
        sum += data[i];
    }
    mean = sum / size;
    for (i = 0; i < size; ++i)
        result += pow(data[i] - mean, 2);
    return sqrt(result / size);
}

double find_MAE(_struct_2DCoord coords[],equation myEquation,int size)
{
    int i;
    double result = 0.0;

    for(i=0; i<size; i++)
    {
        double temp = 0.0;
        temp = coords[i].y_coord - ( (myEquation.a * (double) coords[i].x_coord) + myEquation.b );
        if(temp < 0)
            temp *= -1;
        result += temp;  
    }
    result = result / (double) size;
    return result;
}

double find_MSE(_struct_2DCoord coords[],equation myEquation,int size)
{
    int i;
    double result = 0.0;

    for(i=0; i<size; i++)
    {
        double temp = 0.0;

        temp = coords[i].y_coord - ( (myEquation.a * (double) coords[i].x_coord) + myEquation.b );
        result = result + (temp * temp);  
    }
    result = result / (double) size;
    return result;
}

double find_RMSE(_struct_2DCoord coords[],equation myEquation,int size)
{
    double result = 0.0;
    result = find_MSE(coords, myEquation, size);
    result = sqrt(result);
    return result;
}

void fillEmptyCharArr(char arr[], int size)
{
    int i=0;
    for(i=0; i<size; i++)
        arr[i] = '\0';
}


int secondProcess_CalculateAndWrite(int input_fd, int output_fd, char *inputName)
{ 
    sigset_t mask;

    if ((sigemptyset(&mask) == -1) || sigaddset(&mask, SIGTSTP) == -1 || sigaddset(&mask, SIGINT) == -1)
    {
        perror("Failed to initialize signal mask");
        exit(1);
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
    {
        perror("Failed to block the signal mask");
        exit(1);
    }

    unsigned char _buffer[1];
    int byteCounter = 0;
    int control = 0;
    lseek(input_fd, 0, SEEK_SET);
    size_t _byteRead;

    char LineArray[300];
    int counter=0;
    
    int i;

    struct flock lock;
    struct stat stats;

    //char writeToFile[100];
    char *writeToFile = (char* ) malloc(sizeof(char)*100);
    char *remainingArray;
    int remainingSize;

    int firstRead = 0;
    int secondRead = 0;
    int numberOfCoords = 0;

    double MAE=0;
    double MSE=0;
    double RMSE=0;
    
    stat(nameOfTempFile, &stats);
    do
    {
        _byteRead = read(input_fd, _buffer, sizeof(_buffer));
        if (_buffer[0] == '\n')
            break;

        LineArray[counter] = _buffer[0];

        counter++;
        byteCounter = byteCounter + 1;
    } while (_byteRead == sizeof(_buffer));
    lseek(input_fd, 0, SEEK_SET);


    LineArray[counter] = '\0';

    remainingSize = stats.st_size-counter-1;
    
    if(remainingSize > 0)
    {
        remainingArray = (char*) malloc(sizeof(char)* remainingSize );

        lseek(input_fd, counter+1, SEEK_SET);
        _byteRead = read(input_fd, remainingArray, remainingSize);

        truncate(inputName,0);

        lseek(input_fd, 0, SEEK_SET);
        write(input_fd, remainingArray, strlen(remainingArray));
        free(remainingArray);
    }
    else
    {
        truncate(inputName,0);
    }
    
    

    _struct_2DCoord coords[10];
    equation myEquation;
    
    //this loop will read the LineArray and initialize the values...
    i=0;
    while(i<counter)
    {
        _struct_2DCoord temp;

        //for ax+b part
        if(numberOfCoords >= 10)
        {
            char *char_ax =(char*) malloc(sizeof(char) * 5);
            int firstCounter = 0;
            char *char_b=(char*) malloc(sizeof(char) * 5);
            int secondCounter = 0;

            // ax part
            i+=2;
            while(LineArray[i] != 'x')
            {
                char_ax[firstCounter] = LineArray[i];
                firstCounter++;
                i++;
            }
            i+=2;
            while(i<counter)
            {
                char_b[secondCounter] = LineArray[i];
                secondCounter++;
                i++;
            }

            
            myEquation.a = atof(char_ax);
            myEquation.b = atof(char_b);

            if(myEquation.a != myEquation.a)
                myEquation.a = 0.0;
            if(myEquation.b != myEquation.a)
                myEquation.b = 0.0;   

            free(char_ax);
            free(char_b);
            break;
        }

        if(LineArray[i] == '(')
        {   
            
            char *firstVal = (char*) malloc(sizeof(char) * 3);
            int firstCounter = 0;
            char *secondVal = (char*) malloc(sizeof(char) * 3);;
            int secondCounter = 0;

            i++;
            //first integer
            while(LineArray[i] != ',')
            {
                
                firstVal[firstCounter] = LineArray[i];
                firstCounter++;
                i++;
            }

            if(firstCounter<=2)
            {
                firstVal[firstCounter] = '\0';
            }

            i++;
            //second integer
            while(LineArray[i] != ')')
            {
                secondVal[secondCounter] = LineArray[i];
                secondCounter++;
                i++;
            }

            if(secondCounter<=2)
            {
                secondVal[secondCounter] = '\0';
            }

            temp.x_coord = atoi(firstVal);
            temp.y_coord = atoi(secondVal);
            

            coords[numberOfCoords] = temp;
            numberOfCoords++;

            free(firstVal);
            free(secondVal);
        }
        i++;
    }

    if(numberOfCoords < 10)
    {
        return -1;
    }
    

    MAE = find_MAE(coords,myEquation,10);
    MSE = find_MSE(coords,myEquation,10);
    RMSE = find_RMSE(coords,myEquation,10);

    double tempArr[3];
    double StandardDeviationResult=0;
    tempArr[0] = MAE;
    tempArr[1] = MSE;
    tempArr[2] = RMSE;

    StandardDeviationResult = StandardDeviation(tempArr,3);

    SDResults[_resultCounter] = StandardDeviationResult;

    MeanResults[_resultCounter] = (MAE+MSE+RMSE)/3;
    _resultCounter++;

    lseek(output_fd, 0, SEEK_END );
    write(output_fd, LineArray, strlen(LineArray) );
    
    
    sprintf(writeToFile,", %.3lf, %.3lf, %.3lf\n",MAE,MSE,RMSE);

    write(output_fd, writeToFile, strlen(writeToFile) );
    writeToFile = (char*) malloc(sizeof(char)*1);
    free(writeToFile);
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
    {
        perror("Failed to unblock the signal mask");
        exit(1);
    }
    
    
    return 0;
}

void calculateAndWriteToFile(unsigned int *Arr, int FileDescriptor, pid_t tempPid)
{
    sigset_t mask;
    
    if ((sigemptyset(&mask) == -1) || sigaddset(&mask, SIGTSTP) == -1 || sigaddset(&mask, SIGINT) == -1)
    {
        perror("Failed to initialize signal mask");
        exit(1);
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
    {
        perror("Failed to block the signal mask");
        exit(1);
    }
    

    // y = ax + b
    double sum_x1 = 0, sum_x2 = 0, sum_y1 = 0, sum_y2 = 0;
    double a, b;
    int numberOfEquations = 10;
    int i;
    char printFileResult[200];
    char *temp;

    for (i = 0; i < numberOfEquations * 2; i = i + 2)
    {
        temp = (char *)malloc(sizeof(char) * 50);
        sprintf(temp, "(%d,%d), ", Arr[i], Arr[i + 1]);
        lseek(FileDescriptor, 0, SEEK_END);
        write(FileDescriptor, temp, strlen(temp));
        free(temp);
    }

    for (i = 0; i < numberOfEquations * 2; i = i + 2)
    {
        sum_x1 = sum_x1 + Arr[i];
        sum_y1 = sum_y1 + Arr[i + 1];
        sum_x2 = sum_x2 + Arr[i] * Arr[i];
        sum_y2 = sum_y2 + Arr[i + 1] * Arr[i];
    }

    a = (numberOfEquations * sum_y2 - sum_x1 * sum_y1) / (numberOfEquations * sum_x2 - sum_x1 * sum_x1);
    b = (sum_x2 * sum_y1 - sum_y2 * sum_x1) / (numberOfEquations * sum_x2 - sum_x1 * sum_x1);

    if(a!=a)
        a=0;
    if(b!=b)
        b=0;

    sprintf(printFileResult, "%.3lfx+%.3lf\n", a, b);
    lseek(FileDescriptor, 0, SEEK_END);
    write(FileDescriptor, printFileResult, strlen(printFileResult));

    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
    {
        perror("Failed to unblock the signal mask");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int inputCounter = 0;
    int outputCounter = 0;
    int timeCounter = 0;

    char *inputFile = NULL;
    char *outputFile = NULL;
    char *timeParameter = NULL;

    mode_t User_ReadOnly = S_IRUSR;
    mode_t User_WriteOnly = S_IWUSR;
    mode_t User_ReadWrite = S_IRUSR | S_IWUSR;

    int _inputfileSignal;
    int _outputfileSignal;

    int testFileSignal;

    unsigned char buffer[20];
    size_t bytes_read;

    unsigned int *_20byteInput;

    int _20byteSize;
    int filledCounter;
    int i, j;

    int numberOfReadBytes = 0;
    int numberOfLineOfEquations = 0;


    int tempFileSignal = -99;

    struct flock lock;
    struct stat stats;

    int opt;

    //./programA -i inputPathA -o outputPathA -t time
    while ((opt = getopt(argc, argv, "i:o:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            inputCounter = inputCounter + 1;
            inputFile = optarg;
            break;

        case 'o':
            outputCounter = outputCounter + 1;
            outputFile = optarg;
            break;


        case '?':
            errno = 5;
            printf("unknown option: %c\n", optopt);
            break;

        default:
            errno = EIO;
            printf("Error no: %d\n", errno);
            return 1;
            break;
        }
    }

    if (inputCounter == 0 || outputCounter == 0)
    {
        errno = EIO;
        printf("Error no: %d\n", errno);
        return 1;
    }

    _inputfileSignal = open(inputFile, O_RDONLY, User_ReadWrite);

    if (_inputfileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n", errno);
        return 1;
    }

    _outputfileSignal = open(outputFile, O_RDWR | O_CREAT, User_ReadWrite);
    if (_outputfileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n", errno);
        return 1;
    }


    tempFileSignal = mkstemp(nameOfTempFile);

    inputfilesignal = _inputfileSignal;
    outputfilesignal = _outputfileSignal;
    tempfilesignal = tempFileSignal;
    inputfileName = inputFile;

    sigset_t sigset;
    struct sigaction sact;
    struct sigaction prevSact;

    sigset_t sigset2;
    struct sigaction sact2;
    struct sigaction prevSact2;

    signal(SIGTERM, catcher);

    pid_t pid;
    pid = fork();

    //Child process
    if (pid == 0)
    {

        int tempFileByteSize;
        
        sigemptyset(&sact.sa_mask);

        sact.sa_flags = 0;
        sact.sa_handler = catcher;

        sigaction(SIGUSR1, &sact, NULL);
        
        sigfillset(&sigset);
        sigdelset(&sigset, SIGUSR1);
        
        sigsuspend(&sigset);        
        
        lseek(tempFileSignal,0,SEEK_SET);
        stat(nameOfTempFile, &stats);
        tempFileByteSize = stats.st_size;

        
        struct stat statInputTemp;
        lseek(_inputfileSignal, 0, SEEK_SET);
        stat(inputFile,&statInputTemp);
        int numberOfLinesInputFile = statInputTemp.st_size / 20;

        MeanResults = (double*) malloc(sizeof(double)*numberOfLinesInputFile); 
        SDResults = (double*) malloc(sizeof(double)*numberOfLinesInputFile);
        int control =0;
        while(tempFileByteSize > 0)
        {
            lseek(tempFileSignal,0,SEEK_SET);
            memset(&lock, 0, sizeof(lock));
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_END;
            lock.l_start = 0;
            lock.l_len = stats.st_size;
            tempFileByteSize = stats.st_size;
            //fcntl(tempFileSignal, F_SETLKW, &lock);

            
            control = secondProcess_CalculateAndWrite(tempFileSignal, _outputfileSignal, nameOfTempFile);

            lock.l_type = F_UNLCK;
            //fcntl(tempFileSignal, F_SETLKW, &lock);

            if(control == -1)
            {
                truncate(nameOfTempFile,0);
                break;
            }
            lseek(tempFileSignal,0,SEEK_SET);
            stat(nameOfTempFile, &stats);
            tempFileByteSize = stats.st_size;
            
        }
        
        for(j=0; j<_resultCounter; j++)
        {
            printf("%d. Mean: %.3lf Standard Deviation: %.3lf\n",j,MeanResults[j],SDResults[j]);
        }

        free(MeanResults);
        free(SDResults);
        //unlink(nameOfTempFile);
        close(_inputfileSignal);
        unlink(inputFile);

        
        //wraise(SIGTERM);
        exit(0);
    }


    else
    {
        _counter();
        _20byteInput = (unsigned int *)malloc(sizeof(unsigned int) * (20));
        _20byteSize = 20;
        filledCounter = 0;
        do
        {
            /* Read the next line's worth of bytes*/
            bytes_read = read(_inputfileSignal, buffer, _20byteSize);        

            memset(&lock, 0, sizeof(lock));
            lock.l_type = F_RDLCK;
            lock.l_whence = SEEK_END;
            lock.l_start = 0;
            stat(nameOfTempFile, &stats);
            lock.l_len = stats.st_size;
            fcntl(tempFileSignal, F_SETLKW, &lock);

            for (i = 0; i < bytes_read; ++i)
            {
                
                if( buffer[i] < 0 || buffer[i] > 250  )
                    _20byteInput[filledCounter] = 10; 
                    
                else
                    _20byteInput[filledCounter] = buffer[i];

                filledCounter = filledCounter + 1;
                if (filledCounter == _20byteSize)
                {
                    //printString_unsigned_int(_20byteInput, 20);
                    numberOfReadBytes += bytes_read;
                    calculateAndWriteToFile(_20byteInput, tempFileSignal,pid);
                    numberOfLineOfEquations++;

                    filledCounter = 0;
                    free(_20byteInput);
                    _20byteInput = (unsigned int *)malloc(sizeof(unsigned int) * (20));
             
                    lock.l_type = F_UNLCK;
                    fcntl(tempFileSignal, F_SETLKW, &lock);                  
                }
            }    
            lock.l_type = F_UNLCK;
            fcntl(tempFileSignal, F_SETLKW, &lock);
        } while (bytes_read == 20);
  
        kill(pid, SIGUSR1);
        pid = wait(NULL);
        printf("\n\n-o-o-o-o-o-o- P1 is Terminated -o-o-o-o-o-o-\n\n");
        printf("Number of read bytes : %d\n",numberOfReadBytes);
        printf("Number of read line of equations : %d\n",numberOfLineOfEquations);
        printf("\n\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n\n ");
        
        printAllSignals();

        printf("\n\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n\n ");
        close(_inputfileSignal);
        close(_outputfileSignal);
        close(tempFileSignal);
    }
    

    raise(SIGTERM);

    
    return 0;
}
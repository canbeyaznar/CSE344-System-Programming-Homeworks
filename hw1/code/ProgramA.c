#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

int _time=0;

//I wrote my exit condition for ProgramB. If there is 2 bytes in file programB will exit...
char* exitBprogramFileName = "ExitControlForProgramB.txt";

void ClearArray(char Array[32], int size){
    int i;
    for(i=0; i<size; i++)
        Array[i] = '0';
}

void PrintArrayChar(char *Array, int size){
    int i;
    for(i=0; i<size; i++)
        printf("%c",Array[i]);
    printf("\n");
}

int findEmptyNewLine(int fd){
    
    unsigned char _buffer[1];
    int byteCounter=0;
    int control =0;
    lseek(fd, 0, SEEK_SET);
	size_t _byteRead;
    do{
		_byteRead = read(fd, _buffer, sizeof(_buffer));
        if( ((int) _buffer[0] == 10 ) && control==0 &&  byteCounter == 0 )
            return byteCounter;     
        if( ((int) _buffer[0] == 10 ) && control==1  )
            return byteCounter;      
        if((int) _buffer[0] == 10)
            control = control + 1;
        else
            control = 0;
        byteCounter = byteCounter+1;
	}while(_byteRead == sizeof(_buffer));
    lseek(fd, 0, SEEK_SET);
    return byteCounter;
}

int writeToTheFile(char* filename, int fd, int indexToWrite, char* Str)
{
    struct stat fileStats;
    stat(filename, &fileStats);
    char* copyOfRemainingFile;

    int fileSize = fileStats.st_size;

    printf("\n");
    //PrintArrayChar(Str,strlen(Str));
    if(Str == NULL)
        return -1;

    printf("\nfile size : %d\n",fileSize);
    printf("\nindexToWrite : %d\n", indexToWrite);

    if(indexToWrite>fileSize){
        lseek(fd,fileSize,SEEK_SET );
        write(fd, "\n",strlen("\n"));
    }
    indexToWrite = indexToWrite-1;
    lseek(fd, indexToWrite+1, SEEK_SET);

    //if the indexToWrite is in between two lines...
    if(fileSize - (indexToWrite+1) > 0  ){
        
        copyOfRemainingFile = (char*) malloc(sizeof(char) * (fileSize-indexToWrite) );
        read(fd, copyOfRemainingFile, (fileSize-indexToWrite) );
        PrintArrayChar(copyOfRemainingFile, (fileSize-indexToWrite));
        
        lseek(fd, indexToWrite+1,SEEK_SET);
        write(fd, Str,strlen(Str));
        lseek(fd, indexToWrite + strlen(Str) + 1, SEEK_SET);
        write(fd, copyOfRemainingFile,(fileSize-indexToWrite-1));
        //free(copyOfRemainingFile);
    }

    else{
        write(fd,Str,strlen(Str));
        write(fd,"\n",strlen("\n"));
    }

    return 0;
}

// This function will write to the file
void calculateAndWriteToTheFile(char* Filename, int FileSignal, char *Array, int ArrSize )
{
    
    int i;
    int firstAscii;
    int secondAscii;
    char *tempArr = malloc(sizeof(char)*(200));
    char *lastArray = malloc(sizeof(char)*(200));
    int indexOfEmptyline;
    struct flock lock;

    struct stat stats;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    stat(Filename,&stats );
    lock.l_len = stats.st_size;
    //fcntl(FileSignal, F_SETLKW,&lock);

    for(i=0; i<ArrSize; i=i+2)
    {   
        firstAscii = (int) Array[i];
        secondAscii = (int) Array[i+1];
        sprintf(tempArr,"%d +i%d,",firstAscii,secondAscii);
        strcat(lastArray, tempArr);
    }
    //strcat(lastArray,"\n");
    lseek(FileSignal,0,SEEK_END);
    indexOfEmptyline = findEmptyNewLine(FileSignal);
    writeToTheFile(Filename,FileSignal,indexOfEmptyline,lastArray);

    lock.l_type = F_UNLCK;
    usleep(_time*1000);
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
    int programBControl;

    unsigned char buffer[32];
	size_t bytes_read;
	int i;

    char *_32byteInput;
    int _32byteSize;
    int filledCounter;
    struct flock lock;

    //this will keep which file is opened for read or write
    char *fileRecorder = "fileRecorder.txt"; 

    int opt;
    //./programA -i inputPathA -o outputPathA -t time
    while( (opt = getopt(argc,argv, "i:o:t:")) != -1 )
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
            
            case 't':
                timeCounter = timeCounter + 1;
                timeParameter = optarg;
                break;

            case '?':
                errno = 5;
                printf("unknown option: %c\n", optopt);
                break;
                
            default:
                errno = EIO;
                printf("Error no: %d\n",errno);
                return 1;
                break;
        }
    }
    if (inputCounter == 0 || outputCounter == 0 || timeCounter == 0 )
    {
        errno = EIO;
        printf("Error no: %d\n",errno);
        return 1;
    }

    _time = atoi(timeParameter);
    if( !(( _time > 0) && ( _time < 51 )) ){
        errno = EIO;
        printf("Error no: %d\n",errno);
        return 1;
    }

    _inputfileSignal = open(inputFile, O_RDONLY, User_ReadWrite);

    if(_inputfileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n",errno);
        return 1;
    }

    struct stat stats;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    stat(inputFile,&stats );
    lock.l_len = stats.st_size;
    //fcntl(_outputfileSignal, F_SETLKW,&lock);


    _outputfileSignal = open(outputFile, O_RDWR | O_CREAT, User_ReadWrite);
    if(_outputfileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n",errno);
        return 1;
    }

    //Reading input file and parsing

    _32byteInput = malloc(sizeof(char)*(32));
    _32byteSize = 32;
    filledCounter = 0;
    do{
		/* Read the next line's worth of bytes*/
		bytes_read = read(_inputfileSignal, buffer, 32);
        
		for (i = 0; i < bytes_read; ++i)
		{           
            _32byteInput[filledCounter] = buffer[i];
            filledCounter = filledCounter + 1;
            if(filledCounter == 32)
            {
                //burada output dosyasına hesaplamalar yapılıp yazılmalıdır
                calculateAndWriteToTheFile(outputFile,_outputfileSignal, _32byteInput,32);
                filledCounter = 0;
                free(_32byteInput);
                _32byteInput = malloc(sizeof(char)*(32));
                
            }
		}
	}while(bytes_read == 32);

    close(_inputfileSignal);
    close(_outputfileSignal);

    if( strcmp(exitBprogramFileName,inputFile)!=0 && strcmp(exitBprogramFileName,outputFile)!=0 )
    {
        programBControl = open(exitBprogramFileName, O_WRONLY | O_CREAT,User_ReadWrite);
        lseek(programBControl, 0,SEEK_END);
        write(programBControl,"1",strlen("1"));
    }
    
    close(programBControl);
	return 0;
    
}


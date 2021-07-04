#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <complex.h>
#include <math.h>

//Global Variables...
int _time=-1;
double PI=3.141592;
int exitMainLoop=0;
char* exitBprogramFileName = "ExitControlForProgramB.txt";

struct _complex
{
    int reel;
    int img;
};

struct double_complex
{
    double d_re;
    double d_img;
};

void FFT_Recursive(complex input[], complex result[],int size, int stage);

int getRandomVal(int lower, int upper){
    return  ((rand() % (upper - lower + 1)) + lower); 
} 

void PrintArrayChar(char *Array, int size){
    int i;
    for(i=0; i<size; i++)
        printf("%c",Array[i]);
    printf("\n");
}

int findCountNewlinesOfFile(int fd){
    
    
    int counter=0;

    unsigned char _buffer[1];

    lseek(fd, 0, SEEK_SET);
	size_t _byteRead;

    if(fd == -1)
        return 0;
    do{
		_byteRead = read(fd, _buffer, sizeof(_buffer));
        if( (int) _buffer[0] == 10 )
            counter = counter + 1;

	}while(_byteRead == sizeof(_buffer));
    lseek(fd, 0, SEEK_SET);

    return counter;
}

char* getLineofFile(char* filename, int fd, int line){

    int LineCounter=0;

    unsigned char _buffer[1];
    lseek(fd, 0, SEEK_SET);
	size_t _byteRead;

    int firstflag=0;
    int secondflag=0;
    int isTargetFound=0;

    int SizeofCopiedStr;
    char* copiedString;
    char* remainingString;

    int i;
    int fileSize;
    struct stat fileStats;

    stat(filename,&fileStats);
    
    fileSize = fileStats.st_size;

    do{
        
        if(LineCounter == line && isTargetFound == 0)
        {
            secondflag = firstflag;
            isTargetFound = isTargetFound + 1;
        }
            
		_byteRead = read(fd, _buffer, sizeof(_buffer));

        if( (int) _buffer[0] == 10 )
        {
            if( isTargetFound > 0 )
                break;
            LineCounter = LineCounter + 1;
        }
            
        if( isTargetFound == 0 )
            firstflag = firstflag + 1;
        else
            secondflag = secondflag + 1;
        
	}while(_byteRead == sizeof(_buffer));
    lseek(fd, 0, SEEK_SET);

    // if there is \n (empty line) return null and increase random line parameter
    if (secondflag-firstflag==0 || firstflag> secondflag)
        return NULL;


    SizeofCopiedStr = secondflag - firstflag;
    copiedString = (char*) malloc(sizeof(char) * SizeofCopiedStr);

    lseek(fd, firstflag,SEEK_SET);
    read(fd, copiedString,SizeofCopiedStr);
    lseek(fd,0,SEEK_SET);

    PrintArrayChar(copiedString, SizeofCopiedStr);

    remainingString = (char* ) malloc(fileSize-secondflag);

    lseek(fd, secondflag,SEEK_SET);
    read(fd,remainingString, (fileSize-secondflag));

    lseek(fd,firstflag,SEEK_SET);

    ftruncate(fd,firstflag);
    write(fd, remainingString,strlen(remainingString));

    //free(remainingString);
    return copiedString;
}

struct _complex* defineComplexNumberofString(char* str){

    PrintArrayChar(str,strlen(str));
    struct _complex *resultArr;
    struct _complex tempComplex;
    int i, j, LengthStr, numberOfCompValues=0;
    LengthStr = strlen(str);

    char *reelarr = (char*) malloc(sizeof(char)* 3);
    char *imgarr = (char*) malloc(sizeof(char)* 3);

    int reelCount=0, imgCount=0;
    int reelOrImg = 0;
    int resultCounter=0;

    i=0;

    while(i<LengthStr){
        if(str[i] == '+')
            numberOfCompValues++; 
        i++;
    }

    if(numberOfCompValues == 0)
        return NULL;

    resultArr = (struct _complex*) malloc(sizeof(struct _complex) * numberOfCompValues);
    
    i=0;
    while(i<LengthStr){

        if( str[i] == ' ' || str[i] == '+' )
            reelOrImg = 0;
        else if( str[i] == 'i' )
            reelOrImg = 1;

        else if( isdigit(str[i]) )
        {
            if(reelOrImg == 0){
                reelarr[reelCount] = str[i];
                reelCount++;
            }

            else{
                imgarr[imgCount] = str[i];
                imgCount++;
            }
        }

        else if(str[i] == ','){
            
            resultArr[resultCounter].reel =atoi(reelarr);
            resultArr[resultCounter].img = atoi(imgarr);
            
            free(reelarr);
            free(imgarr);
            reelarr = (char*) malloc(sizeof(char)* 3);;
            imgarr = (char*) malloc(sizeof(char)* 3);;         
            resultCounter++;
            reelOrImg = 0;
            imgCount=0;
            reelCount=0;
        }            

        i++;
    }

    
    
    return resultArr;
}

int isFileEmpty(char* filename, int fd){

    int countofLines;
    int sizeOfFile;
    struct stat stats;
    
    stat(filename,&stats );

    sizeOfFile = stats.st_size;
    countofLines=findCountNewlinesOfFile(fd);

    if(sizeOfFile == countofLines-1)
        return 1;
    return 0;

}

struct _complex* returnRandomComplexValue(char* fileName,int fd){
    
    
    int countOfLine = findCountNewlinesOfFile(fd);
    int randLine = getRandomVal(0,countOfLine-1);
    int readLine = randLine;
    char* str = NULL;
    int control;

    control = isFileEmpty(fileName,fd);
    if(control==1)
    {
        exitMainLoop=1;
        return NULL;
    }


    while(str == NULL)
    {   
        if(readLine == countOfLine-1)
            return NULL;
            
        str = getLineofFile(fileName, fd, readLine);
        
        if(str != NULL)
            break;
        readLine = readLine + 1;
    }
    
    if(str == NULL)
    {
        errno = EBADFD;
        printf("Error no: %d",errno);
        return NULL;
    }
    
    struct _complex *myComplexNumbers;
    complex resultComplexArr[16];
    char* tempChar;
    double number;

    PrintArrayChar(str,strlen(str));
    myComplexNumbers = defineComplexNumberofString(str);
    
    //could not read exit
    if(myComplexNumbers == NULL)
    {
        errno = EBADFD;
        printf("Error no: %d",errno);
        return NULL;
    }


    return myComplexNumbers;
}

void print_complex(struct _complex* input,int size){
    for(int i=0; i<size; i++)
        printf("%d,i%d\n",input[i].reel, input[i].img);
}

void FFT_Recursive(complex input[], complex result[],int size, int stage){

    if(stage < size){
        FFT_Recursive(result, input,size , stage*2);
        FFT_Recursive(result+stage,result + stage, size, stage*2);

        for(int i=0; i<size; i = i + (2*stage)){
            complex temp = cexp(-I*PI*i/size)*result[i+stage];
            input[i/2] = result[i]+temp;
            input[ (i+size)/2 ] = result[i]-temp;
        }
    }

}

void FastFourierTransform(complex input[], int size){
    complex result[size];
    int i;
    for(i=0; i<size; i++)
        result[i] = input[i];
    FFT_Recursive(input, result,size,1);
}


void printToTheFile(char* filename, int fd,complex input[], int size){

    char* sendString = (char*) malloc(sizeof(char)*300);
    char* tempString;
    int emptyLine;

    int i;
    for(i=0; i<size; i++){
        tempString = (char*) malloc(sizeof(char)*20);
        sprintf(tempString,"%.3lf +i%.3lf,",creal(input[i]),cimag(input[i]) );
        strcat(sendString,tempString);
    }
    strcat(sendString,"\n");
    lseek(fd,0,SEEK_END);
    write(fd, sendString,strlen(sendString));

}

int readControlFile(char* filename){
    int programBControl;
    mode_t User_ReadWrite = S_IRUSR | S_IWUSR;
    programBControl = open(exitBprogramFileName, O_RDONLY ,User_ReadWrite);

    struct stat stats;
    stat(filename,&stats );

    if(stats.st_size >= 2)
        return 2;
    else
        return 0;
    close(programBControl);
    
}

int filterCharacter(int fd, char Character){
    unsigned char _buffer[1];
    int byteCounter=0;
    int control =0;
    lseek(fd, 0, SEEK_SET);
	size_t _byteRead;
    int lastChar=0;
    do{
		_byteRead = read(fd, _buffer, sizeof(_buffer));
        if(_buffer[0] == Character)
            lastChar=byteCounter;
        byteCounter = byteCounter+1;
	}while(_byteRead == sizeof(_buffer));
    lseek(fd, 0, SEEK_SET);
    ftruncate(fd,lastChar+1);
}

int main(int argc, char *argv[]){

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

    unsigned char buffer[32];
	size_t bytes_read;
	int i;
    complex randomComplexValue[16];
    struct _complex* tempcomplex;
    int countOfNewLine=0;
    double number;
    char* tempChar;
    struct flock lock;
    struct stat stats;
    srand(time(0));
    int Exit =0;
    int control=0;

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

    _inputfileSignal = open(inputFile, O_RDWR, User_ReadWrite);

    if(_inputfileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n",errno);
        return 1;
    }

    _outputfileSignal = open(outputFile, O_RDWR | O_CREAT, User_ReadWrite);
    if(_outputfileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n",errno);
        return 1;
    }

    
    struct stat inputFileStat;
    stat(inputFile, &inputFileStat);
    int tempControl = inputFileStat.st_size;
    while (tempControl<1)
    {
        stat(inputFile, &inputFileStat);
        tempControl = inputFileStat.st_size;
    }
    
    int programBControl = open(exitBprogramFileName, O_RDONLY ,User_ReadWrite);
    if(programBControl == -1) 
        return 0;

    struct stat programBControlStat;
    while(Exit==0)
    {   
        Exit = programBControlStat.st_size;
        if(Exit >= 2)
            return 0;
        else
        {
            Exit=0; 
            int deneme = filterCharacter(_inputfileSignal,',');
            stat(exitBprogramFileName, &programBControlStat);   
        }
                  
        
        countOfNewLine = findCountNewlinesOfFile(_inputfileSignal);

        printf("%d ",countOfNewLine);
        tempcomplex= returnRandomComplexValue(inputFile, _inputfileSignal);

        
        memset(&lock,0,sizeof(lock));
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_END;
        lock.l_start = 0;
        stat(inputFile,&stats );
        lock.l_len = stats.st_size;
        //fcntl(_inputfileSignal, F_SETLKW,&lock);
        

        if(tempcomplex != NULL)
        {

            for(int i=0; i<16; i++)
            {
                tempChar = (char*) malloc(sizeof(char)*10);
                sprintf(tempChar,"%d.%d",tempcomplex[i].reel,tempcomplex[i].img);
                number = (double) atof(tempChar);
                randomComplexValue[i] = number;
                //free(tempChar);
            }
            
            if(randomComplexValue == NULL){
                errno = EBADFD;
                printf("Error no: %d\n",errno);
                return 1;
            }

            for(int i=0; i<16; i++)
            {
                printf("%.3lf +i%.3lf,\n",creal(randomComplexValue[i]),cimag(randomComplexValue[i]));
            }
            FastFourierTransform(randomComplexValue,16);
            stat(exitBprogramFileName, &programBControlStat);
            Exit = programBControlStat.st_size;
            if(Exit >= 2)
                return 0;
            else
                Exit=0;
            printf("\n-----------------After---------------\n");
            for(int i=0; i<16; i++)
                printf("%.3lf +i%.3lf,",creal(randomComplexValue[i]),cimag(randomComplexValue[i]));
            
            printToTheFile(outputFile,_outputfileSignal,randomComplexValue,16);
        }
        else{
            usleep(_time*1000);
            if(exitMainLoop == 1)
            {
                exitMainLoop=0;
                sleep(_time*1000);
            }
                
            else
            {   
                errno=EBADF;
                printf("Error no: %d",errno);
                
            }
        }
        control++;
        
    }
    close(programBControl);
    close(_inputfileSignal);
    close(_outputfileSignal);
}
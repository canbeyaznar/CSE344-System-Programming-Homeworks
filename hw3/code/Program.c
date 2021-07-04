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
#include <math.h>
#include <sys/wait.h>
#include <signal.h>

#define NR_END 1
#define FREE_ARG char*
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
(dmaxarg1) : (dmaxarg2))
static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
(iminarg1) : (iminarg2))

int counterSIGCHILD = 0;

double **dmatrix(int nrl, int nrh, int ncl, int nch)
{
	int i,nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	m += NR_END;
	m -= nrl;
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	m[nrl] += NR_END;
	m[nrl] -= ncl;
	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;
	return m;
}

double *dvector(int nl, int nh)
{
	double *v;
	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	return v-nl+NR_END;
}

void free_dvector(double *v, int nl, int nh)
{
	free((FREE_ARG) (v+nl-NR_END));
}

double pythag(double a, double b)
{
	double absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb) return absa*sqrt(1.0+(absb/absa)*(absb/absa));
	else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+(absa/absb)*(absa/absb)));
}
/*******************************************************************************
Given a matrix a[1..m][1..n], this routine computes its singular value
decomposition, A = U.W.VT.  The matrix U replaces a on output.  The diagonal
matrix of singular values W is output as a vector w[1..n].  The matrix V (not
the transpose VT) is output as v[1..n][1..n].
*******************************************************************************/
void svdcmp(double **a, int m, int n, double w[], double **v)
{
	int flag,i,its,j,jj,k,l,nm;
	double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

	rv1=dvector(1,n);
	g=scale=anorm=0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(a[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f=a[i][i];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
					f=s/h;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		anorm = DMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++) /* Double division to avoid possible underflow. */
					v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=IMIN(m,n);i>=1;i--) {
        l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
				f=(s/a[i][i])*g;
				for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else for (j=i;j<=m;j++) a[j][i]=0.0;
		++a[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ((double)(fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((double)(fabs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ((double)(fabs(f)+anorm) == anorm) break;
					g=w[i];
					h=pythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=a[j][nm];
						z=a[j][i];
						a[j][nm]=y*c+z*s;
						a[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) { 
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			if (its == 30) printf("no convergence in 30 svdcmp iterations");
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0; /* Next QR transformation: */
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=pythag(f,h);
				w[j]=z; /* Rotation can be arbitrary if z = 0. */
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	free_dvector(rv1,1,n);
}

void catcher(int signal)
{

    switch (signal)
    {
    case SIGINT:
    {
        printf("\nCALLED SIGNAL : SIGINT\n");
        raise(SIGTERM);
        break;
    }

    case SIGCHLD:
    {
        printf("SIGCHILD\n");
        counterSIGCHILD++;
        break;
    }

    default:
    {
        break;
    }
    }
}

void ClearArray(char Array[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        Array[i] = '0';
}

void PrintArrayChar(char *Array, int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%c", Array[i]);
    printf("\n");
}

void multiplication(int *firstInput[], int *secondInput[], int *output[], int _size)
{

    int i, j, k;

    for (i = 0; i < _size; i++)
    {
        for (j = 0; j < _size; j++)
        {
            output[i][j] = 0;
            for (k = 0; k < _size; k++)
                output[i][j] += firstInput[i][k] * secondInput[k][j];
        }
    }
}

int *calculateAndReturnTheResult(unsigned char *inputMatrixes, int eachMatrixSize, int ArrSize)
{
    int rootMatrixSize = (int)sqrt(eachMatrixSize);
    if (eachMatrixSize == 1)
    {
        rootMatrixSize = 1;
        int firstInput = (int)inputMatrixes[0];
        int secondInput = (int)inputMatrixes[1];
        int *outputArr = (int *)malloc(sizeof(int) * 1);
        outputArr[0] = firstInput * secondInput;
        return outputArr;
    }

    int **firstInput = (int **)malloc(sizeof(int *) * rootMatrixSize);
    int **secondInput = (int **)malloc(sizeof(int *) * rootMatrixSize);
    int **outputArr = (int **)malloc(sizeof(int *) * rootMatrixSize);
    int *outputSTR = (int *)malloc(sizeof(int) * (rootMatrixSize * rootMatrixSize));

    int i, j, k;

    for (i = 0; i < rootMatrixSize; i++)
    {
        firstInput[i] = (int *)malloc(sizeof(int) * rootMatrixSize);
        secondInput[i] = (int *)malloc(sizeof(int) * rootMatrixSize);
        outputArr[i] = (int *)malloc(sizeof(int) * rootMatrixSize);
    }

    k = 0;
    for (i = 0; i < rootMatrixSize; i++)
    {
        for (j = 0; j < rootMatrixSize; j++)
        {
            firstInput[i][j] = (int)inputMatrixes[k];
            secondInput[i][j] = (int)inputMatrixes[eachMatrixSize + k];
            k++;
        }
    }

    multiplication(firstInput, secondInput, outputArr, rootMatrixSize);

    k = 0;
    for (i = 0; i < rootMatrixSize; i++)
    {
        for (j = 0; j < rootMatrixSize; j++)
        {
            int temp = outputArr[i][j];
            outputSTR[k] = temp;
            k++;
        }
    }

    for (i = 0; i < rootMatrixSize; i++)
    {
        int *temp = firstInput[i];
        free(temp);

        temp = secondInput[i];
        free(temp);

        temp = outputArr[i];
        free(temp);
    }
    free(firstInput);
    free(secondInput);
    free(outputArr);

    return outputSTR;
}

//this function will split the integer numbers with " "
int *parseStringToIntArr(char *arr, int numberOfIntegers)
{
    int arrsize = strlen(arr);
    char delim[] = " ";
    int *result = (int *)malloc(sizeof(int) * numberOfIntegers);
    int i = 0;

    char *ptr = strtok(arr, delim);

    while (ptr != NULL)
    {
        result[i] = atoi(ptr);
        if (result[i] != result[i])
            result[i] = 0;
        ptr = strtok(NULL, delim);
        i++;
    }
    return result;
}

int main(int argc, char *argv[])
{

    int firstInputCounter = 0;
    int secondInputCounter = 0;
    int n_Counter = 0;

    char *firstInputFile = NULL;
    char *secondInputFile = NULL;
    char *n_Parameter = NULL;

    mode_t User_ReadOnly = S_IRUSR;
    mode_t User_WriteOnly = S_IWUSR;
    mode_t User_ReadWrite = S_IRUSR | S_IWUSR;

    int _FirstInputFileSignal;
    int _SecondInputFileSignal;
    int programBControl;

    char *buffer;
    size_t firstBytes_read;
    size_t secondBytes_read;

    int i, j, k, n_valueWithPow = 0, sizeOfMatrix = 0;

    unsigned char *firstInput;
    unsigned char *secondInput;

    unsigned char **firstMatrix = NULL;
    unsigned char **secondMatrix = NULL;
    int **outputMatrix;

    int n_input;

    int opt;

    //./program -i inputPathA -j inputPathB -n 8
    while ((opt = getopt(argc, argv, "i:j:n:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            firstInputCounter = firstInputCounter + 1;
            firstInputFile = optarg;
            break;

        case 'j':
            secondInputCounter = secondInputCounter + 1;
            secondInputFile = optarg;
            break;

        case 'n':
            n_Counter = n_Counter + 1;
            n_Parameter = optarg;
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
    if (firstInputCounter != 1 && secondInputCounter != 1 && n_Counter != 1)
    {
        errno = EIO;
        printf("Error no: %d\n", errno);
        return 1;
    }

    n_input = atoi(n_Parameter);
    if ((n_input <= 0) || n_input != n_input)
    {
        errno = EIO;
        printf("Error no: %d (n is equal or less than 0)\n", errno);
        return 1;
    }

    n_valueWithPow = pow(2, n_input);
    sizeOfMatrix = n_valueWithPow * n_valueWithPow;

    if (strcmp(firstInputFile, secondInputFile) == 0)
    {
        errno = EBADF;
        printf("Error no: %d (Same Input Files!!)\n", errno);
        return 1;
    }

    _FirstInputFileSignal = open(firstInputFile, O_RDONLY, User_ReadWrite);

    if (_FirstInputFileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n", errno);
        return 1;
    }

    _SecondInputFileSignal = open(secondInputFile, O_RDONLY, User_ReadWrite);

    if (_SecondInputFileSignal == -1)
    {
        errno = EBADF;
        printf("Error no: %d\n", errno);
        return 1;
    }

    firstInput = (unsigned char *)malloc(sizeof(unsigned char) * sizeOfMatrix);
    secondInput = (unsigned char *)malloc(sizeof(unsigned char) * sizeOfMatrix);

    firstBytes_read = read(_FirstInputFileSignal, firstInput, sizeOfMatrix);
    if (firstBytes_read != sizeOfMatrix)
    {
        errno = EIO;
        printf("Error no: %d (Not enough byte in %s) \n", errno, firstInputFile);
        return 1;
    }

    secondBytes_read = read(_SecondInputFileSignal, secondInput, sizeOfMatrix);
    if (secondBytes_read != sizeOfMatrix)
    {
        errno = EIO;
        printf("Error no: %d (Not enough byte in %s) \n", errno, secondInputFile);
        return 1;
    }

    firstMatrix = (unsigned char **)malloc(sizeof(unsigned char *) * n_valueWithPow);
    secondMatrix = (unsigned char **)malloc(sizeof(unsigned char *) * n_valueWithPow);
    outputMatrix = (int **)malloc(sizeof(int *) * n_valueWithPow);

    if (firstMatrix == NULL || secondMatrix == NULL || outputMatrix == NULL)
        return 1;
    for (i = 0; i < n_valueWithPow; i++)
    {
        firstMatrix[i] = (unsigned char *)malloc(sizeof(unsigned char) * n_valueWithPow);
        secondMatrix[i] = (unsigned char *)malloc(sizeof(unsigned char) * n_valueWithPow);
        outputMatrix[i] = (int *)malloc(sizeof(int) * n_valueWithPow);
    }

    k = 0;
    for (i = 0; i < n_valueWithPow; i++)
    {
        for (j = 0; j < n_valueWithPow; j++)
        {
            firstMatrix[i][j] = firstInput[k];
            secondMatrix[i][j] = secondInput[k];
            k++;
        }
    }

    printf("\nFirst Matrix : \n");
    for (i = 0; i < n_valueWithPow; i++)
    {
        for (j = 0; j < n_valueWithPow; j++)
            printf(" %c ", firstMatrix[i][j]);
        printf("\n");
    }
    printf("\n");

    printf("\nSecond Matrix : \n");
    for (i = 0; i < n_valueWithPow; i++)
    {
        for (j = 0; j < n_valueWithPow; j++)
            printf(" %c ", secondMatrix[i][j]);
        printf("\n");
    }
    printf("\n");

    for (i = 0; i < n_valueWithPow; i++)
    {
        for (j = 0; j < n_valueWithPow; j++)
            outputMatrix[i][j] = 0;
    }

    close(_FirstInputFileSignal);
    close(_SecondInputFileSignal);

    int fd_p1W_p2R[2];
    int fd_p1R_p2W[2];

    int fd_p1W_p3R[2];
    int fd_p1R_p3W[2];

    int fd_p1W_p4R[2];
    int fd_p1R_p4W[2];

    int fd_p1W_p5R[2];
    int fd_p1R_p5W[2];

    if (pipe(fd_p1W_p2R) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    if (pipe(fd_p1R_p2W) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    if (pipe(fd_p1W_p3R) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    if (pipe(fd_p1R_p3W) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    if (pipe(fd_p1W_p4R) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    if (pipe(fd_p1R_p4W) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }
    if (pipe(fd_p1W_p5R) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    if (pipe(fd_p1R_p5W) == -1)
    {
        errno = ESTRPIPE;
        printf("Error no: %d\n", errno);
        return 1;
    }

    int x, x2;
    int startMatrix, endMatrix;
    int x_s, x_f, y_s, y_f;
    int quarterSizeOfMatrix = ((n_valueWithPow / 2) * (n_valueWithPow / 2));
    unsigned char *tempStr;
    int *firstQuarter;
    int *secondQuarter;
    int tempStrCounter = 0;
    pid_t p1 = getpid();
    pid_t p2, p3, p4, p5;

    signal(SIGINT, catcher);
    signal(SIGCHLD, catcher);

    p2 = fork();
    //parent yazar child okur
    switch (p2)
    {
    case -1:

        break;
    case 0: //child
    {
        //close write
        if (close(fd_p1W_p2R[1]) == -1)
            return 1;
        if (close(fd_p1R_p2W[0]) == -1)
            return 1;

        unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix * 2));
        unsigned char *sendOutput = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix));
        x = read(fd_p1W_p2R[0], buf, (quarterSizeOfMatrix * 2));
        firstQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

        x = read(fd_p1W_p2R[0], buf, (quarterSizeOfMatrix * 2));
        secondQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

        j = 0;
        k = 0;

        for (i = 0; i < quarterSizeOfMatrix; i++)
        {
            char *_temp = (char *)malloc(sizeof(char) * 20);
            sprintf(_temp, "%d ", firstQuarter[i] + secondQuarter[i]);
            strcat(sendOutput, _temp);
            //outputMatrix[j][k] = firstQuarter[i] + secondQuarter[i];
            k++;
            if (k == n_valueWithPow / 2)
            {
                k = 0;
                j++;
            }
        }
        write(fd_p1R_p2W[1], sendOutput, strlen(sendOutput));
        break;
    }
    default:
    {

        char *firstQuarterOutput = (char *)malloc(sizeof(char) * quarterSizeOfMatrix * 20);
        if (close(fd_p1W_p2R[0]) == -1)
            return 1;
        if (close(fd_p1R_p2W[1]) == -1)
            return 1;
        tempStr = (unsigned char *)malloc(sizeof(unsigned char) * (n_valueWithPow) * (n_valueWithPow));

        x_s = 0;
        x_f = n_valueWithPow / 2;
        y_s = 0;
        y_f = n_valueWithPow / 2;

        for (i = y_s; i < y_f; i++)
        {
            for (j = x_s; j < x_f; j++)
            {
                tempStr[tempStrCounter] = firstMatrix[i][j];
                tempStrCounter++;
            }
        }

        for (i = y_s; i < y_f; i++)
        {
            for (j = x_s; j < x_f; j++)
            {
                tempStr[tempStrCounter] = secondMatrix[i][j];
                tempStrCounter++;
            }
        }

        x_s = n_valueWithPow / 2;
        x_f = n_valueWithPow;
        y_s = 0;
        y_f = n_valueWithPow / 2;

        for (i = y_s; i < y_f; i++)
        {
            for (j = x_s; j < x_f; j++)
            {
                tempStr[tempStrCounter] = firstMatrix[i][j];
                tempStrCounter++;
            }
        }

        x_s = 0;
        x_f = n_valueWithPow / 2;
        y_s = n_valueWithPow / 2;
        y_f = n_valueWithPow;

        for (i = y_s; i < y_f; i++)
        {
            for (j = x_s; j < x_f; j++)
            {
                tempStr[tempStrCounter] = secondMatrix[i][j];
                tempStrCounter++;
            }
        }

        write(fd_p1W_p2R[1], tempStr, strlen(tempStr));

        read(fd_p1R_p2W[0], firstQuarterOutput, quarterSizeOfMatrix * 20);

        int *firstQuarter = parseStringToIntArr(firstQuarterOutput, quarterSizeOfMatrix);

        x_s = 0;
        x_f = n_valueWithPow / 2;
        y_s = 0;
        y_f = n_valueWithPow / 2;
        k = 0;
        for (i = y_s; i < y_f; i++)
        {
            for (j = x_s; j < x_f; j++)
            {
                outputMatrix[i][j] = firstQuarter[k];
                k++;
            }
        }

        p3 = fork();
        switch (p3)
        {

        case -1:
            break;
        case 0:
        {
            //close write
            if (close(fd_p1W_p3R[1]) == -1)
                return 1;
            if (close(fd_p1R_p3W[0]) == -1)
                return 1;

            unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix * 2));
            unsigned char *sendOutput = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix));
            x = read(fd_p1W_p3R[0], buf, (quarterSizeOfMatrix * 2));
            firstQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

            x = read(fd_p1W_p3R[0], buf, (quarterSizeOfMatrix * 2));
            secondQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

            j = 0;
            k = 0;

            for (i = 0; i < quarterSizeOfMatrix; i++)
            {
                char *_temp = (char *)malloc(sizeof(char) * 20);
                sprintf(_temp, "%d ", firstQuarter[i] + secondQuarter[i]);
                strcat(sendOutput, _temp);
                //outputMatrix[j][k] = firstQuarter[i] + secondQuarter[i];
                k++;
                if (k == n_valueWithPow / 2)
                {
                    k = 0;
                    j++;
                }
            }
            write(fd_p1R_p3W[1], sendOutput, strlen(sendOutput));
            break;
        }

        default:
        {
            char *secondQuarterOutput = (char *)malloc(sizeof(char) * quarterSizeOfMatrix * 20);
            if (close(fd_p1W_p3R[0]) == -1)
                return 1;
            if (close(fd_p1R_p3W[1]) == -1)
                return 1;
            tempStr = (unsigned char *)malloc(sizeof(unsigned char) * (n_valueWithPow) * (n_valueWithPow));
            tempStrCounter = 0;
            x_s = 0;
            x_f = n_valueWithPow / 2;
            y_s = 0;
            y_f = n_valueWithPow / 2;

            //A11
            for (i = y_s; i < y_f; i++)
            {
                for (j = x_s; j < x_f; j++)
                {
                    tempStr[tempStrCounter] = firstMatrix[i][j];
                    tempStrCounter++;
                }
            }

            //B12
            x_s = n_valueWithPow / 2;
            x_f = n_valueWithPow;
            y_s = 0;
            y_f = n_valueWithPow / 2;

            for (i = y_s; i < y_f; i++)
            {
                for (j = x_s; j < x_f; j++)
                {
                    tempStr[tempStrCounter] = secondMatrix[i][j];
                    tempStrCounter++;
                }
            }

            //A12
            x_s = n_valueWithPow / 2;
            x_f = n_valueWithPow;
            y_s = 0;
            y_f = n_valueWithPow / 2;

            for (i = y_s; i < y_f; i++)
            {
                for (j = x_s; j < x_f; j++)
                {
                    tempStr[tempStrCounter] = firstMatrix[i][j];
                    tempStrCounter++;
                }
            }

            //B22
            x_s = n_valueWithPow / 2;
            x_f = n_valueWithPow;
            y_s = n_valueWithPow / 2;
            y_f = n_valueWithPow;

            for (i = y_s; i < y_f; i++)
            {
                for (j = x_s; j < x_f; j++)
                {
                    tempStr[tempStrCounter] = secondMatrix[i][j];

                    tempStrCounter++;
                }
            }
            write(fd_p1W_p3R[1], tempStr, strlen(tempStr));

            read(fd_p1R_p3W[0], secondQuarterOutput, quarterSizeOfMatrix * 20);

            int *firstQuarter = parseStringToIntArr(secondQuarterOutput, quarterSizeOfMatrix);

            x_s = n_valueWithPow / 2;
            x_f = n_valueWithPow;
            y_s = 0;
            y_f = n_valueWithPow / 2;
            k = 0;
            for (i = y_s; i < y_f; i++)
            {
                for (j = x_s; j < x_f; j++)
                {
                    outputMatrix[i][j] = firstQuarter[k];
                    k++;
                }
            }

            p4 = fork();
            switch (p4)
            {

            case -1:
                break;
            case 0:
            {
                //close write
                if (close(fd_p1W_p4R[1]) == -1)
                    return 1;
                if (close(fd_p1R_p4W[0]) == -1)
                    return 1;

                unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix * 2));
                unsigned char *sendOutput = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix));
                x = read(fd_p1W_p4R[0], buf, (quarterSizeOfMatrix * 2));
                firstQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

                x = read(fd_p1W_p4R[0], buf, (quarterSizeOfMatrix * 2));
                secondQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

                j = 0;
                k = 0;

                for (i = 0; i < quarterSizeOfMatrix; i++)
                {
                    char *_temp = (char *)malloc(sizeof(char) * 20);
                    sprintf(_temp, "%d ", firstQuarter[i] + secondQuarter[i]);
                    strcat(sendOutput, _temp);
                    k++;
                    if (k == n_valueWithPow / 2)
                    {
                        k = 0;
                        j++;
                    }
                }
                write(fd_p1R_p4W[1], sendOutput, strlen(sendOutput));
                break;
            }

            default:
            {
                char *thirdQuarterOutput = (char *)malloc(sizeof(char) * quarterSizeOfMatrix * 20);
                if (close(fd_p1W_p4R[0]) == -1)
                    return 1;
                if (close(fd_p1R_p4W[1]) == -1)
                    return 1;
                tempStr = (unsigned char *)malloc(sizeof(unsigned char) * (n_valueWithPow) * (n_valueWithPow));
                tempStrCounter = 0;
                x_s = 0;
                x_f = n_valueWithPow / 2;
                y_s = n_valueWithPow / 2;
                y_f = n_valueWithPow;

                //A21
                for (i = y_s; i < y_f; i++)
                {
                    for (j = x_s; j < x_f; j++)
                    {
                        tempStr[tempStrCounter] = firstMatrix[i][j];
                        tempStrCounter++;
                    }
                }

                //B11
                x_s = 0;
                x_f = n_valueWithPow / 2;
                y_s = 0;
                y_f = n_valueWithPow / 2;

                for (i = y_s; i < y_f; i++)
                {
                    for (j = x_s; j < x_f; j++)
                    {
                        tempStr[tempStrCounter] = secondMatrix[i][j];
                        tempStrCounter++;
                    }
                }

                //A22
                x_s = n_valueWithPow / 2;
                x_f = n_valueWithPow;
                y_s = n_valueWithPow / 2;
                y_f = n_valueWithPow;

                for (i = y_s; i < y_f; i++)
                {
                    for (j = x_s; j < x_f; j++)
                    {
                        tempStr[tempStrCounter] = firstMatrix[i][j];
                        tempStrCounter++;
                    }
                }

                //B21
                x_s = 0;
                x_f = n_valueWithPow / 2;
                y_s = n_valueWithPow / 2;
                y_f = n_valueWithPow;

                for (i = y_s; i < y_f; i++)
                {
                    for (j = x_s; j < x_f; j++)
                    {
                        tempStr[tempStrCounter] = secondMatrix[i][j];
                        tempStrCounter++;
                    }
                }
                write(fd_p1W_p4R[1], tempStr, strlen(tempStr));

                read(fd_p1R_p4W[0], thirdQuarterOutput, quarterSizeOfMatrix * 20);

                int *thirdQuarter = parseStringToIntArr(thirdQuarterOutput, quarterSizeOfMatrix);

                x_s = 0;
                x_f = n_valueWithPow / 2;
                y_s = n_valueWithPow / 2;
                y_f = n_valueWithPow;
                k = 0;
                for (i = y_s; i < y_f; i++)
                {
                    for (j = x_s; j < x_f; j++)
                    {
                        outputMatrix[i][j] = thirdQuarter[k];
                        k++;
                    }
                }

                //FOR P5
                p5 = fork();
                switch (p5)
                {

                case -1:
                    break;
                case 0:
                {
                    //close write
                    if (close(fd_p1W_p5R[1]) == -1)
                        return 1;
                    if (close(fd_p1R_p5W[0]) == -1)
                        return 1;

                    unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix * 2));
                    unsigned char *sendOutput = (unsigned char *)malloc(sizeof(unsigned char) * (quarterSizeOfMatrix));
                    x = read(fd_p1W_p5R[0], buf, (quarterSizeOfMatrix * 2));
                    firstQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

                    x = read(fd_p1W_p5R[0], buf, (quarterSizeOfMatrix * 2));
                    secondQuarter = calculateAndReturnTheResult(buf, quarterSizeOfMatrix, (quarterSizeOfMatrix * 2));

                    j = 0;
                    k = 0;

                    for (i = 0; i < quarterSizeOfMatrix; i++)
                    {
                        char *_temp = (char *)malloc(sizeof(char) * 20);
                        sprintf(_temp, "%d ", firstQuarter[i] + secondQuarter[i]);
                        strcat(sendOutput, _temp);
                        //outputMatrix[j][k] = firstQuarter[i] + secondQuarter[i];
                        k++;
                        if (k == n_valueWithPow / 2)
                        {
                            k = 0;
                            j++;
                        }
                    }
                    write(fd_p1R_p5W[1], sendOutput, strlen(sendOutput));
                    break;
                }

                default:
                {
                    char *fourthQuarterOutput = (char *)malloc(sizeof(char) * quarterSizeOfMatrix * 20);
                    if (close(fd_p1W_p5R[0]) == -1)
                        return 1;
                    if (close(fd_p1R_p5W[1]) == -1)
                        return 1;
                    tempStr = (unsigned char *)malloc(sizeof(unsigned char) * (n_valueWithPow) * (n_valueWithPow));
                    tempStrCounter = 0;
                    x_s = 0;
                    x_f = n_valueWithPow / 2;
                    y_s = n_valueWithPow / 2;
                    y_f = n_valueWithPow;

                    //A21
                    for (i = y_s; i < y_f; i++)
                    {
                        for (j = x_s; j < x_f; j++)
                        {
                            tempStr[tempStrCounter] = firstMatrix[i][j];
                            tempStrCounter++;
                        }
                    }

                    //B12
                    x_s = n_valueWithPow / 2;
                    x_f = n_valueWithPow;
                    y_s = 0;
                    y_f = n_valueWithPow / 2;

                    for (i = y_s; i < y_f; i++)
                    {
                        for (j = x_s; j < x_f; j++)
                        {
                            tempStr[tempStrCounter] = secondMatrix[i][j];
                            tempStrCounter++;
                        }
                    }

                    //A22
                    x_s = n_valueWithPow / 2;
                    x_f = n_valueWithPow;
                    y_s = n_valueWithPow / 2;
                    y_f = n_valueWithPow;

                    for (i = y_s; i < y_f; i++)
                    {
                        for (j = x_s; j < x_f; j++)
                        {
                            tempStr[tempStrCounter] = firstMatrix[i][j];
                            tempStrCounter++;
                        }
                    }

                    //B22
                    x_s = n_valueWithPow / 2;
                    x_f = n_valueWithPow;
                    y_s = n_valueWithPow / 2;
                    y_f = n_valueWithPow;

                    for (i = y_s; i < y_f; i++)
                    {
                        for (j = x_s; j < x_f; j++)
                        {
                            tempStr[tempStrCounter] = secondMatrix[i][j];
                            tempStrCounter++;
                        }
                    }
                    write(fd_p1W_p5R[1], tempStr, strlen(tempStr));
                    read(fd_p1R_p5W[0], fourthQuarterOutput, quarterSizeOfMatrix * 20);

                    int *fourthQuarter = parseStringToIntArr(fourthQuarterOutput, quarterSizeOfMatrix);

                    x_s = n_valueWithPow / 2;
                    x_f = n_valueWithPow;
                    y_s = n_valueWithPow / 2;
                    y_f = n_valueWithPow;
                    k = 0;
                    for (i = y_s; i < y_f; i++)
                    {
                        for (j = x_s; j < x_f; j++)
                        {
                            outputMatrix[i][j] = fourthQuarter[k];
                            k++;
                        }
                    }
                    break;
                }
                }
                break;
            }
            }
            break;
        }
        }

        p2 = wait(NULL);
        p3 = wait(NULL);
        p4 = wait(NULL);
        p5 = wait(NULL);

        if (p1 == getpid())
        {
            printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
            printf("Output Matrix : \n\n");
            for (i = 0; i < n_valueWithPow; i++)
            {
                for (j = 0; j < n_valueWithPow; j++)
                    printf(" %d ", outputMatrix[i][j]);
                printf("\n");
            }
            printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");

            int _n_ = n_valueWithPow + 1;
            double **a = (double**) malloc(sizeof(double*)* _n_ );
            for(i=0; i<_n_; i++)
                a[i] = (double*) malloc(sizeof(double)* _n_ );

            double w[_n_];
            double **v= (double **)malloc(_n_ * sizeof(double *)); 
            for (i=0; i<_n_; i++) 
                v[i] = (double *)malloc(_n_* sizeof(double)); 

            for(i=1; i<_n_; i++)
            {
                for(j=1; j<_n_; j++)
                    a[i][j] = (double) outputMatrix[i-1][j-1];
            }
            svdcmp(a,_n_-1,_n_-1,w,v);
            printf("Singular Value Result : \n");
            for(i=1; i<_n_; i++){
                printf("%.3lf ",w[i]);
            }
            printf("\n");

            for(i=0; i<_n_; i++)
            {
                double *temp = a[i];
                free(temp);

                temp = v[i];
                free(temp);
            }
            free(a);
            free(v);

            for (i = 0; i < n_valueWithPow; i++)
            {
                unsigned char *temp = firstMatrix[i];
                int *temp2;
                free(temp);

                temp = secondMatrix[i];
                free(temp);

                temp2 = outputMatrix[i];
                free(temp2);
            }

            free(tempStr);
            free(firstMatrix);
            free(secondMatrix);
            free(outputMatrix);

            raise(SIGTERM);
        }
        break;
    }
    }

    for (i = 0; i < n_valueWithPow; i++)
    {
        unsigned char *temp = firstMatrix[i];
        int *temp2;
        free(temp);

        temp = secondMatrix[i];
        free(temp);

        temp2 = outputMatrix[i];
        free(temp2);
    }

    free(tempStr);
    free(firstMatrix);
    free(secondMatrix);
    free(outputMatrix);
}

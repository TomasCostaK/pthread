#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "helperfuncs.h"
#include "cross_relation.h"

extern int *statusWorker;

static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


static int numberOfFiles;
static char *files[10];
static FILE *file;
static int currIndex = 0;
static int signalSize = 0; // default signal size, we need to change this
static int finishedTexts = 0;
struct PartialInfo *finalInfo;

// returns the int of the next file
int openNextFile(){

    // If the partialInfo class is not empty, store the results in the given file
    if(currIndex != 0 && finalInfo[currIndex-1].signal_size == 0) storeResults();
    
    finalInfo[currIndex].signal_size = 0;
    finalInfo[currIndex].x = (double*)malloc(sizeof(double*));
    finalInfo[currIndex].y = (double*)malloc(sizeof(double*));
    finalInfo[currIndex].xy = (double*)malloc(sizeof(double*));
    finalInfo[currIndex].xy_true = (double*)malloc(sizeof(double*));

    if (numberOfFiles == currIndex){
        finishedTexts = 1;
        return -1; //special index, indicating no more files to read
    }

    printf("---------------OPENED %s-------------- \n", files[currIndex]);
    file = fopen(files[currIndex], "rb+"); 

    //After we open a file, we can load into memory the signals:
    fread(signalSize, sizeof(int), 1, file);   
    printf("Signal Size: %d\n", signalSize);

    // Wont we have problems here??
    // Reading X array of doubles
    fread(finalInfo[currIndex].x, sizeof(double [signalSize]), 1, file);    
    // Reading Y array of doubles
    fread(finalInfo[currIndex].y, sizeof(double [signalSize]), 1, file);
    // Reading true values of XY
    fread(finalInfo[currIndex].xy_true, sizeof(double [signalSize]), 1, file);

    currIndex++;
    return currIndex - 1;
}

// we only write in the end since threads are not ordered and we want to write the results in a given order
void storeResults(){

    
}

void storeFileNames(int nfileNames, char *fileNames[] )
{
    int nextfile;
    numberOfFiles = nfileNames;                     //number of files

    for (int i = 0; i<nfileNames; i++){
        files[i] = fileNames[i];
    }

    openNextFile();
}

int processConvPoint(int threadId, int fileId, int n, double * x, double * y, int point){

    // This condition means we have reached the end of the file, so the next point we want to process is
    if (signalSize == point+1) openNextFile();

    int status;
    if (!finishedTexts)    //work is not over
    {

        double result = 0;
        // open file, read signal, update signal, read x, read y, read xy_true and process xy outside lock
        // Getting the signal size
        result = computeValue(n, x, y, point);

        // Saving partil results makes using of locks
        savePartialResults(threadId, currIndex, point, result);
        status = 0;

    }
    else
        status = 2; // status 2 == endProcess

    return status;
}

double computeValue(int n, double * x, double * y, int point){
    double result = 0;
    // Circular cross
    for (int i=0; i<point; i++){
        for (int k=0; k<point; k++){
            result += x[k] * y[(i+k) % signalSize];
        }
    }
    return result;
}

void savePartialResults(int threadId, int fileId, int point, double val){
    // Here we need the lock to write partial results from PartialInfo to FinalInfo
    // Only after partial results are saved, that we can save final results

    if ((statusWorker[threadId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    printf("THREAD %d aquired lock\n", threadId);

    // Actual writing to struct
    finalInfo[fileId].xy[point] = val;

    printf("THREAD %d realesed lock\n", threadId);

    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }

}



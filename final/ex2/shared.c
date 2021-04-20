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
static int chunkSize = 75;
static int finishedTexts = 0;
struct PartialInfo finalInfo;

void openNextFile(){
    
    finalInfo.signal_size = 0;
    finalInfo.x = (double*)malloc(sizeof(double*));
    finalInfo.y = (double*)malloc(sizeof(double*));
    finalInfo.xy = (double*)malloc(sizeof(double*));
    finalInfo.xy_true = (double*)malloc(sizeof(double*));

    if (numberOfFiles == currIndex){
        finishedTexts = 1;
        return;
    }

    printf("---------------OPENED %s-------------- \n", files[currIndex]);
    file = fopen(files[currIndex], "rb"); 
    currIndex++;
}

void storeFileNames(int nfileNames, char *fileNames[] )
{
    numberOfFiles = nfileNames;                     //number of files

    for (int i = 0; i<nfileNames; i++){
        files[i] = fileNames[i];
    }

    openNextFile();
}

int processConvPoint(int threadId, int *fileId, int *n, double* x, double* y, int* point)
{

    if ((statusWorker[threadId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    printf("THREAD %d aquired lock\n", threadId);

    int status;
    if (!finishedTexts)    //work is not over
    {
        status = 0;
        memset(buff, 0, sizeof buff);   //clear buffer

        int character;
        while(1) {

            character = fgetc(file);

            if (character == EOF){  //File has ended
                printf("Finished\n");
                fclose(file);
                openNextFile();
                break;
            }

            strcat(buff, &character);

            if (strlen(buff)>chunkSize && stopChars(buff[strlen(buff) - 1])){   //Fill buffer with complete words
                break;
            }

            for (int i = 0; i < numberOfBytesInChar((unsigned char)character) - 1; i++) {
                character = fgetc(file);
                strcat(buff, &character);
            }
        }
    }
    else
        status = 2;

    printf("THREAD %d realesed lock\n", threadId);

    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    return status;
}

void savePartialResults(int threadID, struct PartialInfo partialInfo){
    return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "helperfuncs.h"
#include "main.h"

extern int *statusWorker;

static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


static int numberOfFiles;
static char *files[10];
static FILE *file;
static int currIndex = -1;
static int chunkSize = 75;
static int finishedTexts = 0;
//struct PartialInfo finalInfo;
struct PartialInfo finalInfo[10];

void openNextFile(){

    if (numberOfFiles-1 == currIndex){    //Finished all the texts
        finishedTexts = 1;
        return;
    }

    currIndex++;
    file = fopen(files[currIndex], "r"); 
    printf("---------------OPENED %s-------------- \n", files[currIndex]);

}

void storeFileNames(int nfileNames, char *fileNames[] )
{
    numberOfFiles = nfileNames;       //number of files

    for (int i = 0; i<nfileNames; i++){
        files[i] = fileNames[i];    //file name

        //Prepare matrix that will store the data
        finalInfo[i].nwords = 0;
        finalInfo[i].data = (int**)malloc(sizeof(int*));
        finalInfo[i].data[0] = (int*)malloc(sizeof(int));
        finalInfo[i].data[0][0] = 0;
        finalInfo[i].rows = 1;
    }

    

    

    openNextFile();
}

int getDataChunk(int threadId, char buff[], struct PartialInfo *partialInfo)
{

    if ((statusWorker[threadId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    //printf("THREAD %d aquired lock\n", threadId);

    int status;
    if (!finishedTexts)    //work is not over
    {
        status = 0;
        partialInfo->textInd = currIndex;   //need to tell worker from which text info is so that we know where to save it later

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

    //printf("THREAD %d realesed lock\n", threadId);

    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    return status;
}

void savePartialResults(int threadId, struct PartialInfo partialInfo){

    if ((statusWorker[threadId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    //printf("THREAD %d aquired lock\n", threadId);
    printf("TEXT: %d\n", partialInfo.textInd);
    printf("partialInfo rows: %d\n", partialInfo.rows);
    printf("final rows: %d\n", finalInfo[partialInfo.textInd].rows);
    printf("----------\n");

    finalInfo[partialInfo.textInd].data = prepareMatrix(&finalInfo[partialInfo.textInd].rows, partialInfo.rows-1, finalInfo[partialInfo.textInd].data);   //realloc matrix if necessary
    finalInfo[partialInfo.textInd].nwords += partialInfo.nwords; //total number of words
    for (int row = 1; row < partialInfo.rows; row++){   //row represents number of chars in word (row 2 = words with 2 chars)
        

        for (int col = 0; col < partialInfo.rows + 1; col++){   //col represents number of consonants (if row = 2, col will get the value of 0, 1 and 2)
            finalInfo[partialInfo.textInd].data[row][col] += partialInfo.data[row][col];
        }
        finalInfo[partialInfo.textInd].data[row][row+1] += partialInfo.data[row][row+1]; //the number of words is stored in the last cell of the array

    }


    //printf("THREAD %d realesed lock\n", threadId);
    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
}

void printProcessingResults(){

}



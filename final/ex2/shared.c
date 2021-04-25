#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "cross_relation.h"

extern int *statusWorker;

static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

static int numberOfFiles;
static char *files[15];
static FILE *file [15];
static int currFile = 0;
static int currIndex = 0;
static int finishedTexts = 0;
struct PartialInfo finalInfo[15];

void storeFileNames(int nfileNames, char *fileNames[] )
{
    int nextfile;
    numberOfFiles = nfileNames;                     //number of files

    for (int i = 0; i<nfileNames; i++){
        files[i] = fileNames[i];
    }

    openNextFile();
}

// returns the int of the next file
void openNextFile(){

    if (numberOfFiles == currFile){
        printf("\nAll texts read\n");
        finishedTexts = 1;
        return;
    }

    currIndex = 0;

    printf("---------------OPENED %s-------------- \n", files[currFile]);
    file[currFile] = fopen(files[currFile], "ab+"); 

    int sig_size = 0;
    //After we open a file, we can load into memory the signals:
    fread(&sig_size, sizeof(int), 1, file[currFile]);
    finalInfo[currFile].signal_size = sig_size;
    printf("Signal Size: %d\n", finalInfo[currFile].signal_size);

    finalInfo[currFile].x = (double*)malloc(sizeof(double [sig_size]));
    finalInfo[currFile].y = (double*)malloc(sizeof(double [sig_size]));
    finalInfo[currFile].xy = (double*)malloc(sizeof(double [sig_size]));
    finalInfo[currFile].xy_true = (double*)malloc(sizeof(double [sig_size]));

    // Wont we have problems here??
    // Reading X array of doubles
    fread(finalInfo[currFile].x, sizeof(double [sig_size]), 1, file[currFile]);
    printf("X[0]: %f\n", finalInfo[currFile].x[0]);
    
    // Reading Y array of doubles
    fread(finalInfo[currFile].y, sizeof(double [sig_size]), 1, file[currFile]);
    printf("Y[0]: %f\n", finalInfo[currFile].y[0]);

    // Reading true values of XY
    fread(finalInfo[currFile].xy_true, sizeof(double [sig_size]), 1, file[currFile]);
    printf("XY_TRUE[0]: %f\n", finalInfo[currFile].xy_true[0]);

    // Reading true values of XY
    fread(finalInfo[currFile].xy, sizeof(double [sig_size]), 1, file[currFile]);
    //printf("XY[0]: %f\n", finalInfo[currFile].xy[0]);

    return;
}

int processConvPoint(int threadId, int* fileId, int* n, double ** x, double ** y, int * point){

    if ((statusWorker[threadId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    //printf("THREAD %d acquired lock on ProcessConvPoint\n", threadId);
    //printf("SingalSize = %d \t currIndex = %d\n", finalInfo[currFile].signal_size, currIndex);

    // This condition means we have reached the end of the file, so the next point we want to process is
    if (finalInfo[currFile].signal_size == currIndex) {
        currFile++;
        openNextFile();
    }

    int status;
    if (!finishedTexts)    //work is not over
    {

        // Writing to the variables we need to
        *fileId = currFile;
        *point = currIndex;
        *n = finalInfo[currFile].signal_size;
        *x = finalInfo[currFile].x;
        *y = finalInfo[currFile].y;
        currIndex++;
        status = 0;


    }
    else
        status = 2; // status 2 == endProcess

    //printf("THREAD %d released lock on ProcessConvPoint, status=%d\n\n", threadId, status);
    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }

    return status;
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
    //printf("THREAD %d aquired lock on SavingPartialResults\n", threadId);

    // Actual writing to struct
    finalInfo[fileId].xy[point] = val;
    //printf("Saving value[%d] == %f on file %d\n", point, val, fileId);

    //printf("THREAD %d released lock on SavingPartialResults\n\n", threadId);
    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }

}

// we only write in the end since threads are not ordered and we want to write the results in a given order
// this is already inside the lock, so its fine
void storeResults(){
    /* These values are already present in the file, we only need to append our calculations
    fwrite(&finalInfo[currFile].signal_size, sizeof(int), 1, files[currFile]);
    fwrite(&finalInfo[currFile].x, sizeof(double [finalInfo[currFile].signal_size]), 1, files[currFile]);
    fwrite(&finalInfo[currFile].y, sizeof(double [finalInfo[currFile].signal_size]), 1, files[currFile]);
    fwrite(&finalInfo[currFile].xy_true, sizeof(double [finalInfo[currFile].signal_size]), 1, files[currFile]);
    */
    for (int i = 0; i < currFile; i++)
    {
        //printf("Writing values for file %d: XY[0] == %f\n", i, finalInfo[i].xy[0]);
        fwrite(finalInfo[i].xy, sizeof(double [finalInfo[i].signal_size]), 1, file[i]);
        fclose(file[i]);
    }
}

void checkProcessingResults(){
    printf("\n----------------------------------\nChecking results for %d files\n", currFile);
    FILE * file_tmp;
    for (int i = 0; i < currFile; i++)
    {
        file_tmp = fopen(files[i], "rb");
        int count = 0;

        int signal_size;
        fread(&signal_size, sizeof(int), 1, file_tmp);    

        double x[signal_size], y[signal_size], xy[signal_size], xy_true[signal_size];

        fread(&x, sizeof(double [signal_size]), 1, file_tmp);    
        fread(&y, sizeof(double [signal_size]), 1, file_tmp);    
        fread(&xy_true, sizeof(double [signal_size]), 1, file_tmp);    

        fread(&xy, sizeof(double [signal_size]), 1, file_tmp);    

        for (int k = 0; k < signal_size; k++)
        {
            if (xy[k] != xy_true[k]) {
                printf("Values differ on idx: %d, \t RAW=%f  !=  PROCESSED=%f\n", k, xy_true[k], xy[k]);
                count++;
            }
        }

        if (count == 0) printf("All values are the same for file: %s\n", files[i]);
        
    }
    
}
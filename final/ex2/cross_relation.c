#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "cross_relation.h"

int *statusWorker;

static void *work (int tid);

// auxiliary function to check if writing was done correctly
int** check_file(FILE *file){
    int signal_size;
    fread(&signal_size, sizeof(int), 1, file);    

    printf("\n");
    double x[signal_size], y[signal_size], xy[signal_size];

    fread(&x, sizeof(double [signal_size]), 1, file);    
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("WRITTEN X[%d] = %f\n", j, x[j]);
    }

    fread(&y, sizeof(double [signal_size]), 1, file);    
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("WRITTEN Y[%d] = %f\n", j, y[j]);
    }

    fread(&xy, sizeof(double [signal_size]), 1, file);    
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("WRITTEN XY[%d] = %f\n", j, xy[j]);
    }

    return 0;
}

int** process_signal(FILE *file){
    // Getting the signal size
    int signal_size;
    fread(&signal_size, sizeof(int), 1, file);    
    printf("Signal Size: %d\n", signal_size);

    double x[signal_size], y[signal_size], xy[signal_size];

    // Reading X array of doubles
    fread(&x, sizeof(double [signal_size]), 1, file);    
    // Reading Y array of doubles
    fread(&y, sizeof(double [signal_size]), 1, file);    

    double result = 0;
    // Circular cross
    for (int i=0; i<signal_size; i++){
        for (int k=0; k<signal_size; k++){
            result += x[k] * y[(i+k) % signal_size];
        }
        xy[i] = result;
        //write to file
        fwrite(&result, sizeof(double), 1, file);
        result = 0;
    }

    // Append to file
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("XY[%d] = %f\n", j, xy[j]);
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[])
{   
    char *files[10];    
    int fi = 0;
    int threads = 0;

    int c;
    while ((c = getopt (argc, argv, "t:i:")) !=-1){
        switch (c)
        {
        case 't':
            threads = atoi(optarg);
            break;
        case 'i':
            files[fi] = optarg;
            fi++;
            break;
        default:
            break;
        }
    }

    statusWorker = malloc(sizeof(int)*threads);

    pthread_t tIdworker[threads];
    unsigned int workers[threads];
    int *status_p;                                                                      /* pointer to execution status */

    for (int t = 0; t < threads; t++)
        workers[t] = t;


    storeFileNames(fi, files );

    //---------------THREADS
    for (int t = 0; t < threads; t++){
        //create(t)
        if (pthread_create (&tIdworker[t], NULL, work, workers[t]) != 0)                              /* thread producer */
        { 
            perror ("error on creating thread worker");
            exit (EXIT_FAILURE);
        }
    }
    for (int t = 0; t < threads; t++){
        if (pthread_join (tIdworker[t], (void *) &status_p) != 0)                                       /* thread producer */
        { 
            perror ("error on waiting for thread worker");
            exit (EXIT_FAILURE);
        }
        
        printf ("thread worker, with id %d, has terminated: ", t);
        printf ("its status was %d\n", *status_p);
    }
/*
    printProcessingResults ();
    getTime (&t1);
    printProcessingTime (t1 – t0);
    
    char buff[100];  
    getDataChunk(1, buff);
    printf(buff);
*/

}

static void *work(int tid){

    int id = tid;
    char buff[100];
    printf("Thread %d created \n", id);

    while (getDataChunk(id, buff) != 2){
        //make triangular matrix
        struct PartialInfo partialInfo;
        partialInfo.signal_size = 0;
        partialInfo.x = (double*)malloc(sizeof(double*));
        partialInfo.y = (double*)malloc(sizeof(double*));
        partialInfo.xy = (double*)malloc(sizeof(double*));
        partialInfo.xy_true = (double*)malloc(sizeof(double*));

        int signal_size;
        fread(&signal_size, sizeof(int), 1, file);    
        printf("Signal Size: %d\n", signal_size);

        double x[signal_size], y[signal_size], xy[signal_size];

        // Reading X array of doubles
        fread(&x, sizeof(double [signal_size]), 1, file);    
        // Reading Y array of doubles
        fread(&y, sizeof(double [signal_size]), 1, file);    

        double result = 0;
        // Circular cross
        for (int i=0; i<signal_size; i++){
            for (int k=0; k<signal_size; k++){
                result += x[k] * y[(i+k) % signal_size];
            }
            xy[i] = result;
            //write to file
            fwrite(&result, sizeof(double), 1, file);
            result = 0;
        }

        // Append to file
        for(int j=signal_size-4;j<signal_size;j++) {
            printf("XY[%d] = %f\n", j, xy[j]);
        }

        savePartialResults(id, partialInfo);

    }

    statusWorker[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorker[id]);
}


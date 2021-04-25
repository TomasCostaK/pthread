#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "cross_relation.h"

int *statusWorker;
static void *work (int tid);

int main(int argc, char *argv[])
{   
    char *files[10];    
    int fi = 0;
    int threads = 0;

    // Calculating time
    double t0, t1;
    t0 = ((double) clock ()) / CLOCKS_PER_SEC;

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

    storeFileNames(fi, files);

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

    t1 = ((double) clock ()) / CLOCKS_PER_SEC;
    printf ("\nElapsed time = %.6f s\n", t1 - t0);
    exit (EXIT_SUCCESS);
}


static void *work(int tid){

    int id = tid;
    printf("Thread %d created \n", id);
    int fileId, n, point;
    double * x;
    double * y;
    double val = 0;

    while (processConvPoint(id, &fileId, &n, &x, &y, &point) != 2){
        val = computeValue(n, x, y, point);
        savePartialResults(id, fileId, point, val);
    }

    statusWorker[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorker[id]);
}
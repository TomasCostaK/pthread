#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "shared.h"
#include "helperfuncs.h"

int *statusWorker;

static void *work (int tid);


int main(int argc, char *argv[]){

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
        //printf("BUFFER thread %d: %s \n", id, buff);
        int i = 0;
        char ch;
        //char c[4];
    
        while (i < strlen(buff)){
            /*c[0] = buff[i];
            for (int j = 1; j < numberOfBytesInChar((unsigned char)c); j++) {
                i++;
                c[j] = buff[i];
            }*/
            
            ch = buff[i];
            i++;

            ch = tolower(ch);   //lower case

            switch (ch) //accent removal
            {
                case (char) 'á': ch = 'a'; break;
                case (char) 'à': ch = 'a'; break;
                case (char) 'ã': ch = 'a'; break;
                case (char) 'â': ch = 'a'; break;
                case (char) 'é': ch = 'e'; break;
                case (char) 'è': ch = 'e'; break;
                case (char) 'ê': ch = 'e'; break;
                case (char) 'í': ch = 'i'; break;
                case (char) 'ì': ch = 'i'; break;
                case (char) 'ó': ch = 'o'; break;
                case (char) 'ò': ch = 'o'; break;
                case (char) 'õ': ch = 'o'; break;
                case (char) 'ô': ch = 'o'; break;
                case (char) 'ú': ch = 'u'; break;
                case (char) 'ù': ch = 'u'; break;
                case (char) 'ç': ch = 'c'; break;
                default: break;
            }
            if ((int)ch < 0 || (int)ch > 127)   //remove non ascii
                continue;

            printf("%c",ch);
        }

    }

    statusWorker[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorker[id]);
}

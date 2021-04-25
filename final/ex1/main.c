#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "shared.h"
#include "helperfuncs.h"
#include "main.h"

int *statusWorker;

static void *work (int tid);



int main(int argc, char *argv[]){

    //Process command line
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
    int *status_p;                                  /* pointer to execution status */

    double t0, t1; 

    for (int t = 0; t < threads; t++)
        workers[t] = t;


    storeFileNames(fi, files );

    t0 = ((double) clock ()) / CLOCKS_PER_SEC;

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

    printProcessingResults();
    t1 = ((double) clock ()) / CLOCKS_PER_SEC;
    printf ("\nElapsed time = %.6f s\n", t1 - t0);

}

static void *work(int tid){

    int id = tid;
    char buff[1050];
    printf("Thread %d created \n", id);
    struct PartialInfo partialInfo;

    while (getDataChunk(id, buff, &partialInfo) != 2){
        int i = 0;

        //char c[4];
        char ch;

        //init flags
        int consonants = 0;
        int inword = 0;
        int numchars = 0;

        //make triangular matrix
        partialInfo.nwords = 0;
        partialInfo.data = (int**)malloc(sizeof(int*));
        partialInfo.data[0] = (int*)malloc(sizeof(int));
        partialInfo.data[0][0] = 0;
        partialInfo.rows = 1;

        while (i < strlen(buff)){   //go through the chars in the buffer

            ch = buff[i];
            i++;
            
            //The way we get the characters is not the correct way, we dont account for utf8 coding here
            //we did not have time do fix this even though we know it is wrong.

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
                case (char) 'Á': ch = 'a'; break;
                case (char) 'À': ch = 'a'; break;
                case (char) 'Ã': ch = 'a'; break;
                case (char) 'Â': ch = 'a'; break;
                case (char) 'É': ch = 'e'; break;
                case (char) 'È': ch = 'e'; break;
                case (char) 'Ê': ch = 'e'; break;
                case (char) 'Í': ch = 'i'; break;
                case (char) 'Ì': ch = 'i'; break;
                case (char) 'Ó': ch = 'o'; break;
                case (char) 'Ò': ch = 'o'; break;
                case (char) 'Õ': ch = 'o'; break;
                case (char) 'Ô': ch = 'o'; break;
                case (char) 'Ú': ch = 'u'; break;
                case (char) 'Ù': ch = 'u'; break;
                case (char) 'Ç': ch = 'c'; break;
                default: break;
            }
            if ((int)ch < 0 || (int)ch > 127)   //remove non ascii
                continue;


            if (ch == '\''){    //apostrhope
                continue;
            }

            int isStopChar = stopChars(ch);

            if (isStopChar && inword){ //word already started and end of word
                partialInfo.nwords++;
                partialInfo.data = prepareMatrix(&partialInfo.rows, numchars, partialInfo.data);
                partialInfo.data[numchars][consonants]++;   //add consonant count
                partialInfo.data[numchars][numchars+1]++;   //add word count 

                consonants = 0;
                numchars = 0;
                inword = 0;
            }

            if (!isStopChar){
                inword = 1;
                numchars++;
                if (isConsonant(ch))
                    consonants++;

            }
        }
        if (inword == 1){ //last word
            partialInfo.data = prepareMatrix(&partialInfo.rows, numchars, partialInfo.data);
            partialInfo.data[numchars][consonants]++; 
            partialInfo.data[numchars][numchars+1]++;
            partialInfo.nwords++;
        }

        savePartialResults(id, partialInfo);
        
        free(partialInfo.data);

    }

    statusWorker[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorker[id]);
}

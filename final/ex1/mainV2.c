#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "shared.h"
#include "helperfuncs.h"
#include "main.h"
#include <locale.h>
#include <wchar.h>

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

        //printf("%s\n\n\n", buff);
        processDataString(buff);
        //printf("%s\n\n\n", buff);


        while (i < strlen(buff)){   //go through the chars in the buffer

            ch = buff[i];
            i++;
            
            //The way we get the characters is not the correct way, we dont account for utf8 coding here
            //we did not have time do fix this even though we know it is wrong.

            ch = tolower(ch);   //lower case

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


void processDataString(unsigned char * data)
{

    wint_t c;

    unsigned long data_len = strlen((const char*)data);
    unsigned char temp[1050+1];
    int i=0;
    int j=0;
    while(i<data_len){
        int inci = 1;
        int incj = 1;
        c = data[i];
        /* if the character is lower then 128 ASCII numbers*/
        if(c<=128){
            temp[j] = data[i];
        }else{
            /* In case the of a UTF-8 character && refering to a special character letter*/
            if(c == 0xc3){
                wint_t c2 = data[i+1];
                if((c2 >= 0x80 && c2 <= 0x86) || (c2 >= 0xa0 && c2 <= 0xa6)){
                    temp[j] = 'A';
                }else if((c2 >= 0xa8 && c2 <= 0xab) || (c2 >= 0x88 && c2 <= 0x8b)){
                    temp[j] = 'E';
                }else if((c2 >= 0xac && c2 <= 0xaf) || (c2 >= 0x8c && c2 <= 0x8f)){
                    temp[j] = 'I';
                }else if((c2 >= 0xb2 && c2 <= 0xb6) || (c2 >= 0x92 && c2 <= 0x96)){
                    temp[j] = 'O';
                }else if((c2 >= 0xb9 && c2 <= 0xba) || (c2 >= 0x99 && c2 <= 0x9c)){
                    temp[j] = 'U';
                }else if(c2 == 0xa7 || c2 == 0x87){
                    temp[j] = 'C';
                }else{
                    incj = 0;
                }
                inci = 2;   
            }else if(c == 0xc2){
                temp[j] = 0x20;
                inci = 2;
            }else if(c == 0xe2){
                wint_t c2 = data[i+1];
                wint_t c3 = data[i+2];
                if((c2 == 0x80 && c3 == 0x93) || (c2 == 0x80 && c3 == 0xa6)){
                    temp[j] = 0x20;
                }else if((c2 == 0x80 && c3 == 0x98) || (c2 == 0x80 && c3 == 0x99)){
                    temp[j] = 0x27;
                }else{
                    incj = 0;
                }
                inci = 3;
            }
        }
        temp[j] = temp[j] > 96 ? temp[j]-32 : temp[j];
        i += inci;
        j += incj;
    }
    memcpy(data, temp, j);
    data[j]='\0';
}

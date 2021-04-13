#include <stdio.h>
#include <stdlib.h>

int** allocTriMatrix(int oldrows, int newrows, int ** m){

    m = (int**)realloc(m, newrows * sizeof(int*));
    for(int i = oldrows; i < newrows; i++){
        int ncols = i + 2;  //number of chars + 0 consonant case + number of words
        m[i] = (int*)malloc(ncols * sizeof(int));
        for (int j = 0; j < ncols; j++){
            m[i][j] = 0;
        }
    }
    return m;
}

int** prepareMatrix(int *currentRows, int row, int ** m){
    //printf("%d %d\n", *currentRows, row);
    row++;
    if (row > *currentRows){
        m = allocTriMatrix(*currentRows, row, m);
        *currentRows = row;
    }

    return m;

}

int isInList(char val, char arr[], int len){
    int i;
    for(i = 0; i < len; i++)
    {
        if(arr[i] == val)
            return 1;
    }
    return 0;
}

int stopChars(char val)
{
    char arr[16] = {' ', '\t', '\n', '\0', '.', ',', ':', ';', '?', '!', '-', '"', '(', ')', '[',']'};
    return isInList(val, arr, 16);
}

int isConsonant(char val)
{
    char arr[21] = {'b', 'c', 'd', 'f', 'g','h', 'j', 'k', 'l', 'm','n', 'p', 'q', 'r', 's', 't', 'v', 'w', 'x', 'y', 'z'};
    return isInList(val, arr, 21);
}




int main(int argc, char *argv[])
{   

    for (int i = 1; i < argc; i++)
    {     //for each arg
        
        printf(argv[i]);
        printf("\n");

        FILE * file;
        file = fopen(argv[i], "r");     //open file

        if (file == NULL)
        {
            printf("\nUnable to open file.\n");
            exit(EXIT_FAILURE);
        }

        int words = 0;
        int consonants = 0;
        int inword = 0;
        char ch;
        int numchars = 0;

        //make triangular matrix
        int ** data = (int**)malloc(sizeof(int*));
        data[0] = (int*)malloc(sizeof(int));
        data[0][0] = 0;
        int currentRows = 1;
        
        while ((ch = fgetc(file)) != EOF)
        {   

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

            if (ch == '\''){    //apostrhope
                continue;
            }

            int isStopChar = stopChars(ch);

            if (isStopChar && inword){ //word already started and end of word
                words++;
                data = prepareMatrix(&currentRows, numchars, data);
                data[numchars][consonants]++;   //add consonant count
                data[numchars][numchars+1]++;   //add word count 

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
            data = prepareMatrix(&currentRows, numchars, data);
            data[numchars][consonants]++; 
            words++;
        }

        /* Print file statistics */

        //LINE 1
        printf("  ");
        for (int i=1; i<currentRows; i++){
            printf("%6d", i);
        }
        printf("\n");
        
        //LINE 2
        printf("  ");
        for (int i=1; i<currentRows; i++){
            printf("%6d", data[i][i+1]);
        }
        printf("\n");

        //LINE 3
        printf("  ");
        for (int i=1; i<currentRows; i++){
            printf("%6.2f", 100 * (float)data[i][i+1] / (float)words);
        }
        printf("\n");

        //---------
        for (int col = 0; col<currentRows; col++){
            printf("%2d", col);
            for (int row = 1; row < currentRows; row++){
                if (col > row){
                    printf("      ");
                }
                else
                    printf("%6.1f", 100 * (float)data[row][col] / (float)data[row][row+1]);
            }
            printf("\n");
        }




        /*for(int i = 1; i < currentRows ; i++){
            for (int j = 0; j < (i + 1); j++)
                printf("%d  ", data[j][i]);
            printf("\n");
        }*/
        free(data);

        printf("\n");
        printf("Total words = %d\n", words);
        printf("Consonant Frequency = %d\n", consonants/words);


        /* Close files to release resources */
        fclose(file);

    }
    return 0;
}



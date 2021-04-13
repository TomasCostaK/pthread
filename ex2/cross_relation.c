#include <stdio.h>
#include <stdlib.h>

int** read_signal(FILE * file){
    char buffer[1024];
    size_t num_read = fread(buffer, sizeof(char), 1024, file);
    if (num_read < 1024) printf("Reached EOF!\n"); 

    // We must explicitly NULL terminate the buffer with '\0'
    buffer[num_read] = '\0';

    printf("Read %lu characters\n", num_read);
    printf("Buffer: %s\n", buffer);
    fclose(file);
    return 0;
}

int main(int argc, char *argv[])
{   

    for (int i = 1; i < argc; i++)
    {     //for each arg
        
        printf("%s\n",argv[i]);
        FILE * file;
        file = fopen(argv[i], "rb+");     //open file

        if (file == NULL)
        {
            printf("\nUnable to open file.\n");
            exit(EXIT_FAILURE);
        }

        // read one file, then read the other and send it to the function of processing
        read_signal(file);

    }

    //after reading both files, we need to cross relate the signals
    

    return 0;
}



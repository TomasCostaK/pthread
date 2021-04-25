#include <stdio.h>
#include <stdlib.h>

// auxiliary function to check if writing was done correctly
int** check_file(FILE *file){
    int signal_size;
    fread(&signal_size, sizeof(int), 1, file);    

    printf("\n");
    double x[signal_size], y[signal_size], xy[signal_size], xy_true[signal_size];

    fread(&x, sizeof(double [signal_size]), 1, file);    
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("WRITTEN X[%d] = %f\n", j, x[j]);
    }

    fread(&y, sizeof(double [signal_size]), 1, file);    
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("WRITTEN Y[%d] = %f\n", j, y[j]);
    }

    fread(&xy_true, sizeof(double [signal_size]), 1, file);    
    for(int j=signal_size-4;j<signal_size;j++) {
         printf("WRITTEN XY_TRUE[%d] = %f\n", j, xy[j]);
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

    for (int i = 1; i < argc; i++)
    {
        
        printf("%s\n",argv[i]);
        FILE *file;
        file = fopen(argv[i], "ab+");     //open file

        if (file == NULL)
        {
            printf("\nUnable to open file.\n");
            exit(EXIT_FAILURE);
        }

        // read one file, then read the other and send it to the function of processing
        //process_signal(file);
        //file = fopen(argv[i], "rb"); 
        check_file(file);
    }    
    return 0;
}



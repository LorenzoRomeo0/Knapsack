#include <stdio.h>
#include <stdlib.h>
#include <fminknap_npg.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <fnmatch.h>

//#define SHOWRES
//#define SHOWMAT

#define MAX_INSTANCES 15000
#define MAX_FILES 15000
#define MAX_INSTANCE_FILENAME_SIZE 200

void printa(int a[], size_t size){
    /* print array */
    for(int i = 0; i<size; i++)
        printf("%3d ",a[i]);
    printf("\n");
}
void printa_d(double a[], size_t size){
    /* print array */
    for(int i = 0; i<size; i++)
        printf("%3lf ",a[i]);
    printf("\n");
}


void getFnmatch(char *filenames[MAX_FILES], char *output[MAX_INSTANCES], size_t filenameSize, size_t *outputSize, char* match){
    *outputSize = 0;
    for(int i = 0; i<filenameSize; i++){
        if (!fnmatch(match, filenames[i], FNM_NOESCAPE)){
            //output[*outputSize] = malloc(strlen(filenames[i]) + 1);
            //strcpy(output[*outputSize], filenames[i]);
            output[*outputSize] = filenames[i];
            (*outputSize)++;
        }
    }
}

void readitypeValues(char* filename, itype **profits, itype **weights, int *size){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(filename, "r");
    itype b = 0;
    int a=0, c=0;

    // Read items nr
    int itemsNr = 0;
    read = getline(&line, &len, fp);
    
    sscanf(line, "%d", &itemsNr);
    *size = itemsNr;
    
    *profits = calloc(*size, sizeof(itype));
    *weights = calloc(*size, sizeof(itype));
    
    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1 && itemsNr--) {
        sscanf(line, "%5d %lf %5d", &a, &b, &c);
        (*profits)[a-1] =  b;
        (*weights)[a-1] = (itype) c; 
        //printf("%d) %f ", a, (*profits)[a-1]);
    }

    if (ferror(fp)) {
        printf("File error!\n");
        exit(EXIT_FAILURE);
    }
    free(line);
    fclose(fp);
}

void readValues_d_i(char* filename, double **profits, int **weights, int *size){

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(filename, "r");
    itype b = 0;
    int a=0, c=0;

    // Read items nr
    int itemsNr = 0;
    read = getline(&line, &len, fp);
    
    sscanf(line, "%d", &itemsNr);
    *size = itemsNr;
    
    *profits = calloc(*size, sizeof(itype));
    *weights = calloc(*size, sizeof(int));
    
    if (fp == NULL) {
        printf("Failed to open file.\n");
        free(*profits);
        free(*weights);
        exit(EXIT_FAILURE);
    }

    if (*profits == NULL || *weights == NULL) {
        printf("Failed to allocate memory.\n");
        free(*profits);
        free(*weights);
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1 && itemsNr--) {
        sscanf(line, "%5d %lf %5d", &a, &b, &c);
        (*profits)[a-1] = b;
        (*weights)[a-1] = c; 
    }

    if (ferror(fp)) {
        printf("File error!\n");
        free(*profits);
        free(*weights);
        exit(EXIT_FAILURE);
    }

    free(line);
    fclose(fp);
}

/**
 * Inserts in filenames the names of the files contained in the dir specified in dirname.
 * returns the actual number of files.
*/
int getFilenames(char* dirname, int maxFiles, char *filenames[maxFiles]){
    int fileNr = 0;

    // Open the directory
    DIR *dir = opendir(dirname);
    
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // Read the entries in the directory one by one
    struct dirent *entry;
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        fileNr++;
        
        // Allocate memory for the filename and copy it into the array
        filenames[i] = malloc(strlen(entry->d_name) + 1);
        strcpy(filenames[i], entry->d_name);
        i++;

        // Stop reading if we reach the maximum number of files
        if (i == maxFiles) {
            break;
        }
    }

    // Close the directory
    closedir(dir);

    return fileNr;
}

// KS V1
void ks_d(double *profits, int *weights, int capacity, int n, int* x){
    /*  0-1 knapsack */

    // Trivial solution
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Table creation
    double mat[n+1][capacity+1];

    for(int i = 0; i<n+1;i++)
        for(int j=0;j<capacity+1;j++) mat[i][j] = 0;

    for(int i=0; i < n+1; i++){
        for(int j=0; j<capacity+1; j++){
            int w = (i == 0)? 0 : weights[i-1];
            double p = (i == 0)? 0 : profits[i-1];

            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(j-w<0) {
                mat[i][j] = mat[i-1][j];
            }else if(mat[i-1][j] > mat[i-1][j-w]+p){
                mat[i][j] = mat[i-1][j];
            }else{
                mat[i][j] = mat[i-1][j-w]+p; 
            }
        }
    }

    // Result search
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(mat[i][indexWeight] != mat[i-1][indexWeight]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }


    // puts("Mat original");
    // for(int k=0; k<n+1; k++){
    //     for(int l=0; l<capacity+1; l++){
    //         printf("%2f ", mat[k][l]);
    //     }
    //     puts("");
    // }

    // Result visualization
    #ifdef SHOWMAT
    int maxprofit = 0;
    printf("matrix:\n");

    for(int j = 0; j < capacity+1; j++) printf("%3d ", j);
    printf("\n");
    for(int j = 0; j < capacity+1; j++) printf("%3s","_");
    printf("\n");    

    printm_d(n+1, capacity+1, mat);
    printf("\n");
    #endif

    /*
    #ifdef SHOWRES
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", x[i], i, (i+1==n)?".\n":", ");
            maxprofit += x[i]*profits[i];

        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

//-

// KS2 con ottimizzazione delle colnne
int minCap_opt(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity) {
    // calcolo della dimensione di t
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    //capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    // *t_size = capacity+1;
    *t_size = capacity+1;

    // Costruzione degli array che simulano la tabella di memoizzazione
    int *c1 = (int*) calloc(computedCapacity+1, sizeof(int));
    int *c2 = (int*) calloc(computedCapacity+1, sizeof(int));

    int *prev = c2;
    int *current = c1;
    int *aux;

    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    for(int i=0; i<n+1; i++){
        //for(int j=0; j<capacity+1; j++){
        for(int j=0; j<computedCapacity+1; j++){

            int w = (i == 0)? 0 : weights[i-1];

            if(j == 0){
                current[j] = 0;
            }else if(i == 0){
                current[j] = capacity+1;
            } else if(j-w < 0){ 
                current[j] = prev[j];
            }else if(prev[j] < current[j-w]+w ){
                current[j] = prev[j];
            } else{
                current[j] = prev[j-w]+w; 
            }
        // printf("%3d ", current[j]);
        }
        // puts("");
        aux = current;
        current = prev;
        prev = aux;
    }
//puts("§");
    // Build result arrays
    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] <= capacity) (*s_size)++;


    *t = NULL;
    *s = NULL;
    *t = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));


    // for(int i=0; i<(*s_size); i++) (*s)[i] = 1;

    // *t = malloc((*t_size)* sizeof(int));
    // *s = malloc((*s_size)* sizeof(int));
    // for(int i=0; i< (*t_size); i++) (*t) = 0;
    // for(int i=0; i< (*s_size); i++) (*s) = 0;

    

    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(prev[i] <= capacity){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }

    // rilascio della memoria dedicata agli array
    free(c1);
    free(c2);
    return capacity+1;
}

int minCap_opt_1(int *weights, int n, int **t, int **t1, int *t_size, int **s, int *s_size, int capacity) {
    // calcolo della dimensione di t
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    //capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    // *t_size = capacity+1;
    *t_size = capacity+1;

    // Costruzione degli array che simulano la tabella di memoizzazione
    int *c1 = (int*) calloc(computedCapacity+1, sizeof(int));
    int *c2 = (int*) calloc(computedCapacity+1, sizeof(int));

    int *prev = c2;
    int *current = c1;
    int *aux;

    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    for(int i=0; i<n+1; i++){
        //for(int j=0; j<capacity+1; j++){
        for(int j=0; j<computedCapacity+1; j++){

            int w = (i == 0)? 0 : weights[i-1];

            if(j == 0){
                current[j] = 0;
            }else if(i == 0){
                current[j] = capacity+1;
            } else if(j-w < 0){ 
                current[j] = prev[j];
            }else if(prev[j] < current[j-w]+w ){
                current[j] = prev[j];
            } else{
                current[j] = prev[j-w]+w; 
            }
        // printf("%3d ", current[j]);
        }
        // puts("");
        aux = current;
        current = prev;
        prev = aux;
    }
//puts("§");
    // Build result arrays
    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] <= capacity) (*s_size)++;


    *t = NULL;
    *s = NULL;
    *t = calloc(*t_size, sizeof(int));
    *t1 = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));


    // for(int i=0; i<(*s_size); i++) (*s)[i] = 1;

    // *t = malloc((*t_size)* sizeof(int));
    // *s = malloc((*s_size)* sizeof(int));
    // for(int i=0; i< (*t_size); i++) (*t) = 0;
    // for(int i=0; i< (*s_size); i++) (*s) = 0;

    

    int counter = 0;
    int s_cont = 0;
    int lastValue = 0;
    for(int i=0; i<capacity+1; i++){
        if(prev[i] <= capacity){
            (*t)[i] = counter++;
            lastValue = (*t)[i];
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
        (*t1)[i] = lastValue;
    }

    // rilascio della memoria dedicata agli array
    free(c1);
    free(c2);
    return capacity+1;
}


int minCapv2_(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity<capacity)?capacity:computedCapacity;

    *t_size = capacity+1;

    // Build table
    
    uint *c1 = (uint*) calloc(capacity+1, sizeof(uint));
    uint *c2 = (uint*) calloc(capacity+1, sizeof(uint));

    uint *prev = c2;
    uint *current = c1;
    uint *aux;
    
    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    for(int i=0; i<n+1; i++){
        //int w = (i == 0)? 0 : weights[i-1];
        int w = weights[i];
        //int p = (i == 0)? 0 : 1;
        current[w] = prev[w] + 1;
        //current[w] = prev[w];
        for(int j=0; j<w; j++){
            current[j] = prev[j];
        }
        for(int j=w+1; j<capacity+1; j++){
            current[j] = prev[j] + prev[j-w];
        }
        aux = current;
        current = prev;
        prev = aux;
        
        //printa(current, n+1);
    }

    
    // Build result arrays

    int threshold = 1;

    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] > 0) (*s_size)++;

    
    *t = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));
    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(prev[i] >= threshold){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    
    //free

    free(c1);
    free(c2);
    return capacity+1;
}

int minCapv2__(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity, int threshold) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    //capacity = (computedCapacity<capacity)?capacity:computedCapacity;

    *t_size = capacity+1;

    // Build table
    
    uint *c1 = (uint*) calloc(computedCapacity+1, sizeof(uint));
    uint *c2 = (uint*) calloc(computedCapacity+1, sizeof(uint));

    uint *prev = c2;
    uint *current = c1;
    uint *aux;
    
    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    for(int i=0; i<n+1; i++){
        //int w = (i == 0)? 0 : weights[i-1];
        int w = weights[i];
        //int p = (i == 0)? 0 : 1;
        current[w] = prev[w] + 1;
        //current[w] = prev[w];
        for(int j=0; j<w; j++){
            current[j] = prev[j];
        }
        for(int j=w+1; j<capacity+1; j++){
            current[j] = prev[j] + prev[j-w];
        }
        aux = current;
        current = prev;
        prev = aux;
        
        //printa(current, n+1);
    }

    
    // Build result arrays

    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] > 0) (*s_size)++;




    printf("---- %d %d\n", *s_size, *t_size);

    //QUI
    *t = NULL;
    *s = NULL;
    *t = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));
    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(prev[i] >= threshold){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    
    //free
    free(c1);
    free(c2);
    return capacity+1;
}

int minCapv2___(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity, int threshold) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    //capacity = (computedCapacity<capacity)?capacity:computedCapacity;
    // puts("ok");puts("ok");puts("ok");puts("ok");puts("ok");puts("ok");
    // printf("cap: %d, n:%d, computed:%d\n", capacity, n, computedCapacity);
    
    *t_size = capacity+1;
    *t = calloc(*t_size, sizeof(int));
    //*s = calloc(*t_size, sizeof(int));
    //printa(*t, *t_size);
    // Build table
    
    uint *c1 = (uint*) calloc(computedCapacity+1, sizeof(uint));
    uint *c2 = (uint*) calloc(computedCapacity+1, sizeof(uint));

    uint *prev = c2;
    uint *current = c1;
    uint *aux;
    
    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    printf("n=%d, computed=%d\n", n, computedCapacity);

    for(int i=0; i<n; i++){
        
        int w = (i==0)? 0: weights[i-1];
        current[w] = prev[w] + 1;
        printf("w = %d, i=%d, current[w]=%d\n", w, i, current[w]);
        for(int j=0; j<w; j++){
            printf(" prev[j] = %d\n", prev[j]);
            current[j] = prev[j];
        }
        for(int j=w+1; j<computedCapacity+1; j++){
            printf(" prev[j] = %d, prev[j-w]=%d, j-w=%d\n", prev[j], prev[j-w], j-w);
            current[j] = prev[j] + prev[j-w];
        }

        aux = current;
        current = prev;
        prev = aux;
        
        //printa(current, computedCapacity);
        // for(int k=0; k<computedCapacity; k++) printf("%d ", current[k]);
        // puts("-");
    }
    
    
    // Build result arrays

    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] > 0) (*s_size)++;

    printf("---- s:%d t:%d\n", *s_size, *t_size);

    *s = calloc(*s_size, sizeof(int));

    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(prev[i] >= threshold){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    
    //free
    free(c1);
    free(c2);
    return capacity+1;
}

int minCapv2(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity, int threshold) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    //capacity = (computedCapacity<capacity)?capacity:computedCapacity;
    // puts("ok");puts("ok");puts("ok");puts("ok");puts("ok");puts("ok");
    // printf("cap: %d, n:%d, computed:%d\n", capacity, n, computedCapacity);

    *t_size = capacity+1;
    *t = calloc(*t_size, sizeof(int));
    //*s = calloc(*t_size, sizeof(int));
    //printa(*t, *t_size);
    // Build table

    ulong *c1 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));
    ulong *c2 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));

    ulong *prev = c2;
    ulong *current = c1;
    ulong *aux;
    
    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    // printa(weights, n);

    // printf("n=%d, computed=%d\n", n, computedCapacity);

    for(int i=0; i<n; i++){
       
        int w = (i==0)? 0: weights[i-1];

        // printf("w = %d, prev[w] = %ld, current[w] = %ld\n", w, prev[w], current[w]);
        //printf("w = %d,  current[w] = %ld\n", w, current[w]);
        current[w] = prev[w] + 1;
        // printf("w = %d, i=%d, current[w]=%ld\n", w, i, current[w]);
        for(int j=0; j<w; j++){
            //printf(" prev[j] = %ld\n", prev[j]);
            current[j] = prev[j];
        }
        for(int j=w+1; j<computedCapacity+1; j++){
            //printf(" prev[j] = %ld, prev[j-w]=%ld, j-w=%d\n", prev[j], prev[j-w], j-w);
            current[j] = prev[j] + prev[j-w];
        }
        
        // for(int k=0; k<computedCapacity+1; k++) printf("%3ld ", current[k]);
        // puts("");
        aux = current;
        current = prev;
        prev = aux;
        
        
    }
    
    free(current);
    // Build result arrays
    //puts("helo");
    *s_size = 0;
    //printf("capacity: %d, computedCapacity: %d\n", capacity, computedCapacity);
    int cfr = (computedCapacity < capacity )? computedCapacity+1: capacity+1;
    for(int i=0; i<cfr; i++){
        // printf("------ i = %d (computed = %d), s_s= %d\n", i, computedCapacity, *s_size);
        // printf("------- prev[i] = %ld\n", prev[i]);
        if(prev[i] > 0) (*s_size)++;
    }
    //puts("helo1");

    
    //printf("---- s:%d t:%d\n", *s_size, *t_size);

    *s = calloc(*s_size, sizeof(int));

    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<cfr; i++){
        if(prev[i] >= threshold){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }

    for(int i=cfr; i<capacity+1; i++){
        (*t)[i] = -1;
    }
    *s_size = s_cont;
    
    //free
    //free(c1);
    //free(c2);
    free(prev);
    return capacity+1;
}

int minCapv2_1(int *weights, int n, int **t, int **t1, int *t_size, int **s, int *s_size, int capacity, int threshold) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    //capacity = (computedCapacity<capacity)?capacity:computedCapacity;
    // puts("ok");puts("ok");puts("ok");puts("ok");puts("ok");puts("ok");
    // printf("cap: %d, n:%d, computed:%d\n", capacity, n, computedCapacity);
//puts("ok°°°°°°°°°°°");
    *t_size = capacity+1;
    
    *t = calloc(*t_size, sizeof(int));
    //printf("++%d\n", *t_size);
    *t1 = calloc(*t_size, sizeof(int));
    //*s = calloc(*t_size, sizeof(int));
    //printa(*t, *t_size);
    // Build table

    ulong *c1 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));
    ulong *c2 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));

    ulong *prev = c2;
    ulong *current = c1;
    ulong *aux;
    
    // for(int i = 0; i<n+1;i++) c1[i] = 0;
    // for(int i = 0; i<n+1;i++) c2[i] = 0;

    // printa(weights, n);

    // printf("n=%d, computed=%d\n", n, computedCapacity);

    for(int i=0; i<n; i++){
       
        int w = (i==0)? 0: weights[i-1];

        // printf("w = %d, prev[w] = %ld, current[w] = %ld\n", w, prev[w], current[w]);
        //printf("w = %d,  current[w] = %ld\n", w, current[w]);
        current[w] = prev[w] + 1;
        // printf("w = %d, i=%d, current[w]=%ld\n", w, i, current[w]);
        for(int j=0; j<w; j++){
            //printf(" prev[j] = %ld\n", prev[j]);
            current[j] = prev[j];
        }
        for(int j=w+1; j<computedCapacity+1; j++){
            //printf(" prev[j] = %ld, prev[j-w]=%ld, j-w=%d\n", prev[j], prev[j-w], j-w);
            current[j] = prev[j] + prev[j-w];
        }
        
        // for(int k=0; k<computedCapacity+1; k++) printf("%3ld ", current[k]);
        // puts("");
        aux = current;
        current = prev;
        prev = aux;
        
        
    }
    
    free(current);
    // Build result arrays
    //puts("helo");
    *s_size = 0;
    //printf("capacity: %d, computedCapacity: %d\n", capacity, computedCapacity);
    int cfr = (computedCapacity < capacity )? computedCapacity+1: capacity+1;
    for(int i=0; i<cfr; i++){
        // printf("------ i = %d (computed = %d), s_s= %d\n", i, computedCapacity, *s_size);
        // printf("------- prev[i] = %ld\n", prev[i]);
        if(prev[i] > 0) (*s_size)++;
    }
    //puts("helo1");

    
    //printf("---- s:%d t:%d\n", *s_size, *t_size);

    *s = calloc(*s_size, sizeof(int));

    int counter = 0;
    int s_cont = 0;
    int lastValue = 0;
    for(int i=0; i<capacity+1; i++){
        if(prev[i] >= threshold){
            (*t)[i] = counter++;
            lastValue = (*t)[i];
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
        (*t1)[i] = lastValue;
    }

    for(int i=cfr; i<capacity+1; i++){
        (*t)[i] = -1;
    }
    *s_size = s_cont;
    
    //free
    free(prev);
    return capacity+1;
}


void ks2_d(double *profits, int *weights, int capacity, int n, int* x){
    // trivial solution
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Get column values
    int *t;
    int *s;
    int t_size;
    int s_size;
    minCap_opt(weights, n, &t, &t_size, &s, &s_size, capacity);

    // Table creation
    double mat[n+1][s_size];

    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) mat[i][j] = 0;
    
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0)? 0 : weights[i-1];
            float p = (i == 0)? 0 : profits[i-1];
            int clm = s[j];                                 
            int cfr = (clm - w < 0)? -2 : t[clm - w];
            int prevCol = -1;
            
            if((clm - w) >= 0)
                for(prevCol=(clm-w); t[prevCol] == -1; prevCol--){}

            double oldVal = (t[clm - w] == -1 && i != 0)? mat[i-1][t[prevCol]]: mat[i-1][cfr];
            
            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr == -2){
                mat[i][j] = mat[i-1][j];   
            }else if(t[clm - w]<0 && mat[i-1][j] == 0){
                mat[i][j] = profits[0];
            }else if(mat[i-1][j] > oldVal+p){ 
                 
                mat[i][j] = mat[i-1][j];
            } else{
                mat[i][j] = oldVal+p; 
            }
        }
    }

    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(t[indexWeight]<0){
            indexWeight--;
            i++;
            continue;
        } 
        if(mat[i][t[indexWeight]] != mat[i-1][t[indexWeight]]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization

    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    /*
    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

void ks2_di(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int s_size, int s[s_size],  int* x){
    // trivial solution
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Table creation
    double **mat = (double**) malloc((n+1)*sizeof(double*));
    for(int i=0; i<n+1; i++){
        mat[i] = (double*) malloc(s_size*sizeof(double));
    }

    //

    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) mat[i][j] = 0;
    
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0)? 0 : weights[i-1];
            float p = (i == 0)? 0 : profits[i-1];
            int clm = s[j];                                 
            int cfr = (clm - w < 0)? -2 : t[clm - w];
            int prevCol = -1;
            
            if((clm - w) >= 0)
                for(prevCol=(clm-w); t[prevCol] == -1; prevCol--){}
            
            // printf("i = %d, j = %d\n", i, j);
            // printf("cfr = %d\n", cfr);
            // printf("t[clm-w] = %d - clm = %d, w = %d\n", t[clm-w], clm, w);
            // printf("prevCol = %d\n", prevCol);

            double oldVal = (t[clm - w] == -1 && i > 0)? mat[i-1][t[prevCol]]: (cfr>0 && i>0)? mat[i-1][cfr]:0;
            //il problema avviene dopo questo punto, prova a stampare i valori
            


            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr == -2){
                mat[i][j] = mat[i-1][j];   
            }else if(t[clm - w]<0 && mat[i-1][j] == 0){
                mat[i][j] = profits[0];
            }else if(mat[i-1][j] > oldVal+p){ 
                 
                mat[i][j] = mat[i-1][j];
            } else{
                mat[i][j] = oldVal+p; 
            }
        }
    }

    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(t[indexWeight]<0){
            indexWeight--;
            i++;
            continue;
        } 
        if(mat[i][t[indexWeight]] != mat[i-1][t[indexWeight]]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization

    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    //

    for(int i=0; i<n+1; i++){
        free(mat[i]);
    }
    free(mat);

    /*
    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

void ks2_di_alloc(int n, int s_size, double ***mat){
    (*mat) = (double**) malloc((n+1)*sizeof(double*));
    for(int i=0; i<n+1; i++){
        (*mat)[i] = (double*) malloc(s_size*sizeof(double));
    }

    //
    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) (*mat)[i][j] = 0;
}

void ks2_di_free(int n, double ***mat){
    for(int i=0; i<n+1; i++){
        free((*mat)[i]);
    }
    free(*mat);
}

void ks2_di_prealloc_(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int s_size, int s[s_size], double** mat,  int* x){
    // trivial solution check
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

printf("ks: tsize: %d, s_size: %d, n: %d, capac: %d\n", t_size, s_size, n, capacity);
    // Table creation
    for(int i = 0; i < n+1; i++){
        //printf("i: %d, n:%d\n",i, n);
        for(int j = 0; j < s_size; j++){
            int w = (i == 0) ? 0 : weights[i-1];
            float p = (i == 0) ? 0 : profits[i-1];
            int clm = s[j];                     
            int cfr = (clm - w < 0) ? -2 : t[clm - w];
            int prevCol = -1;

            // printf("j = %d, s[j]=%d, clm = %d, w=%d, clm-w=%d\n", j, s[j],clm, w, clm-w);
            //printf(" j:%d, w=%d, p=%d, clm=%d, cfr=%d oldVal=%lf\n", j, w, p, clm, cfr, 0);
            //printf(" -j:%d, w=%d, p=%d, clm=%d, cfr=%d, clm-w=%d, prevCol=%d\n", j, w, p, clm, cfr, clm - w, prevCol);
            
            if((clm - w) >= 0){
                for(prevCol=(clm-w); t[prevCol] == -1 && prevCol>=0; prevCol--){}
            }

            // printf("clm:%d ", clm);
            // printf("j:%d ",j);
            // printf("s[j]:%d ",s[j]);
            // printf("clm:%d ",clm);
            // printf("w:%d ",w);
            // printf("clm-w:%d ",clm-w);
            // puts("");
            //printf(" -j:%d, w=%d, p=%d, clm=%d, cfr=%d, clm-w=%d, prevCol=%d\n", j, w, p, clm, cfr, clm - w, prevCol);
            double oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t[prevCol]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            // puts("5");
            
            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr == -2){
                mat[i][j] = mat[i-1][j];   
            }else if(mat[i-1][j] > oldVal+p){ 
                mat[i][j] = mat[i-1][j];
            } else{
                mat[i][j] = oldVal+p; 
            }
        }
    }
//puts("ok");puts("ok");puts("ok");puts("ok");
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(t[indexWeight]<0){
            indexWeight--;
            i++;
            continue;
        } 
        if(mat[i][t[indexWeight]] != mat[i-1][t[indexWeight]]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization

    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    //

    // for(int i=0; i<n+1; i++){
    //     free(mat[i]);
    // }
    // free(mat);

    /*
    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

void ks2_di_prealloc(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int s_size, int s[s_size], double** mat,  int* x){
    //printf(" s-size %d:\n", s_size);
    // printa(s, s_size);
    // puts("++");
    
    // trivial solution check
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // printf("ks: tsize: %d, s_size: %d, n: %d, capac: %d\n", t_size, s_size, n, capacity);
    // Table creation
    for(int i = 0; i < n+1; i++){
        //printf("i: %d, n:%d\n",i, n);
        for(int j = 0; j < s_size; j++){
            int w = (i == 0) ? 0 : weights[i-1];
            float p = (i == 0) ? 0 : profits[i-1];
            int clm = s[j];                     
            int cfr = (clm - w < 0) ? -2 : t[clm - w];
            int prevCol = -1;

            //printf("j = %d, s[j]=%d, clm = %d, w=%d, clm-w=%d\n", j, s[j],clm, w, clm-w);
            
            //printf(" j:%d, w=%d, p=%d, clm=%d, cfr=%d oldVal=%lf\n", j, w, p, clm, cfr, 0);
            //printf(" -j:%d, w=%d, p=%d, clm=%d, cfr=%d, clm-w=%d, prevCol=%d\n", j, w, p, clm, cfr, clm - w, prevCol);
            
            double oldVal = 0;

            if((clm - w) >= 0){
                for(prevCol=(clm-w); t[prevCol] == -1 && prevCol>=0; prevCol--){}
                oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t[prevCol]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            }

            // printf("clm:%d ", clm);
            // printf("j:%d ",j);
            // printf("s[j]:%d ",s[j]);
            // printf("clm:%d ",clm);
            // printf("w:%d ",w);
            // printf("clm-w:%d ",clm-w);
            // puts("");

            //printf(" -j:%d, w=%d, p=%d, clm=%d, cfr=%d, clm-w=%d, prevCol=%d\n", j, w, p, clm, cfr, clm - w, prevCol);
            //double oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t[prevCol]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            // puts("5");
            
            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr == -2){
                mat[i][j] = mat[i-1][j];   
            }else if(mat[i-1][j] > oldVal+p){ 
                mat[i][j] = mat[i-1][j];
            }else{
                mat[i][j] = oldVal+p; 
            }
        }
    }
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(t[indexWeight]<0){
            indexWeight--;
            i++;
            continue;
        } 
        if(mat[i][t[indexWeight]] != mat[i-1][t[indexWeight]]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization

    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    //

    // for(int i=0; i<n+1; i++){
    //     free(mat[i]);
    // }
    // free(mat);

    /*
    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

void ks2_di_prealloc_1(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int t1[t_size], int s_size, int s[s_size], double** mat,  int* x){
    //printf(" s-size %d:\n", s_size);
    // printa(s, s_size);
    // puts("++");
    
    // trivial solution check
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }
    // printf("ks: tsize: %d, s_size: %d, n: %d, capac: %d\n", t_size, s_size, n, capacity);
    // Table creation
    for(int i = 0; i < n+1; i++){
        //printf("i: %d, n:%d\n",i, n);
        for(int j = 0; j < s_size; j++){
            int w = (i == 0) ? 0 : weights[i-1];
            float p = (i == 0) ? 0 : profits[i-1];
            int clm = s[j];                     
            int cfr = (clm - w < 0) ? -2 : t[clm - w];

            //printf("j = %d, s[j]=%d, clm = %d, w=%d, clm-w=%d\n", j, s[j],clm, w, clm-w);
            
            //printf(" j:%d, w=%d, p=%d, clm=%d, cfr=%d oldVal=%lf\n", j, w, p, clm, cfr, 0);
            //printf(" -j:%d, w=%d, p=%d, clm=%d, cfr=%d, clm-w=%d, prevCol=%d\n", j, w, p, clm, cfr, clm - w, prevCol);
            double oldVal = 0;
            if((clm - w) >= 0){
                oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t1[clm - w]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            }
            // printf("clm:%d ", clm);
            // printf("j:%d ",j);
            // printf("s[j]:%d ",s[j]);
            // printf("clm:%d ",clm);
            // printf("w:%d ",w);
            // printf("clm-w:%d ",clm-w);
            // puts("");

            //printf(" -j:%d, w=%d, p=%d, clm=%d, cfr=%d, clm-w=%d, prevCol=%d\n", j, w, p, clm, cfr, clm - w, prevCol);
            //double oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t[prevCol]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            // puts("5");
            
            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr == -2){
                mat[i][j] = mat[i-1][j];   
            }else if(mat[i-1][j] > oldVal+p){ 
                mat[i][j] = mat[i-1][j];
            }else{
                mat[i][j] = oldVal+p; 
            }
        }
    }
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(t[indexWeight]<0){
            indexWeight--;
            i++;
            continue;
        } 
        if(mat[i][t[indexWeight]] != mat[i-1][t[indexWeight]]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization

    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    //

    // for(int i=0; i<n+1; i++){
    //     free(mat[i]);
    // }
    // free(mat);

    /*
    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

// --

/* ======================================================================
				  Main
   ====================================================================== */

int main(int argc, char *argv[]) {

    if(argc < 3) {
        printf("Please specify the pathname and the capacity of the knapsack:\nbinaryKnapsack2a generator/files/derived/int 100");
        exit(EXIT_FAILURE);
    }
    int capacity = atoi(argv[2]);       //knapsack capacity
    char* filesPath = argv[1];          //instances path

    /* testmodes:
        0: full (esecuzione di tutti gli algoritmi risolutivi)
        1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
    */

    int testmode = (argc > 3)? atoi(argv[3]):0;
    int cut_threshold = ((testmode == 3 || testmode == 4) && argc > 4)? atoi(argv[4]):1;
    int division_value = ((testmode == 3 || testmode == 4) && argc > 4)? atoi(argv[5]):1;

    // printf("test: %d\n", testmode);
    // printf("cut: %d\n", cut_threshold);
    // printf("div: %d\n", division_value);

    // instance types initialization ---
    char **filenames = (char**) malloc(MAX_FILES * sizeof(char*));
    int fileNr = getFilenames(filesPath, MAX_FILES, filenames);
    
    char **uncorr = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t uncorrSize;
    getFnmatch(filenames, uncorr, fileNr, &uncorrSize, "uncorr*");

    char **alm = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t almSize;
    getFnmatch(filenames, alm, fileNr, &almSize, "alm*");

    char **inv = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t invSize;
    getFnmatch(filenames, inv, fileNr, &invSize, "inv*");

    char **str = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t strSize;
    getFnmatch(filenames, str, fileNr, &strSize, "str*");

    char **weak = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t weakSize;
    getFnmatch(filenames, weak, fileNr, &weakSize, "weak*");
    
    char** allTypes[] = {uncorr, alm, inv, str, weak};
    char* allTypesNames[] = {"uncorr", "almost", "inv", "str", "weak"};
    size_t allTypesSizes[] = {uncorrSize, almSize, invSize, strSize, weakSize};
    int typeCount = 0;
    //---

    // clock initialization ---

    clock_t start, end;

    // execution ---

    if(testmode == 0){          // 0: full (esecuzione di tutti gli algoritmi risolutivi)
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n");
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            //printf("ok");
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // columOpt_V1_nocol_noalloc    -------------------------------------------------------------------------------------------
                int *x2_v1_nocol_noalloc = calloc(n, sizeof(int));

                start = clock();
                //ks2_d(profits, weights, capacity, n, x2_v1_nocol_noalloc);                          //ks
                end = clock();
                double time_ks2_v1_nocol_noalloc = ((double) (end - start)) / CLOCKS_PER_SEC;

            // original                     -------------------------------------------------------------------------------------------
                int *x_original = calloc(n, sizeof(int));

                start = clock();
                ks_d(profits, weights, capacity, n, x_original);                                   //ks
                end = clock();
                double time_original = ((double) (end - start)) / CLOCKS_PER_SEC;


            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            

                //      1  2  3  4  5  6  7   8             1                 2              3         4            5              6          7       8
                printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], time_original, time_ks2,              time_ks2_v1,              time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap, resOpt_v1, resOpt, resFmin);
                //printf("                          typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
            
            // FREE                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x2_v1_nocol_noalloc);
                free(x_original);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }

    }else if(testmode == 1){    // 1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            
                
                printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0, time_ks2, time_ks2_v1, 0.0, time_ks2_v1_allocation, 0.0, time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap, resOpt_v1, resOpt, resFmin);

                //      1  2  3  4  5  6  7   8             1                 2              3                      4                         5              6          7       8
                //printf("%s,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,              time_ks2_v1,              time_fminknap, resOpt_v1, resOpt, resFmin);
                                        //printf("typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap, resOpt_v1, resOpt, resFmin\n");
            // FREE                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }
    
    }else if(testmode == 2){    // 2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        printf("type,capacity,n,t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;

            start = clock();
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            end = clock();
            double mincap_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            
            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;

            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            end = clock();
            double mincapv2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////---

            printf("%s,                      %d,      %d,%d,        %d,     %d,        %d,     %f,%f\n", allTypesNames[typeCount],capacity,n, t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
            //      allTypesNames[typeCount],capacity,n, t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
        }

    }else if(testmode == 3){    // 3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_1,time_columOpt_prealloc_V1,time_columOpt_prealloc_V1_1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            int value = 0;
            for(int i=0; i<n; i++){
                value = ceil(weights[i]/division_value);
                dividedWeights[i] = (value > 0)? value: 1;
            }
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, 1);

            // v1 1 --
            int *t_v1_1 = NULL;
            int *t1_v1_1 = NULL;
            int *s_v1_1 = NULL;
            int t_size_v1_1 = 0;
            int s_size_v1_1 = 0;
            minCap_opt_1(dividedWeights, n, &t_v1_1, &t1_v1_1, &t_size_v1_1, &s_v1_1, &s_size_v1_1, capacity);  

            // v2 1  --
            int *t_1 = NULL;
            int *t1_1 = NULL;
            int *s_1 = NULL;
            int t_size_1 = 0;
            int s_size_1 = 0;
            minCapv2_1(weights, n, &t_1, &t1_1, &t_size_1, &s_1, &s_size_1, capacity, cut_threshold);

            //printf("s_div: %d, s_cut: %d\n", s_size_v1, s_size);
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                int *fakeWeights = NULL;
                readValues_d_i(instanceFilename, &profits, &fakeWeights, &n);
            
            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc 1            ------------------------------------------------------------------------------------------
                int *x2_1 = calloc(n, sizeof(int));
                double **mat_ks2_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_1, &mat_ks2_1);                                                  // alloc
                end = clock();
                double time_ks2_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_1, t_1, t1_1, s_size_1, s_1, mat_ks2_1, x2_1);  // ks
                end = clock();
                double time_ks2_1 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt_1 = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt_1 += x2_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_1);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, dividedWeights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            //columOpt_prealloc_V1 1        -------------------------------------------------------------------------------------------
                int *x2_v1_1 = calloc(n, sizeof(int));
                double **mat_ks2_v1_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1_1, &mat_ks2_v1_1);                                            //alloc
                end = clock();
                double time_ks2_v1_1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, dividedWeights, capacity, n, t_size_v1_1, t_v1_1, t1_v1_1, s_size_v1_1, s_v1_1, mat_ks2_v1_1, x2_v1_1); //ks
                end = clock();
                double time_ks2_v1_1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1_1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1_1 += x2_v1_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1_1);                                                        //free
            
            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            
                                                                     //printf("typename,                time_original,  time_columOpt_prealloc,   time_columOpt_prealloc_1, time_columOpt_prealloc_V1,time_columOpt_prealloc_V1_1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation, time_ks2_allocation, time_ks2_v1_allocation, time_ks2_allocation, time_ks2_v1_allocation, time_ks2_allocation, time_fminknap, resOpt_v1, resOpt, resFmin\n"); 
                
                printf("v1:%f, v1_1:%f, v2:%f, v2_1:%f; rv1=%f, rv1_1=%f, rv2=%f, rv2_1=%f, rfmin=%f\n", time_ks2_v1, time_ks2_v1_1, time_ks2, time_ks2_1, resOpt_v1, resOpt_v1_1, resOpt, resOpt_1, resFmin);
                //printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,                 time_ks2_1,               time_ks2_v1,              time_ks2_v1_1,              0.0,                      time_ks2_v1_allocation, time_ks2_allocation, time_ks2_v1_allocation, time_ks2_allocation, time_ks2_v1_allocation, time_ks2_allocation, time_fminknap, resOpt_v1, resOpt, resFmin);
                //      1  2  3  4  5  6  7   8             1                 2              3                      4                         5              6          7       8
                //printf("%s,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,              time_ks2_v1,              time_fminknap, resOpt_v1, resOpt, resFmin);
                                        //printf("typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap, resOpt_v1, resOpt, resFmin\n");
            // FREE                         -------------------------------------------------------------------------------------------
                
                free(x2);
                free(x2_v1);
                free(x2_1);
                free(x2_v1_1);
                free(x);
                free(profits);
                
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
                free(fakeWeights);
            }
            free(weights);
        }
    
    }else if(testmode == 4){    // 4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
        printf("type,capacity,n,t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            for(int i=0; i<n; i++) dividedWeights[i] = ceil(weights[i]/division_value);

            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            start = clock();
            minCap_opt(dividedWeights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            end = clock();
            double mincap_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;

            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, cut_threshold);
            end = clock();
            double mincapv2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////----------------------------------------------------------------------------------------------

            printf("%s,                      %d,      %d,%d,        %d,     %d,        %d,     %f,%f\n", allTypesNames[typeCount],capacity,n, t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
        }
    
    }
    
    return 0;
}

//gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ./generator/files/derived/int/ 50000

int main___(int argc, char *argv[]) {

    if(argc < 3) {
        printf("Please specify the pathname and the capacity of the knapsack:\nbinaryKnapsack2a generator/files/derived/int 100");
        exit(EXIT_FAILURE);
    }
    int capacity = atoi(argv[2]);       //knapsack capacity
    char* filesPath = argv[1];          //instances path

    /* testmodes:
        0: full (esecuzione di tutti gli algoritmi risolutivi)
        1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
    */
    int testmode = 0;
    int cut_threshold = 1;
    int division_value = 1;
    
    // instance types initialization ---
    char **filenames = (char**) malloc(MAX_FILES * sizeof(char*));
    int fileNr = getFilenames(filesPath, MAX_FILES, filenames);
    
    char **uncorr = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t uncorrSize;
    getFnmatch(filenames, uncorr, fileNr, &uncorrSize, "uncorr*");

    char **alm = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t almSize;
    getFnmatch(filenames, alm, fileNr, &almSize, "alm*");

    char **inv = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t invSize;
    getFnmatch(filenames, inv, fileNr, &invSize, "inv*");

    char **str = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t strSize;
    getFnmatch(filenames, str, fileNr, &strSize, "str*");

    char **weak = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t weakSize;
    getFnmatch(filenames, weak, fileNr, &weakSize, "weak*");
    
    char** allTypes[] = {uncorr, alm, inv, str, weak};
    char* allTypesNames[] = {"uncorr", "almost", "inv", "str", "weak"};
    size_t allTypesSizes[] = {uncorrSize, almSize, invSize, strSize, weakSize};
    int typeCount = 0;
    //---

    // clock initialization ---

    clock_t start, end;

    // execution ---

    if(testmode == 0){          // 0: full (esecuzione di tutti gli algoritmi risolutivi)
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n");
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            //printf("ok");

            puts("+++");
            puts(" V1:");
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            puts(" V2:");
            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            
            puts("weights");
            printa(weights, n);
            puts("t:");
            printa(t_v1, t_size_v1);
            puts("s:");
            printa(s_v1, s_size_v1);
            puts("t_v2:");
            printa(t, t_size);
            puts("s_v2:");
            printa(s, s_size);
            puts("...");

            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

                puts("profits\n");
                printa_d(profits, n);

            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
                puts("\nMat ks2 v2");
                for(int k=0; k<n+1; k++){
                    for(int l=0; l<s_size; l++){
                        printf("%2f ", mat_ks2[k][l]);
                    }
                    puts("");
                }

                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                puts("Sol ks2 v2:");
                printa(x2, n);

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                puts("\nMat ks2 v1");
                for(int k=0; k<n+1; k++){
                    for(int l=0; l<s_size_v1; l++){
                        printf("%2f ", mat_ks2_v1[k][l]);
                    }
                    puts("");
                }

                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                puts("Sol ks2 v1:");
                printa(x2_v1, n);

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // columOpt_V1_nocol_noalloc    -------------------------------------------------------------------------------------------
                int *x2_v1_nocol_noalloc = calloc(n, sizeof(int));

                start = clock();
                //ks2_d(profits, weights, capacity, n, x2_v1_nocol_noalloc);                          //ks
                end = clock();
                double time_ks2_v1_nocol_noalloc = ((double) (end - start)) / CLOCKS_PER_SEC;

            // original                     -------------------------------------------------------------------------------------------
                int *x_original = calloc(n, sizeof(int));

                start = clock();
                ks_d(profits, weights, capacity, n, x_original);                                   //ks
                end = clock();
                double time_original = ((double) (end - start)) / CLOCKS_PER_SEC;


                puts("Sol original:");
                printa(x_original, n);

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            

                //      1  2  3  4  5  6  7   8             1                 2              3         4            5              6          7       8
                //printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], time_original, time_ks2,              time_ks2_v1,              time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap, resOpt_v1, resOpt, resFmin);
                //printf("                          typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
            
            // FREE                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x2_v1_nocol_noalloc);
                free(x_original);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }

    }else if(testmode == 1){    // 1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            
                
                printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0, time_ks2, time_ks2_v1, 0.0, time_ks2_v1_allocation, 0.0, time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap, resOpt_v1, resOpt, resFmin);

                //      1  2  3  4  5  6  7   8             1                 2              3                      4                         5              6          7       8
                //printf("%s,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,              time_ks2_v1,              time_fminknap, resOpt_v1, resOpt, resFmin);
                                        //printf("typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap, resOpt_v1, resOpt, resFmin\n");
            // FREE                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }
    
    }else if(testmode == 2){    // 2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        printf("t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;

            start = clock();
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            end = clock();
            double mincap_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            
            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;

            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            end = clock();
            double mincapv2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////---

            printf("%d,%d,%d,%d,%f,%f\n", t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
        }

    }else if(testmode == 3){    // 3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            for(int i=0; i<n; i++) dividedWeights[i] = ceil(weights[i]/division_value);

            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(dividedWeights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, cut_threshold);
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                int *fakeWeights = NULL;
                readValues_d_i(instanceFilename, &profits, &fakeWeights, &n);
                free(fakeWeights);
            
            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, dividedWeights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            

                //      1  2  3  4  5  6  7   8             1                 2              3                      4                         5              6          7       8
                printf("%s,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,              time_ks2_v1,              time_fminknap, resOpt_v1, resOpt, resFmin);
                                        //printf("typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap, resOpt_v1, resOpt, resFmin\n");
            // FREE                         -------------------------------------------------------------------------------------------
                
                free(x2);
                free(x2_v1);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }
    
    }else if(testmode==4){
        printf("t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            for(int i=0; i<n; i++) dividedWeights[i] = ceil(weights[i]/division_value);

            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            start = clock();
            minCap_opt(dividedWeights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            end = clock();
            double mincap_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;

            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, cut_threshold);
            end = clock();
            double mincapv2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////----------------------------------------------------------------------------------------------

            printf("%d,%d,%d,%d,%f,%f\n", t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
        }
    
    }
    
    return 0;
}

int main__(int argc, char *argv[]) {

    if(argc < 3) {
        printf("Please specify the pathname and the capacity of the knapsack:\nbinaryKnapsack2a generator/files/derived/int 100");
        exit(EXIT_FAILURE);
    }
    int capacity = atoi(argv[2]);       //knapsack capacity
    char* filesPath = argv[1];          //instances path

    /* testmodes:
        0: full (esecuzione di tutti gli algoritmi risolutivi)
        1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
    */
    int testmode = 0;
    int cut_threshold = 2;
    int division_value = 2;
    
    // instance types initialization ---
    char **filenames = (char**) malloc(MAX_FILES * sizeof(char*));
    int fileNr = getFilenames(filesPath, MAX_FILES, filenames);
    
    char **uncorr = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t uncorrSize;
    getFnmatch(filenames, uncorr, fileNr, &uncorrSize, "uncorr*");

    char **alm = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t almSize;
    getFnmatch(filenames, alm, fileNr, &almSize, "alm*");

    char **inv = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t invSize;
    getFnmatch(filenames, inv, fileNr, &invSize, "inv*");

    char **str = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t strSize;
    getFnmatch(filenames, str, fileNr, &strSize, "str*");

    char **weak = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t weakSize;
    getFnmatch(filenames, weak, fileNr, &weakSize, "weak*");
    
    char** allTypes[] = {uncorr, alm, inv, str, weak};
    char* allTypesNames[] = {"uncorr", "almost", "inv", "str", "weak"};
    size_t allTypesSizes[] = {uncorrSize, almSize, invSize, strSize, weakSize};
    int typeCount = 0;
    //---

    // clock initialization ---

    clock_t start, end;

    // execution ---

    if(testmode == 0){          // 0: full (esecuzione di tutti gli algoritmi risolutivi)
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n");
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            puts("Weights: ");
            printa(weights, n);
            puts("Divided weights: ");
            int *dividedWeights = calloc(n, sizeof(int));
            for(int i=0; i<n; i++) dividedWeights[i] = ceil(weights[i]/division_value);
            printa(dividedWeights, n);
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            //printf("ok");

            puts("+++");
            puts(" V1:");
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(dividedWeights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            puts(" V2:");
            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            
            // puts("weights");
            // printa(weights, n);
            puts("t:");
            printa(t_v1, t_size_v1);
            puts("s:");
            printa(s_v1, s_size_v1);
            puts("t_v2:");
            printa(t, t_size);
            puts("s_v2:");
            printa(s, s_size);
            puts("...");

            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

                puts("profits\n");
                printa_d(profits, n);

            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
                puts("\nMat ks2 v2");
                for(int k=0; k<n+1; k++){
                    for(int l=0; l<s_size; l++){
                        printf("%2f ", mat_ks2[k][l]);
                    }
                    puts("");
                }

                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                puts("Sol ks2 v2:");
                printa(x2, n);

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                puts("\nMat ks2 v1");
                for(int k=0; k<n+1; k++){
                    for(int l=0; l<s_size_v1; l++){
                        printf("%2f ", mat_ks2_v1[k][l]);
                    }
                    puts("");
                }

                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                puts("Sol ks2 v1:");
                printa(x2_v1, n);

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // columOpt_V1_nocol_noalloc    -------------------------------------------------------------------------------------------
                int *x2_v1_nocol_noalloc = calloc(n, sizeof(int));

                start = clock();
                //ks2_d(profits, weights, capacity, n, x2_v1_nocol_noalloc);                          //ks
                end = clock();
                double time_ks2_v1_nocol_noalloc = ((double) (end - start)) / CLOCKS_PER_SEC;

            // original                     -------------------------------------------------------------------------------------------
                int *x_original = calloc(n, sizeof(int));

                start = clock();
                ks_d(profits, weights, capacity, n, x_original);                                   //ks
                end = clock();
                double time_original = ((double) (end - start)) / CLOCKS_PER_SEC;


                puts("Sol original:");
                printa(x_original, n);

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            

                //      1  2  3  4  5  6  7   8             1                 2              3         4            5              6          7       8
                //printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], time_original, time_ks2,              time_ks2_v1,              time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap, resOpt_v1, resOpt, resFmin);
                //printf("                          typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
            
            // FREE                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x2_v1_nocol_noalloc);
                free(x_original);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }

    }else if(testmode == 1){    // 1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_ks2_v1_nocol_noalloc,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        //printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            
                
                printf("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0, time_ks2, time_ks2_v1, 0.0, time_ks2_v1_allocation, 0.0, time_ks2_v1_allocation,time_ks2_allocation,time_ks2_v1_allocation,time_ks2_allocation,time_fminknap, resOpt_v1, resOpt, resFmin);

                //      1  2  3  4  5  6  7   8             1                 2              3                      4                         5              6          7       8
                //printf("%s,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,              time_ks2_v1,              time_fminknap, resOpt_v1, resOpt, resFmin);
                                        //printf("typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap, resOpt_v1, resOpt, resFmin\n");
            // FREE                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }
    
    }else if(testmode == 2){    // 2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        printf("t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;

            start = clock();
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            end = clock();
            double mincap_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            
            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            int threshold = 1;

            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, threshold);
            end = clock();
            double mincapv2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////---

            printf("%d,%d,%d,%d,%f,%f\n", t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
        }

    }else if(testmode == 3){    // 3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        
        printf("typename,time_original,time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap,resOpt_v1,resOpt,resFmin\n"); 
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            for(int i=0; i<n; i++) dividedWeights[i] = ceil(weights[i]/division_value);

            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(dividedWeights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, cut_threshold);
            ////---                             -------------------------------------------------------------------------------------------

            // printf("told:\nt: %d; s: %d\ntnew:\nt: %d; s:%d\n", t_size_v1, s_size_v1, t_size, s_size);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);
                
            // BINARYKNAPSACK ----
                int *fakeWeights = NULL;
                readValues_d_i(instanceFilename, &profits, &fakeWeights, &n);
                free(fakeWeights);
            
            //columOpt_prealloc             -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  // alloc
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  // ks
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;
            
                double resOpt = 0;                                                                  //res1
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           // free

            //columOpt_prealloc_V1          -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                            //alloc
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, dividedWeights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); //ks
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;
                
                double resOpt_v1 = 0;                                                               //resv1 
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);                                                        //free

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi 
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;           
                //int z = 0;                      //optimal objective value

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                openPisinger(n);                                                                    //alloc

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                           //ks
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 //resFmin
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];
            //  -----------------------------------------------------------------------------------------------------------------------            

                //      1  2  3  4  5  6  7   8             1                 2              3                      4                         5              6          7       8
                printf("%s,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount], 0.0,           time_ks2,              time_ks2_v1,              time_fminknap, resOpt_v1, resOpt, resFmin);
                                        //printf("typename,                 time_original, time_columOpt_prealloc,time_columOpt_prealloc_V1,time_fminknap, resOpt_v1, resOpt, resFmin\n");
            // FREE                         -------------------------------------------------------------------------------------------
                
                free(x2);
                free(x2_v1);
                free(x);
                free(profits);
                free(weights);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
            }
        }
    
    }else if(testmode==4){
        printf("t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            
            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            for(int i=0; i<n; i++) dividedWeights[i] = ceil(weights[i]/division_value);

            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            start = clock();
            minCap_opt(dividedWeights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            end = clock();
            double mincap_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;

            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, cut_threshold);
            end = clock();
            double mincapv2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////----------------------------------------------------------------------------------------------

            printf("%d,%d,%d,%d,%f,%f\n", t_size_v1, t_size, s_size_v1, s_size, mincap_time, mincapv2_time);
        }
    
    }
    
    return 0;
}

int main1(int argc, char *argv[]) {

    if(argc < 3) {
        printf("Please specify the pathname and the capacity of the knapsack:\nbinaryKnapsack2a generator/files/derived/int 100");
        exit(EXIT_FAILURE);
    }
    int capacity = atoi(argv[2]);       //knapsack capacity
    char* filesPath = argv[1];          //instances path

    /* testmodes:
        0: full (esecuzione di tutti gli algoritmi risolutivi)
        1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
        2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
        4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
    */

    
    // instance types initialization ---
    char **filenames = (char**) malloc(MAX_FILES * sizeof(char*));
    int fileNr = getFilenames(filesPath, MAX_FILES, filenames);
    
    char **uncorr = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t uncorrSize;
    getFnmatch(filenames, uncorr, fileNr, &uncorrSize, "uncorr*");

    char **alm = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t almSize;
    getFnmatch(filenames, alm, fileNr, &almSize, "alm*");

    char **inv = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t invSize;
    getFnmatch(filenames, inv, fileNr, &invSize, "inv*");

    char **str = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t strSize;
    getFnmatch(filenames, str, fileNr, &strSize, "str*");

    char **weak = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t weakSize;
    getFnmatch(filenames, weak, fileNr, &weakSize, "weak*");
    
    char** allTypes[] = {uncorr, alm, inv, str, weak};
    char* allTypesNames[] = {"uncorr", "almost", "inv", "str", "weak"};
    size_t allTypesSizes[] = {uncorrSize, almSize, invSize, strSize, weakSize};
    int typeCount = 0;
    //---

    // clock initialization ---

    clock_t start, end;

    // execution ---

        //printf("t_v1,t_v2,s_v1,s_v2,time_v1,time_v2\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;               
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);
            
            ///--- Calcolo colonne ottimizzate  -------------------------------------------------------------------------------------------
            int testmode = 0;
            int cut_threshold = 2;
            int division_value = 100;

            // divisione pesi per micapv1
            int *dividedWeights = calloc(n, sizeof(int));
            int value = 0;
            for(int i=0; i<n; i++){
                value = ceil(weights[i]/division_value);
                dividedWeights[i] = (value > 0)? value: 1;
            }

            puts("weights:");
            printa(weights, n);
            puts("divided weights:");
            printa(dividedWeights,n);

            // v1   --
            int *t_v1 = NULL;
            int *s_v1 = NULL;
            int t_size_v1 = 0;
            int s_size_v1 = 0;
            minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);        
            
            puts("v1:");
            printf("s_size: %d\n", s_size_v1);
            printa(s_v1, s_size_v1);

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, 1);

            puts("v2:");
            printf("s_size: %d\n", s_size);
            printa(s, s_size);

            // v1 1 --
            int *t_v1_1 = NULL;
            int *t1_v1_1 = NULL;
            int *s_v1_1 = NULL;
            int t_size_v1_1 = 0;
            int s_size_v1_1 = 0;
            minCap_opt_1(dividedWeights, n, &t_v1_1, &t1_v1_1, &t_size_v1_1, &s_v1_1, &s_size_v1_1, capacity);  


            puts("v1 div:");
            printf("s_size: %d\n", s_size_v1);
            printa(s_v1_1, s_size_v1_1);

            //printa(weights, n);
            // printf("%d\n", cut_threshold);
            // printf("%d\n", capacity);
            // v2 1  --
            int *t_1 = NULL;
            int *t1_1 = NULL;
            int *s_1 = NULL;
            int t_size_1 = 0;
            int s_size_1 = 0;
            minCapv2_1(weights, n, &t_1, &t1_1, &t_size_1, &s_1, &s_size_1, capacity, cut_threshold);

            puts("v2 cut:");
            printf("s_size: %d\n", s_size_1);
            printa(s_1, s_size_1);

            puts("...");
            //printf("s_div: %d, s_cut: %d\n", s_size_v1, s_size);

            //printf("%d,%d,%d,%d\n", t_size_v1, t_size, s_size_v1, s_size);
            free(t_v1);
            free(s_v1);
            free(s);
            free(t);
            free(s_v1_1);
            free(t_v1_1);
            free(t1_v1_1);
            free(t_1);
            free(t1_1);
            free(s_1);
            
        }

    
    return 0;
}

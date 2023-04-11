#include <stdio.h>
#include <stdlib.h>
#include <fminknap_npg.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <fnmatch.h>

#define MAX_INSTANCES 15000
#define MAX_FILES 15000
#define MAX_INSTANCE_FILENAME_SIZE 200

//#define SHOWRES
//#define SHOWMAT

/* print array */
void printa(int a[], size_t size){
    for(int i = 0; i<size; i++)
        printf("%3d ",a[i]);
    printf("\n");
}

/* print double array */
void printa_d(double a[], size_t size){
    for(int i = 0; i<size; i++)
        printf("%3lf ",a[i]);
    printf("\n");
}

/* Funzioni per la lettura dei file delle istanze                               */

void getFnmatch(char *filenames[MAX_FILES], char *output[MAX_INSTANCES], size_t filenameSize, size_t *outputSize, char* match){
    *outputSize = 0;
    for(int i = 0; i<filenameSize; i++){
        if (!fnmatch(match, filenames[i], FNM_NOESCAPE)){
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

    // legge il numero di righe
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

    // legge il numero di righe
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

int getFilenames(char* dirname, int maxFiles, char *filenames[maxFiles]){
    int fileNr = 0;

    DIR *dir = opendir(dirname);

    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // Legge le entry della directory una per una
    struct dirent *entry;
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        fileNr++;

        filenames[i] = malloc(strlen(entry->d_name) + 1);
        strcpy(filenames[i], entry->d_name);
        i++;

        if (i == maxFiles) {
            break;
        }
    }

    closedir(dir);

    return fileNr;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Funzioni per la risoluzione del problema del knapsack binario non ottimizzate*/
void ks_d(double *profits, int *weights, int capacity, int n, int* x){
    // Soluzione triviale
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    double **mat = (double**) malloc((n+1)*sizeof(double*));
    for(int i=0; i<n+1; i++){
        mat[i] = (double*) malloc((capacity+1)*sizeof(double));
    }

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

    // Ricerca del risultato
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(mat[i][indexWeight] != mat[i-1][indexWeight]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;
        }
    }

    // Visualizzazione del risultato
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

    
    #ifdef SHOWRES
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", x[i], i, (i+1==n)?".\n":", ");
            maxprofit += x[i]*profits[i];

        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    
    // free
    for(int i=0; i<n+1; i++){
        free((mat)[i]);
    }
    free(mat);
}

void ks_d_alloc(int n, int capacity, double ***mat){
    (*mat) = (double**) malloc((n+1)*sizeof(double*));
    for(int i=0; i<n+1; i++){
        (*mat)[i] = (double*) malloc((capacity+1)*sizeof(double));
    }

    for(int i = 0; i<n+1; i++)
        for(int j=0; j<(capacity+1); j++) (*mat)[i][j] = 0;
}

void ks_d_free(int n, double ***mat){
    for(int i=0; i<n+1; i++){
        free((*mat)[i]);
    }
    free(*mat);
}

void ks_d_prealloc(double *profits, int *weights, int capacity, int n, double **mat, int* x){
    // Soluzione triviale
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Creazione della tablella
    for(int i = 0; i<n+1;i++)
        for(int j=0;j<capacity+1;j++) mat[i][j] = 0;

    // Riempimento della tabella
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

    // Ricerca del risultato
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(mat[i][indexWeight] != mat[i-1][indexWeight]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;
        }
    }

    // Visualizzazione del risultato
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
    
    #ifdef SHOWRES
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", x[i], i, (i+1==n)?".\n":", ");
            maxprofit += x[i]*profits[i];

        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Funzioni per la risoluzione del problema del knapsack binario ottimizzate    */
int minCap_opt(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity) {
    // calcolo della dimensione di t
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i];
    *t_size = capacity+1;

    // Costruzione degli array che simulano la tabella di memoizzazione
    int *c1 = (int*) calloc(computedCapacity+1, sizeof(int));
    int *c2 = (int*) calloc(computedCapacity+1, sizeof(int));

    int *prev = c2;
    int *current = c1;
    int *aux;

    // Riempimento della tabella
    for(int i=0; i<n+1; i++){
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
        }
        aux = current;
        current = prev;
        prev = aux;
    }

    // Costruzione di s e t
    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] <= capacity) (*s_size)++;

    *t = NULL;
    *s = NULL;
    *t = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));

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

    // Rilascio della memoria dedicata agli array
    free(c1);
    free(c2);
    return capacity+1;
}

int minCap_opt_1(int *weights, int n, int **t, int **t1, int *t_size, int **s, int *s_size, int capacity) {
    // calcolo della dimensione di t
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i];
    *t_size = capacity+1;

    // Costruzione degli array che simulano la tabella di memoizzazione
    int *c1 = (int*) calloc(computedCapacity+1, sizeof(int));
    int *c2 = (int*) calloc(computedCapacity+1, sizeof(int));

    int *prev = c2;
    int *current = c1;
    int *aux;

    //Riempimento della tabella
    for(int i=0; i<n+1; i++){
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
        }
        aux = current;
        current = prev;
        prev = aux;
    }
    
    // Costruzione di s e t
    *s_size = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] <= capacity) (*s_size)++;

    *t = NULL;
    *s = NULL;
    *t = calloc(*t_size, sizeof(int));
    *t1 = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));

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

    // Rilascio della memoria dedicata agli array
    free(c1);
    free(c2);
    return capacity+1;
}

int minCap_opt_1_divided(int *weights, int **dividedWeights, int n, int **t, int **t1, int *t_size, int **s, int *s_size, int capacity, int *dividedCapacity, int division_value) {
    // Divisione dei valori di peso e della capacità
    (*dividedCapacity) = ceil(capacity/division_value);
    if(*dividedCapacity<1) (*dividedCapacity) = 1;
    (*dividedWeights) = calloc(n+1, sizeof(int));
    int currentWeight = 0;
    for(int i=0; i<n; i++){
        currentWeight = ceil(weights[i]/division_value);
        (*dividedWeights)[i] = (currentWeight>0)?currentWeight:1;
    }
    
    // calcolo della dimensione di t
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity += (*dividedWeights)[i];

    *t_size = (*dividedCapacity)+1;

    // Costruzione degli array che simulano la tabella di memoizzazione
    int *c1 = (int*) calloc(computedCapacity+1, sizeof(int));
    int *c2 = (int*) calloc(computedCapacity+1, sizeof(int));

    int *prev = c2;
    int *current = c1;
    int *aux;

    // Rimepimento della tabella
    for(int i=0; i<n+1; i++){
        for(int j=0; j<computedCapacity+1; j++){

            int w = (i == 0)? 0 :  (*dividedWeights)[i-1];

            if(j == 0){
                current[j] = 0;
            }else if(i == 0){
                current[j] = (*dividedCapacity)+1;
            } else if(j-w < 0){
                current[j] = prev[j];
            }else if(prev[j] < current[j-w]+w ){
                current[j] = prev[j];
            } else{
                current[j] = prev[j-w]+w;
            }
        }
        aux = current;
        current = prev;
        prev = aux;
    }

    // Costruzione di s e t
    *s_size = 0;

    int cfr = (computedCapacity < (*dividedCapacity) )? computedCapacity+1: (*dividedCapacity)+1;
    for(int i=0; i<cfr; i++)
        if(prev[i] <= (*dividedCapacity)) (*s_size)++;

    *t = NULL;
    *s = NULL;
    *t = calloc(*t_size, sizeof(int));
    *t1 = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));

    int counter = 0;
    int s_cont = 0;
    int lastValue = 0;
    for(int i=0; i<(*dividedCapacity)+1; i++){
        if(prev[i] <= (*dividedCapacity)){
            (*t)[i] = counter++;
            lastValue = (*t)[i];
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
        (*t1)[i] = lastValue;
    }

    for(int i=cfr; i<(*dividedCapacity)+1; i++){
        (*t)[i] = -1;
    }

    // Rilascio della memoria dedicata agli array
    free(prev);
    free(current);
    return (*dividedCapacity)+1;
}

int minCapv2(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity, int threshold) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i];

    *t_size = capacity+1;
    *t = calloc(*t_size, sizeof(int));

    ulong *c1 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));
    ulong *c2 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));

    ulong *prev = c2;
    ulong *current = c1;
    ulong *aux;

    for(int i=0; i<n; i++){

        int w = (i==0)? 0: weights[i-1];
        current[w] = prev[w] + 1;

        for(int j=0; j<w; j++) 
            current[j] = prev[j];

        for(int j=w+1; j<computedCapacity+1; j++) 
            current[j] = prev[j] + prev[j-w];

        aux = current;
        current = prev;
        prev = aux;
    }

    free(current);

    // Costruzione di s e t
    *s_size = 0;
    int cfr = (computedCapacity < capacity )? computedCapacity+1: capacity+1;
    for(int i=0; i<cfr; i++){
        if(prev[i] > 0) (*s_size)++;
    }

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

    //free
    free(prev);
    return capacity+1;
}

int minCapv2_1(int *weights, int n, int **t, int **t1, int *t_size, int **s, int *s_size, int capacity, int threshold) {
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i];
    *t_size = capacity+1;

    *t = calloc(*t_size, sizeof(int));
    *t1 = calloc(*t_size, sizeof(int));

    ulong *c1 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));
    ulong *c2 = (ulong*) calloc(computedCapacity+1, sizeof(ulong));

    ulong *prev = c2;
    ulong *current = c1;
    ulong *aux;

    for(int i=0; i<n; i++){

        int w = (i==0)? 0: weights[i-1];
        current[w] = prev[w] + 1;

        for(int j=0; j<w; j++)
            current[j] = prev[j];
        
        for(int j=w+1; j<computedCapacity+1; j++)
            current[j] = prev[j] + prev[j-w];
        
        aux = current;
        current = prev;
        prev = aux;
    }

    free(current);
    *s_size = 0;
    int cfr = (computedCapacity < capacity )? computedCapacity+1: capacity+1;
    for(int i=0; i<cfr; i++){
        if(prev[i] > 0) (*s_size)++;
    }

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

    // Ottenimento di s e t
    int *t;
    int *s;
    int t_size;
    int s_size;
    minCap_opt(weights, n, &t, &t_size, &s, &s_size, capacity);


    // Creazione della tabella
    double **mat = (double**) calloc((n+1),sizeof(double*));
    for(int i=0; i<n+1; i++){
        mat[i] = (double*) calloc((s_size),sizeof(double));
    }
    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) mat[i][j] = 0;

    // Riempimento della tabella
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0)? 0 : weights[i-1];
            float p = (i == 0)? 0 : profits[i-1];
            int clm = s[j];
            int cfr = (clm - w < 0)? -2 : t[clm - w];
            int prevCol = -1;

            if((clm - w) >= 0)
                for(prevCol=(clm-w); t[prevCol] == -1; prevCol--){}

            double oldVal = (t[clm - w] == -1 && i > 0)? mat[i-1][t[prevCol]]: (cfr>0 && i>0)? mat[i-1][cfr]:0;

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

    // Visualizzazione dei risultati
    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif

    //free
    for(int i=0; i<n+1; i++){
        free(mat[i]);
    }
    free(mat);
    
}

void ks2_di(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int s_size, int s[s_size],  int* x){
    // Soluzione triviale
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Creazione della tabella
    double **mat = (double**) malloc((n+1)*sizeof(double*));
    for(int i=0; i<n+1; i++){
        mat[i] = (double*) malloc(s_size*sizeof(double));
    }
    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) mat[i][j] = 0;

    // Riempimento della tabella
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0)? 0 : weights[i-1];
            float p = (i == 0)? 0 : profits[i-1];
            int clm = s[j];
            int cfr = (clm - w < 0)? -2 : t[clm - w];
            int prevCol = -1;

            if((clm - w) >= 0)
                for(prevCol=(clm-w); t[prevCol] == -1; prevCol--){}

            double oldVal = (t[clm - w] == -1 && i > 0)? mat[i-1][t[prevCol]]: (cfr>0 && i>0)? mat[i-1][cfr]:0;

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

    // Ricerca del risultato
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

    // Visualizzazione del risultato
    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif

   //free
    for(int i=0; i<n+1; i++){
        free(mat[i]);
    }
    free(mat);
}

void ks2_di_alloc(int n, int s_size, double ***mat){
    (*mat) = (double**) malloc((n+1)*sizeof(double*));
    for(int i=0; i<n+1; i++){
        (*mat)[i] = (double*) malloc(s_size*sizeof(double));
    }

    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) (*mat)[i][j] = 0;
}

void ks2_di_free(int n, double ***mat){
    for(int i=0; i<n+1; i++){
        free((*mat)[i]);
    }
    free(*mat);
}

void ks2_di_prealloc(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int s_size, int s[s_size], double** mat,  int* x){
    // Soluzione triviale
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Riempimento della tabella
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0) ? 0 : weights[i-1];
            float p = (i == 0) ? 0 : profits[i-1];
            int clm = s[j];
            int cfr = (clm - w < 0) ? -2 : t[clm - w];
            int prevCol = -1;

            double oldVal = 0;

            if((clm - w) >= 0){
                for(prevCol=(clm-w); t[prevCol] == -1 && prevCol>=0; prevCol--){}
                oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t[prevCol]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            }

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

    // Ricerca del risultato
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

    // Visualizzazione del risultato
    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
}

void ks2_di_prealloc_1(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int t1[t_size], int s_size, int s[s_size], double** mat,  int* x){
    // Soluzione triviale
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Riempimento della tabella
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0) ? 0 : weights[i-1];
            float p = (i == 0) ? 0 : profits[i-1];
            int clm = s[j];
            int cfr = (clm - w < 0) ? -2 : t[clm - w];

            double oldVal = 0;
            if((clm - w) >= 0){
                oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t1[clm - w]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;
            }

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

    // Visualizzazione del risultato
    #ifdef SHOWMAT
        printf("matrix:\n");

        for(int j = 0; j < s_size; j++) printf("%10lf ", (float)s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        printm_d(n+1, s_size, mat);
        printf("\n");
    #endif

    #ifdef SHOWRES
    int maxprofit = 0;
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Main                                                                          */
int main(int argc, char *argv[]) {
/* testmodes:
        0: full (esecuzione di tutti gli algoritmi risolutivi)
        2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
        3: original (confronto tempi dell'algoritmo originale)
        4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
*/

    if(argc < 3) {
        puts("Specificare il pathname delle istanze, la capacità del knapsack e la modalita' di test:\nbinaryKnapsack2a generator/files/derived/int 100 0");
        puts("Le modalita' di test sono le seguenti: ");
        puts("\t0: full (esecuzione di tutti gli algoritmi risolutivi) ");
        puts("\t2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)");
        puts("\t3: original (confronto tempi dell'algoritmo originale)");
        puts("4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)");
        exit(EXIT_FAILURE);
    }

    int capacity = atoi(argv[2]);       //capacità del knapsack
    char* filesPath = argv[1];          //path dei file delle istanze

    int testmode = (argc > 3)? atoi(argv[3]):0;
    int cut_threshold = (argc > 3)? atoi(argv[4]):1;
    int division_value = (argc > 4)? atoi(argv[5]):1;

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

    // inizializzazione del clock ---

    clock_t start, end;

    // esecuzione ---
    if(testmode == 0){          // 0: full (esecuzione di tutti gli algoritmi risolutivi)
        printf("type,capacity,n,mincap_v1_1_div_time,t_size_v1,s_size_v1,mincap_v1_time,t_size_v2,s_size_v2,mincap_v2_time,");
        printf("t_size_v1_1,s_size_v1_1,mincap_v1_1_time,t_size_v2_1,s_size_v2_1,mincap_v2_1_time,t_size_v1_1_div,s_size_v1_1_div,");
        printf("mincap_v1_1_div_time,t_size_v2_1_cut,s_size_v2_1_cut,mincap_v2_1_cut_time,time_original,time_original_allocation,");
        printf("time_original_prealloc,res_original_alloc,time_ks2_v2_allocation,time_ks2_v2,resOpt_v2,time_ks2_v1_allocation,");
        printf("time_ks2_v1,resOpt_v1,time_ks2_v1_allocation_1,time_ks2_v1_1,resOpt_v1_1,time_ks2_v2_allocation_1,time_ks2_v2_1,");
        printf("resOpt_v2_1,time_ks2_v1_allocation_1_div,time_ks2_v1_1_div,resOpt_v1_1_div,time_ks2_allocation_1_cut,time_ks2_v2_1_cut,");
        printf("resOpt_v2_1_cut,time_ks2_v1_nocol_noalloc,time_fminknap_allocation,time_fminknap,resFmin\n");
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
            double mincap_v1_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2   --
            int *t = NULL;
            int *s = NULL;
            int t_size = 0;
            int s_size = 0;
            start = clock();
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, 1);
            end = clock();
            double mincap_v2_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v1 t'  --
            int *t_v1_1 = NULL;
            int *t1_v1_1 = NULL;
            int *s_v1_1 = NULL;
            int t_size_v1_1 = 0;
            int s_size_v1_1 = 0;
            start = clock();
            minCap_opt_1(weights, n, &t_v1_1, &t1_v1_1,  &t_size_v1_1, &s_v1_1, &s_size_v1_1, capacity);
            end = clock();
            double mincap_v1_1_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2 t'  --
            int *t_v2_1 = NULL;
            int *t1_v2_1 = NULL;
            int *s_v2_1 = NULL;
            int t_size_v2_1 = 0;
            int s_size_v2_1 = 0;

            start = clock();
            minCapv2_1(weights, n, &t_v2_1, &t1_v2_1, &t_size_v2_1, &s_v2_1, &s_size_v2_1, capacity, 1);
            end = clock();
            double mincap_v2_1_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v1 div  --
            int *dividedWeights = NULL;
            int *t_v1_1_div = NULL;
            int *t1_v1_1_div = NULL;
            int *s_v1_1_div = NULL;
            int t_size_v1_1_div = 0;
            int s_size_v1_1_div = 0;
            int dividedCapacity = 0;
            start = clock();
            minCap_opt_1_divided(weights, &dividedWeights, n, &t_v1_1_div, &t1_v1_1_div,  &t_size_v1_1_div, &s_v1_1_div, &s_size_v1_1_div, capacity, &dividedCapacity, division_value);
            end = clock();
            double mincap_v1_1_div_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2 cut  --
            int *t_v2_1_cut = NULL;
            int *t1_v2_1_cut = NULL;
            int *s_v2_1_cut = NULL;
            int t_size_v2_1_cut = 0;
            int s_size_v2_1_cut = 0;

            start = clock();
            minCapv2_1(weights, n, &t_v2_1_cut, &t1_v2_1_cut, &t_size_v2_1_cut, &s_v2_1_cut, &s_size_v2_1_cut, capacity, cut_threshold);
            end = clock();
            double mincap_v2_1_cut_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////----------------------------------------------------------------------------------------------

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);

            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            // original                         -------------------------------------------------------------------------------------------
                int *x_original = calloc(n, sizeof(int));

                start = clock();
                ks_d(profits, weights, capacity, n, x_original);                                   
                end = clock();
                double time_original = ((double) (end - start)) / CLOCKS_PER_SEC;


            // original alloc                    -------------------------------------------------------------------------------------------
                int *x_original_alloc = calloc(n, sizeof(int));

                double **mat_original = NULL;
                start = clock();
                ks_d_alloc(n, capacity, &mat_original);
                end = clock();
                double time_original_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks_d_prealloc(profits, weights, capacity, n, mat_original, x_original_alloc);                                  
                end = clock();
                double time_original_prealloc = ((double) (end - start)) / CLOCKS_PER_SEC;

                double res_original_alloc = 0;                                                               
                for(int k=0; k<n; k++)
                    res_original_alloc += x_original_alloc[k]*profits[k];

                ks_d_free(n, &mat_original);                               

            //columOpt_prealloc  (micapv2)          -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);
                                                                         
            //columOpt_prealloc_V1    (micapv1)      -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                           
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); 
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1 = 0;                                                               
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);

            //columOpt_prealloc_V1    (micapv1 + t')      -------------------------------------------------------------------------------------------
                int *x2_v1_1 = calloc(n, sizeof(int));
                double **mat_ks2_v1_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1_1, &mat_ks2_v1_1);                                           
                end = clock();
                double time_ks2_v1_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v1_1, t_v1_1, t1_v1_1, s_size_v1_1, s_v1_1, mat_ks2_v1_1, x2_v1_1); 
                end = clock();
                double time_ks2_v1_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1_1 = 0;                                                               
                for(int k=0; k<n; k++)
                    resOpt_v1_1 += x2_v1_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1_1);                                                      

            //columOpt_prealloc_v2  (micapv2 + t')       -------------------------------------------------------------------------------------------
                int *x2_1 = calloc(n, sizeof(int));
                double **mat_ks2_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v2_1, &mat_ks2_1);                                                  
                end = clock();
                double time_ks2_v2_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v2_1, t_v2_1, t1_v2_1, s_size_v2_1, s_v2_1, mat_ks2_1, x2_1);  
                end = clock();
                double time_ks2_v2_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v2_1 = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt_v2_1 += x2_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_1);                                                           

            //columOpt_prealloc_V1    (micapv1 + t' + div)      -------------------------------------------------------------------------------------------
                int *x2_v1_1_div = calloc(n, sizeof(int));
                double **mat_ks2_v1_1_div = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1_1_div, &mat_ks2_v1_1_div);                                            
                end = clock();
                double time_ks2_v1_allocation_1_div = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, dividedWeights, dividedCapacity, n, t_size_v1_1_div, t_v1_1_div, t1_v1_1_div, s_size_v1_1_div, s_v1_1_div, mat_ks2_v1_1_div, x2_v1_1_div);
                end = clock();
                double time_ks2_v1_1_div = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1_1_div = 0;                                                               
                for(int k=0; k<n; k++)
                    resOpt_v1_1_div += x2_v1_1_div[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1_1_div);                                                      

            //columOpt_prealloc_v2  (micapv2 + t'+ cut)       -------------------------------------------------------------------------------------------
                int *x2_1_cut = calloc(n, sizeof(int));
                double **mat_ks2_1_cut = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v2_1_cut, &mat_ks2_1_cut);                                                  
                end = clock();
                double time_ks2_allocation_1_cut = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v2_1_cut, t_v2_1_cut, t1_v2_1_cut, s_size_v2_1_cut, s_v2_1_cut, mat_ks2_1_cut, x2_1_cut);  
                end = clock();
                double time_ks2_v2_1_cut = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v2_1_cut = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt_v2_1_cut += x2_1_cut[k]*profits[k];

                ks2_di_free(n, &mat_ks2_1_cut);


            // columOpt_V1_nocol_noalloc    -------------------------------------------------------------------------------------------
                int *x2_v1_nocol_noalloc = calloc(n, sizeof(int));

                start = clock();
                ks2_d(profits, weights, capacity, n, x2_v1_nocol_noalloc);                         
                end = clock();
                double time_ks2_v1_nocol_noalloc = ((double) (end - start)) / CLOCKS_PER_SEC;

            // FMINKNAP                     -------------------------------------------------------------------------------------------
                itype *p_fminknap = NULL;              //profitti
                itype *w_fminknap = NULL;              //pesi
                int *x = calloc(n, sizeof(int));       //solution vector
                SolutionList *s_list = NULL;

                readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);

                start = clock();
                openPisinger(n);                                                                   
                end = clock();
                double time_fminknap_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);                         
                end = clock();
                double time_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resFmin = 0;                                                                 
                for(int k=0; k<n; k++)
                    resFmin += x[k]*profits[k];

            //  -----------------------------------------------------------------------------------------------------------------------
                printf("%s,%d,%d,%f,%d,%d,%f,%d,%d,%f,%d,%d,%f,%d,%d,%f,%d,%d,%f,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",allTypesNames[typeCount],capacity,n,mincap_v1_1_div_time,t_size_v1,s_size_v1,mincap_v1_time,t_size,   s_size,   mincap_v2_time,t_size_v1_1,s_size_v1_1,mincap_v1_1_time,t_size_v2_1,s_size_v2_1,mincap_v2_1_time,t_size_v1_1_div,s_size_v1_1_div,mincap_v1_1_div_time,t_size_v2_1_cut,s_size_v2_1_cut,mincap_v2_1_cut_time,time_original,time_original_allocation,time_original_prealloc,res_original_alloc,time_ks2_allocation,   time_ks2   ,resOpt,   time_ks2_v1_allocation,time_ks2_v1,resOpt_v1,time_ks2_v1_allocation_1,time_ks2_v1_1,resOpt_v1_1,time_ks2_v2_allocation_1,time_ks2_v2_1,resOpt_v2_1,time_ks2_v1_allocation_1_div,time_ks2_v1_1_div,resOpt_v1_1_div,time_ks2_allocation_1_cut,time_ks2_v2_1_cut,resOpt_v2_1_cut,time_ks2_v1_nocol_noalloc,time_fminknap_allocation,time_fminknap,resFmin);

            // FREE                         -------------------------------------------------------------------------------------------
                free(x_original);
                free(x_original_alloc);
                free(x2_v1_1);
                free(x2);
                free(x2_v1);

                free(x2_1);
                free(x2_v1_1_div);
                free(x2_1_cut);
                free(x2_v1_nocol_noalloc);

                free(x);
                free(p_fminknap);
                free(w_fminknap);
                free(s_list);
                free(profits);
                free(weights);
            }

            free(t_v1);
            free(s_v1);
            free(t);
            free(s);
            free(t_v1_1);
            free(t1_v1_1);
            free(s_v1_1);
            free(t_v2_1);
            free(t1_v2_1);
            free(s_v2_1);
            free(t_v1_1_div);
            free(t1_v1_1_div);
            free(s_v1_1_div);
            free(t_v2_1_cut);
            free(t1_v2_1_cut);
            free(s_v2_1_cut);
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
        }
    }else if(testmode == 3){    // 3: original (confronto tempi dell'algoritmo originale)
        printf("typename,time_original,time_original_prealloc\n");
        for(;typeCount<5; typeCount++){
            char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
            strcat(instanceFilename0, filesPath);
            strcat(instanceFilename0, allTypes[typeCount][0]);

            // lettura di un'istanza
            double *profits;
            int *weights;
            int n = 0;
            readValues_d_i(instanceFilename0, &profits, &weights, &n);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);

            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            // original                     -------------------------------------------------------------------------------------------
                int *x_original = calloc(n, sizeof(int));

                start = clock();
                ks_d(profits, weights, capacity, n, x_original);                                   
                end = clock();
                double time_original = ((double) (end - start)) / CLOCKS_PER_SEC;

            // original alloc                    -------------------------------------------------------------------------------------------
                int *x_original_alloc = calloc(n, sizeof(int));

                double **mat_original = NULL;
                ks_d_alloc(n, capacity, &mat_original);

                start = clock();
                ks_d_prealloc(profits, weights, capacity, n, mat_original, x_original_alloc);                                   
                end = clock();
                double time_original_prealloc = ((double) (end - start)) / CLOCKS_PER_SEC;

                ks_d_free(n, &mat_original);

            //  -----------------------------------------------------------------------------------------------------------------------

                printf("%s,%f,%f\n", allTypesNames[typeCount], time_original, time_original_prealloc);

            // FREE                         -------------------------------------------------------------------------------------------
                free(x_original);
                free(profits);
                free(weights);
            }
        }
    }if(testmode == 4){         // 4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)
        printf("type,capacity,n,t_v1,t_v2,s_v1,s_v2,t_size_v1_div,t_size_v2_cut,s_sizev1_div,s_size_v2_cut,time_v1,time_v2,time_v1_div,time_v2_cut,time_original_prealloc,time_ks2_v1_1,time_ks2_v2_1,time_ks2_v1_1_div,time_ks2_v2_1_cut,res_original_alloc,resOpt_v1_1,resOpt_v2_1,resOpt_v1_1_div,resOpt_v2_1_cut\n");
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
            int *t_v1_1 = NULL;
            int *t1_v1_1 = NULL;
            int *s_v1_1 = NULL;
            int t_size_v1_1 = 0;
            int s_size_v1_1 = 0;
            start = clock();
            minCap_opt_1(weights, n, &t_v1_1, &t1_v1_1,  &t_size_v1_1, &s_v1_1, &s_size_v1_1, capacity);
            end = clock();
            double mincap_v1_1_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2   --
            int *t_v2_1 = NULL;
            int *t1_v2_1 = NULL;
            int *s_v2_1 = NULL;
            int t_size_v2_1 = 0;
            int s_size_v2_1 = 0;

            start = clock();
            minCapv2_1(weights, n, &t_v2_1, &t1_v2_1, &t_size_v2_1, &s_v2_1, &s_size_v2_1, capacity, 1);
            end = clock();
            double mincap_v2_1_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v1 div  --
            int *dividedWeights = NULL;
            int *t_v1_1_div = NULL;
            int *t1_v1_1_div = NULL;
            int *s_v1_1_div = NULL;
            int t_size_v1_1_div = 0;
            int s_size_v1_1_div = 0;
            int dividedCapacity = 0;
            start = clock();
            minCap_opt_1_divided(weights, &dividedWeights, n, &t_v1_1_div, &t1_v1_1_div,  &t_size_v1_1_div, &s_v1_1_div, &s_size_v1_1_div, capacity, &dividedCapacity, division_value);
            end = clock();
            double mincap_v1_1_div_time = ((double) (end - start)) / CLOCKS_PER_SEC;

            // v2 cut  --
            int *t_v2_1_cut = NULL;
            int *t1_v2_1_cut = NULL;
            int *s_v2_1_cut = NULL;
            int t_size_v2_1_cut = 0;
            int s_size_v2_1_cut = 0;

            start = clock();
            minCapv2_1(weights, n, &t_v2_1_cut, &t1_v2_1_cut, &t_size_v2_1_cut, &s_v2_1_cut, &s_size_v2_1_cut, capacity, cut_threshold);
            end = clock();
            double mincap_v2_1_cut_time = ((double) (end - start)) / CLOCKS_PER_SEC;
            ////----------------------------------------------------------------------------------------------

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);

            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            // original alloc                    -------------------------------------------------------------------------------------------
                int *x_original_alloc = calloc(n, sizeof(int));

                double **mat_original = NULL;
                ks_d_alloc(n, capacity, &mat_original);

                start = clock();
                ks_d_prealloc(profits, weights, capacity, n, mat_original, x_original_alloc);                
                end = clock();
                double time_original_prealloc = ((double) (end - start)) / CLOCKS_PER_SEC;

                double res_original_alloc = 0;                                                               
                for(int k=0; k<n; k++)
                    res_original_alloc += x_original_alloc[k]*profits[k];

                ks_d_free(n, &mat_original);

            //columOpt_prealloc_V1    (micapv1 + t')      -------------------------------------------------------------------------------------------
                int *x2_v1_1 = calloc(n, sizeof(int));
                double **mat_ks2_v1_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1_1, &mat_ks2_v1_1);                                           
                end = clock();
                double time_ks2_v1_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v1_1, t_v1_1, t1_v1_1, s_size_v1_1, s_v1_1, mat_ks2_v1_1, x2_v1_1); 
                end = clock();
                double time_ks2_v1_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1_1 = 0;                                                               
                for(int k=0; k<n; k++)
                    resOpt_v1_1 += x2_v1_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1_1);                                                      

            //columOpt_prealloc  (micapv2 + t')       -------------------------------------------------------------------------------------------
                int *x2_1 = calloc(n, sizeof(int));
                double **mat_ks2_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v2_1, &mat_ks2_1);                                                  
                end = clock();
                double time_ks2_v2_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v2_1, t_v2_1, t1_v2_1, s_size_v2_1, s_v2_1, mat_ks2_1, x2_1);  
                end = clock();
                double time_ks2_v2_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v2_1 = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt_v2_1 += x2_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_1);                                                           

            //columOpt_prealloc_V1    (micapv1 + t' + div)      -------------------------------------------------------------------------------------------
                int *x2_v1_1_div = calloc(n, sizeof(int));
                double **mat_ks2_v1_1_div = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1_1_div, &mat_ks2_v1_1_div);                                            
                end = clock();
                double time_ks2_v1_allocation_1_div = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, dividedWeights, dividedCapacity, n, t_size_v1_1_div, t_v1_1_div, t1_v1_1_div, s_size_v1_1_div, s_v1_1_div, mat_ks2_v1_1_div, x2_v1_1_div); //ks
                end = clock();
                double time_ks2_v1_1_div = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1_1_div = 0;                                                               
                for(int k=0; k<n; k++)
                    resOpt_v1_1_div += x2_v1_1_div[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1_1_div);                                                      

            //columOpt_prealloc  (micapv2 + t'+ cut)       -------------------------------------------------------------------------------------------
                int *x2_1_cut = calloc(n, sizeof(int));
                double **mat_ks2_1_cut = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v2_1_cut, &mat_ks2_1_cut);                                                  
                end = clock();
                double time_ks2_allocation_1_cut = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v2_1_cut, t_v2_1_cut, t1_v2_1_cut, s_size_v2_1_cut, s_v2_1_cut, mat_ks2_1_cut, x2_1_cut);  // ks
                end = clock();
                double time_ks2_v2_1_cut = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v2_1_cut = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt_v2_1_cut += x2_1_cut[k]*profits[k];

                ks2_di_free(n, &mat_ks2_1_cut);


               printf("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", allTypesNames[typeCount],capacity,n, t_size_v1_1, t_size_v2_1, s_size_v1_1, s_size_v2_1, t_size_v1_1_div, t_size_v2_1_cut,   s_size_v1_1_div, s_size_v2_1_cut, mincap_v1_1_time, mincap_v2_1_time,  mincap_v1_1_div_time, mincap_v2_1_cut_time,time_original_prealloc, time_ks2_v1_1, time_ks2_v2_1, time_ks2_v1_1_div, time_ks2_v2_1_cut, res_original_alloc,resOpt_v1_1,resOpt_v2_1,resOpt_v1_1_div,resOpt_v2_1_cut);

                // free
                free(x_original_alloc);
                free(x2_v1_1);
                free(x2_1);
                free(x2_v1_1_div);
                free(x2_1_cut);
                free(profits);
                free(weights);
            }

            // free
            free(t_v1_1);
            free(t1_v1_1);
            free(s_v1_1);
            free(t_v2_1);
            free(t1_v2_1);
            free(s_v2_1);
            free(t_v1_1_div);
            free(t1_v1_1_div);
            free(s_v1_1_div);
            free(t_v2_1_cut);
            free(t1_v2_1_cut);
            free(s_v2_1_cut);
        }

    }else if(testmode == 5){    // 5: confronto tempi di esecuzione tra column opt e t'
        printf("typename,s_size_v1,s_size_v2,s_size_v1_1,s_size_v2_1,time_ks2_allocation,time_ks2_v2,time_ks2_v2_allocation_1,time_ks2_v2_1,time_ks2_v1_allocation,time_ks2_v1,time_ks2_v1_allocation_1,time_ks2_v1_1,time_original\n");
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
            minCapv2(weights, n, &t, &t_size, &s, &s_size, capacity, 1);

            // v1 t' --
            int *t_v1_1 = NULL;
            int *t1_v1_1 = NULL;
            int *s_v1_1 = NULL;
            int t_size_v1_1 = 0;
            int s_size_v1_1 = 0;
            minCap_opt_1(weights, n, &t_v1_1, &t1_v1_1, &t_size_v1_1, &s_v1_1, &s_size_v1_1, capacity);

            // v2 t'  --
            int *t_1 = NULL;
            int *t1_1 = NULL;
            int *s_1 = NULL;
            int t_size_1 = 0;
            int s_size_1 = 0;
            minCapv2_1(weights, n, &t_1, &t1_1, &t_size_1, &s_1, &s_size_1, capacity, 1);

            for(size_t i=0; i<allTypesSizes[typeCount] ; i++){
                char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
                strcat(instanceFilename, filesPath);
                strcat(instanceFilename, allTypes[typeCount][i]);

            // BINARYKNAPSACK ----
                readValues_d_i(instanceFilename, &profits, &weights, &n);

            //columOpt_prealloc  (micapv2)       -------------------------------------------------------------------------------------------
                int *x2 = calloc(n, sizeof(int));
                double **mat_ks2 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size, &mat_ks2);                                                  
                end = clock();
                double time_ks2_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);  
                end = clock();
                double time_ks2 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt += x2[k]*profits[k];

                ks2_di_free(n, &mat_ks2);                                                           

            //columOpt_prealloc  (micapv2 + t')       -------------------------------------------------------------------------------------------
                int *x2_1 = calloc(n, sizeof(int));
                double **mat_ks2_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_1, &mat_ks2_1);                                                  
                end = clock();
                double time_ks2_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_1, t_1, t1_1, s_size_1, s_1, mat_ks2_1, x2_1);  
                end = clock();
                double time_ks2_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_1 = 0;                                                                  
                for(int k=0; k<n; k++)
                    resOpt_1 += x2_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_1);                                                           


            //columOpt_prealloc_V1    (micapv1)      -------------------------------------------------------------------------------------------
                int *x2_v1 = calloc(n, sizeof(int));
                double **mat_ks2_v1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1, &mat_ks2_v1);                                           
                end = clock();
                double time_ks2_v1_allocation = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc(profits, weights, capacity, n, t_size_v1, t_v1, s_size_v1, s_v1, mat_ks2_v1, x2_v1); 
                end = clock();
                double time_ks2_v1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1 = 0;                                                              
                for(int k=0; k<n; k++)
                    resOpt_v1 += x2_v1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1);

            //columOpt_prealloc_V1    (micapv1 + t')      -------------------------------------------------------------------------------------------
                int *x2_v1_1 = calloc(n, sizeof(int));
                double **mat_ks2_v1_1 = NULL;

                start = clock();
                ks2_di_alloc(n, s_size_v1_1, &mat_ks2_v1_1);                                          
                end = clock();
                double time_ks2_v1_allocation_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                start = clock();
                ks2_di_prealloc_1(profits, weights, capacity, n, t_size_v1_1, t_v1_1, t1_v1_1, s_size_v1_1, s_v1_1, mat_ks2_v1_1, x2_v1_1); //ks
                end = clock();
                double time_ks2_v1_1 = ((double) (end - start)) / CLOCKS_PER_SEC;

                double resOpt_v1_1 = 0;                                                               
                for(int k=0; k<n; k++)
                    resOpt_v1_1 += x2_v1_1[k]*profits[k];

                ks2_di_free(n, &mat_ks2_v1_1);                                                      

            // original                     -------------------------------------------------------------------------------------------
                int *x_original = calloc(n, sizeof(int));

                start = clock();
                ks_d(profits, weights, capacity, n, x_original);                                  
                end = clock();
                double time_original = ((double) (end - start)) / CLOCKS_PER_SEC;

            //  -----------------------------------------------------------------------------------------------------------------------
                printf("%s,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",allTypesNames[typeCount],s_size_v1,s_size,     s_size_v1_1,s_size_1,   time_ks2_allocation,time_ks2,   time_ks2_allocation_1,   time_ks2_1,   time_ks2_v1_allocation,time_ks2_v1,time_ks2_v1_allocation_1,time_ks2_v1_1,time_original);

            // free                         -------------------------------------------------------------------------------------------
                free(x2);
                free(x2_v1);
                free(x_original);
                free(x2_1);
                free(x2_v1_1);
                free(profits);
                free(weights);
            }

            //free
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
    }
    return 0;
}

//gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ./generator/files/derived/int/ 50000

#include <stdio.h>
#include <stdlib.h>
#include <fminknap_npg.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#include <fnmatch.h>

//#define SHOWRES
//#define SHOWMAT

#define MAX_INSTANCES 15000
#define MAX_FILES 15000
#define MAX_INSTANCE_FILENAME_SIZE 200

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

int readitypeValues(char* filename, itype **profits, itype **weights, int *size){
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

int readValues_d_i(char* filename, double **profits, int **weights, int *size){
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
    
    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1 && itemsNr--) {
        sscanf(line, "%5d %lf %5d", &a, &b, &c);
        (*profits)[a-1] = b;
        (*weights)[a-1] = c; 
    }

    if (ferror(fp)) {
        printf("File error!\n");
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

int readAllFilenames(char* dirname){
    DIR *dir;
    struct dirent *entry;

    dir = opendir(dirname);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}


int minCap_opt(int *weights, int n, int **t, int *size_t, int **s, int *size_s, int capacity) {

    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity>capacity)?capacity:computedCapacity;

    *size_t = capacity+1;

    // Build table
    //int mat[n+1][capacity+1];
    // int **mat = (int**) malloc((n+1)* sizeof(int*));
    // for(int i=0; i<n+1; i++){
    //     mat[i] = (int*) malloc((capacity+1)* sizeof(int));
    // }

    int *c1 = (int*) calloc(capacity+1, sizeof(int));
    int *c2 = (int*)  calloc(capacity+1, sizeof(int));

    int *prev = c2;
    int *current = c1;
    int *aux;

    for(int i = 0; i<n+1;i++) c1[i] = 0;
    for(int i = 0; i<n+1;i++) c2[i] = 0;

    for(int i=0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){

            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : 1;

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

    // Build result arrays
    *size_s = 0;
    for(int i=0; i<capacity+1; i++)
        if(prev[i] <= capacity) (*size_s)++;

    *t = calloc(*size_t, sizeof(int));
    *s = calloc(*size_s, sizeof(int));
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
    
    //free
    // for(int i=0; i<n+1; i++){
    //     free(mat[i]);
    // }
    // free(mat);

    free(c1);
    free(c2);

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


//SEPARA MINCAP DA QUESTO, FINIRE QUI
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

void ks2_di_prealloc(double *profits, int *weights, int capacity, int n, int t_size, int t[t_size], int s_size, int s[s_size], double** mat,  int* x){
    // trivial solution
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }

    // Table creation
    
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0) ? 0 : weights[i-1];
            float p = (i == 0) ? 0 : profits[i-1];
            int clm = s[j];                                 
            int cfr = (clm - w < 0) ? -2 : t[clm - w];
            int prevCol = -1;
            
            if((clm - w) >= 0)
                for(prevCol=(clm-w); t[prevCol] == -1; prevCol--){}
            
            // printf("i = %d, j = %d\n", i, j);
            // printf("cfr = %d\n", cfr);
            // printf("t[clm-w] = %d - clm = %d, w = %d\n", t[clm-w], clm, w);
            // printf("prevCol = %d\n", prevCol);

            double oldVal = (t[clm - w] == -1 && i > 0) ? mat[i-1][t[prevCol]] : (cfr > 0 && i > 0) ? mat[i-1][cfr] : 0;

            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr == -2){
                mat[i][j] = mat[i-1][j];   
            // }else if(t[clm - w]<0 && mat[i-1][j] == 0){
            //     mat[i][j] = profits[0];
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

    //puts("begin\n");

    // instance types initialization ---
    char **filenames = (char**) malloc(MAX_FILES * sizeof(char*));
    
    int fileNr = getFilenames(filesPath, MAX_FILES, filenames);
    //printf("File count: %d\n", fileNr);
    
    char **uncorr = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t uncorrSize;
    getFnmatch(filenames, uncorr, fileNr, &uncorrSize, "uncorr*");
    //printf("uncorr count: %ld\n", uncorrSize);

    char **alm = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t almSize;
    getFnmatch(filenames, alm, fileNr, &almSize, "alm*");
    //printf("alm count: %ld\n", almSize);

    char **inv = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t invSize;
    getFnmatch(filenames, inv, fileNr, &invSize, "inv*");
    //printf("inv count: %ld\n", invSize);

    char **str = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t strSize;
    getFnmatch(filenames, str, fileNr, &strSize, "str*");
    //printf("str count: %ld\n", strSize);

    char **weak = (char**) malloc(MAX_INSTANCES * sizeof(char*));
    size_t weakSize;
    getFnmatch(filenames, weak, fileNr, &weakSize, "weak*");
    //printf("weak count: %ld\n", weakSize);
    
    char** allTypes[] = {uncorr, alm, inv, str, weak};
    char* allTypesNames[] = {"uncorr", "almost", "inv", "str", "weak"};
    size_t allTypesSizes[] = {uncorrSize, almSize, invSize, strSize, weakSize};
    int typeCount = 0;

    //---
    // clock initialization ---

    clock_t start, end;
    double cpu_time_used_opt[5] =       {0.0,0.0,0.0,0.0,0.0};
    double cpu_time_used_fminknap[5] =  {0.0,0.0,0.0,0.0,0.0};
    double opt_time_total = 0;
    double fminknap_time_total = 0;
    
    double res1[5] = {0.0,0.0,0.0,0.0,0.0}; // algorithm with subset-sum
    double res2[5] = {0.0,0.0,0.0,0.0,0.0};
    //---

    // execution ---

    // BINARYKNAPSACK
    
    for(;typeCount<5; typeCount++){
        
        // calcola colonne con minknap
        char instanceFilename0[MAX_INSTANCE_FILENAME_SIZE]="";
        //build filename
        strcat(instanceFilename0, filesPath);
        strcat(instanceFilename0, allTypes[typeCount][0]);
        double *profits;               
        int *weights;
        int n = 0;
        readValues_d_i(instanceFilename0, &profits, &weights, &n);

        int *t;
        int *s;
        int t_size;
        int s_size;
        minCap_opt(weights, n, &t, &t_size, &s, &s_size, capacity);
        //printf(".");
        for(size_t i=0; i<allTypesSizes[typeCount]; i++){
            
            char instanceFilename[MAX_INSTANCE_FILENAME_SIZE]="";
            //build filename
            strcat(instanceFilename, filesPath);
            strcat(instanceFilename, allTypes[typeCount][i]);
            
            // BINARYKNAPSACK
            // readValues_d_i(instanceFilename, &profits, &weights, &n);

            // int *x2 = calloc(n, sizeof(int));

            // double **mat_ks2;
            // ks2_di_alloc(n, s_size, &mat_ks2);

            // start = clock();
            // //ks2_di(profits, weights, capacity, n, t_size, t, s_size, s, x2);
            // //ks2_d(profits, weights, capacity, n, x2);
            // ks2_di_prealloc(profits, weights, capacity, n, t_size, t, s_size, s, mat_ks2, x2);
            // end = clock();
            // cpu_time_used_opt[typeCount] += ((double) (end - start)) / CLOCKS_PER_SEC;

            // //res1
            // for(int k=0; k<n; k++)
            //     res1[typeCount] += x2[k]*profits[k];
            
            // FMINKNAP ----
            
            itype *p_fminknap;              //profitti
            itype *w_fminknap;              //pesi 
            int *x;                         //solution vector
            SolutionList *s_list;           
            int z = 0;                      //optimal objective value
            x = calloc(n, sizeof(int));
            readitypeValues(instanceFilename, &p_fminknap, &w_fminknap, &n);
            openPisinger(n);
            start = clock();
            minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);
            end = clock();
            cpu_time_used_fminknap[typeCount] += ((double) (end - start)) / CLOCKS_PER_SEC;  
            for(int k=0; k<n; k++)
                res2[typeCount] += x[k]*profits[k];
        }

    }
    
    // ---

    // Results
    /*
    printf("\n");

    for(int i=0; i<5; i++){
        printf("-%s:\n", allTypesNames[i]);
        printf("CPU time used with column optimization:        %f\n", cpu_time_used_opt[i]);
        printf("CPU time used with minknap:                    %f\n", cpu_time_used_fminknap[i]);
        printf("res ks:                                        %10lf\n", res1[i]/allTypesSizes[i]);
        printf("res fminknap:                                  %10lf\n", res2[i]/allTypesSizes[i]);
    }

    for(int i=0; i<5; i++){
        opt_time_total+=cpu_time_used_opt[i];
        fminknap_time_total+=cpu_time_used_fminknap[i];
    }
    printf("opt total time:                                %f\n", opt_time_total);
    printf("fminknap total time:                           %f\n", fminknap_time_total);
    printf("capacity:                                    %d\n", capacity);
    */
    //printf("typename, columOpt, fminknap\n");
    for(int i=0; i<5; i++){
        printf("%s, %f, %f\n", allTypesNames[i],  cpu_time_used_opt[i], cpu_time_used_fminknap[i]); 
    }
    return 0;
}
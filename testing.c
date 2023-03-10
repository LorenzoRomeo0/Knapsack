#include <stdio.h>
#include <stdlib.h>
#include <fminknap_npg.h>
#include <time.h>

//#define SHOWRES
#define SHOWMAT


/* ======================================================================
				  utils
   ====================================================================== */


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

int readValues(char* filename, int **profits, int **weights, int *size){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(filename, "r");
    int a=0, b=0, c=0;

    // Read items nr
    int itemsNr = 0;
    read = getline(&line, &len, fp);
    sscanf(line, "%d", &itemsNr);
    *size = itemsNr;
    
    *profits = calloc(*size, sizeof(int));
    *weights = calloc(*size, sizeof(int));

    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1 && itemsNr--) {
        sscanf(line, "%5d %5d %5d", &a, &b, &c);
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

void printm (size_t sizei, size_t sizej, int a[sizei][sizej]){
    /* print matrix */
    for(int i=0; i<sizei; i++){
        for(int j=0; j<sizej; j++){
            printf("%3d ", a[i][j]);
        }
        printf("\n");
    }
}

void printm_d (size_t sizei, size_t sizej, double a[sizei][sizej]){
    /* print matrix */
    for(int i=0; i<sizei; i++){
        for(int j=0; j<sizej; j++){
            printf("%10lf ", a[i][j]);
        }
        printf("\n");
    }
}

/* ======================================================================
				  binary knapsack v1
   ====================================================================== */


void ks(int *profits, int *weights, int capacity, int n, int* x){
    /*  0-1 knapsack */

    // Trivial solution
    int sumWeights = 0;
    for(int i=0; i<n; i++) sumWeights+=weights[i];
    if(capacity >= sumWeights){
        for(int i=0; i<n; i++) x[i] = 1;
        return;
    }
    
    // Table creation
    int mat[n+1][capacity+1];

    for(int i = 0; i<n+1;i++)
        for(int j=0;j<capacity+1;j++) mat[i][j] = 0;

    for(int i=0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){
            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : profits[i-1];

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
    //int *res = calloc(n, sizeof(int));
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(mat[i][indexWeight] != mat[i-1][indexWeight]){
            x[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }

    // Result visualization
    #ifdef SHOWMAT
    int maxprofit = 0;
    printf("matrix:\n");

    for(int j = 0; j < capacity+1; j++) printf("%3d ", j);
    printf("\n");
    for(int j = 0; j < capacity+1; j++) printf("%3s","_");
    printf("\n");    

    printm(n+1, capacity+1, mat);
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
            int p = (i == 0)? 0 : profits[i-1];

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

/* ---------------------------------------------------------------------- */

/* ======================================================================
				  binary knapsack v2
   ====================================================================== */

int minCap(int *weights, int n, int **t, int *size_t, int **s, int *size_s, int capacity) {

    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    

    *size_t = capacity+1;

    // Build table
    int mat[n+1][capacity+1];

    for(int i = 0; i<n+1;i++)
        for(int j=0;j<capacity+1;j++) mat[i][j] = capacity;


    for(int i=0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){

            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : 1;

            if(j == 0){
                mat[i][j] = 0;
            }else if(i == 0){
                mat[i][j] = capacity+1;
            } else if(j-w < 0){ 
                mat[i][j] = mat[i-1][j];
            }else if(mat[i-1][j] < mat[i-1][j-w]+w ){
                mat[i][j] = mat[i-1][j];
            } else{
                mat[i][j] = mat[i-1][j-w]+w; 
            }
        }
    }

    // Build result arrays
    *size_s = 0;
    for(int i=0; i<capacity+1; i++)
        if(mat[n][i] <= capacity) (*size_s)++;

    *t = calloc(*size_t, sizeof(int));
    *s = calloc(*size_s, sizeof(int));
    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(mat[n][i] <= capacity){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    return capacity+1;
}

/*
// Removes the redundant columns
int minCap_d(int *weights, int n, int **t, int *size_t, int **s, int *size_s, int capacity) {

    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    

    *size_t = capacity+1;

    // Build table
    int mat[n+1][capacity+1];

    for(int i = 0; i<n+1;i++)
        for(int j=0;j<capacity+1;j++) mat[i][j] = capacity;

    for(int i=0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){

            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : 1;

            if(j == 0){
                mat[i][j] = 0;
            }else if(i == 0){
                mat[i][j] = capacity+1;
            } else if(j-w < 0){ 
                mat[i][j] = mat[i-1][j];
            }else if(mat[i-1][j] < mat[i-1][j-w]+w ){
                mat[i][j] = mat[i-1][j];
            } else{
                mat[i][j] = mat[i-1][j-w]+w; 
            }
        }
    }

    // Build result arrays
    *size_s = 0;
    for(int i=0; i<capacity+1; i++)
        if(mat[n][i] <= capacity) (*size_s)++;

    *t = calloc(*size_t, sizeof(int));
    *s = calloc(*size_s, sizeof(int));
    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(mat[n][i] <= capacity){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    return capacity+1;
}
*/

// 0-1 knapsack with optimized columns
// Invariant: capacity cannot be greater than the sum of all the weights
void ks2(int *profits, int *weights, int capacity, int n, int* x){

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
    int t_size = 0;
    int s_size = 0;
    minCap(weights, n, &t, &t_size, &s, &s_size, capacity);

    // Table creation
    int mat[n+1][s_size];

    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) mat[i][j] = 0;
        
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : profits[i-1];

            int clm = s[j];
            int cfr = (clm-w < 0)? -1 : (t[ clm - w ]<0) ? t[ clm - w -1 ]: t[ clm - w ];

            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr < 0){
                mat[i][j] = mat[i-1][j];
            }else if(mat[i-1][j] > mat[i-1][cfr]+p){
                mat[i][j] = mat[i-1][j];
           } else{
                mat[i][j] = mat[i-1][cfr]+p; 
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

        for(int j = 0; j < s_size; j++) printf("%3d ", s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        for(int j = 0; j < s_size; j++){
            printf("%3d ", s[j]);
        }
        printf("\n");

        printm(n+1, s_size, mat);
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
    minCap(weights, n, &t, &t_size, &s, &s_size, capacity);

    // Table creation
    double mat[n+1][s_size];

    for(int i = 0; i<n+1; i++)
        for(int j=0; j<s_size; j++) mat[i][j] = 0;
    
    for(int i = 0; i < n+1; i++){
        for(int j = 0; j < s_size; j++){
            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : profits[i-1];

            int clm = s[j];
            int cfr = (clm-w < 0)? -1 : (t[ clm - w ]<0) ? t[ clm - w -1 ]: t[ clm - w ];

            if(i == 0 || j == 0) mat[i][j] = 0;
            else if(cfr < 0){
                mat[i][j] = mat[i-1][j];
            }else if(mat[i-1][j] > mat[i-1][cfr]+p){
                mat[i][j] = mat[i-1][j];
           } else{
                mat[i][j] = mat[i-1][cfr]+p; 
            }
        }
    }

    //int *res = calloc(n, sizeof(int));
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

        for(int j = 0; j < s_size; j++) printf("%3d ", s[j]);
        printf("\n");
        for(int j = 0; j < s_size; j++) printf("%3s","_");
        printf("\n");

        for(int j = 0; j < s_size; j++){
            printf("%3d ", s[j]);
        }
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

/* ---------------------------------------------------------------------- */

/* ======================================================================
				  Main
   ====================================================================== */

void main(int argc, char *argv[]){
    if(argc < 3) {
        printf("Please specify the file containing the values and the capacity of the knapsack:\nbinaryKnapsack2a values.in 10");
        exit(EXIT_FAILURE);
    }
    
    int capacity = atoi(argv[2]);       //capacità
    //int n = 0;                        //numero elementi

    clock_t start, end;
    double cpu_time_used;
    double cpu_time_used_opt;
    double cpu_time_used_to_alloc;
    double cpu_time_used_alloc;
    double cpu_time_used_fminknap;

    // BINARYKNAPSACK
    double *profits;               
    int *weights;
    int n = 0;
    readValues_d_i(argv[1], &profits, &weights, &n);

    // V1 ----
    
    int *x1 = calloc(n, sizeof(int));
    
    start = clock();
    ks_d(profits, weights, capacity, n, x1);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    #ifdef SHOWRES
    printa(x1, n);
    #endif

    double resv1 = 0;
    for(int i=0; i<n; i++){
        resv1 += x1[i]*profits[i];
    }
    printf("resv1 ks:\t\t%10lf\n", resv1);

    // V2 ----
    
    int *x2 = calloc(n, sizeof(int));

    start = clock();
    ks2_d(profits, weights, capacity, n, x2);
    end = clock();
    cpu_time_used_opt = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    #ifdef SHOWRES
    printa(x2, n);
    #endif

    double resv2 = 0;
    for(int i=0; i<n; i++){
        resv2 += x2[i]*profits[i];
    }
    printf("rev2 ks:\t\t%10lf\n", resv2);

}


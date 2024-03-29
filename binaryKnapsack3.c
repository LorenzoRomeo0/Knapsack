#include <stdio.h>
#include <stdlib.h>
#include <fminknap_npg.h>
#include <time.h>

//#define SHOWRES
//#define SHOWMAT


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

int minCap_opt(int *weights, int n, int **t, int *size_t, int **s, int *size_s, int capacity) {

    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    

    *size_t = capacity+1;

    // Build table
    int mat[n+1][capacity+1];

    int c1[capacity+1];
    int c2[capacity+1];

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

void ks2_d1(double *profits, int *weights, int capacity, int n, int* x){

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

    printf("w, p:\n");
    for(int i=0; i<n; i++) printf("%10f ", (double)i);
    printf("\n");
    for(int i=0; i<n; i++) printf("%10f ", (double)weights[i]);
    printf("\n");
    for(int i=0; i<n; i++) printf("%10f ", profits[i]);

    printf("\n");


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

/* ---------------------------------------------------------------------- */

/* ======================================================================
				  binary knapsack v3
   ====================================================================== */

void alloc_minCap(int **t, int *t_size, int ***table_minCap, int capacity, int n, int *weights){

    *t_size = capacity+1;
    (*t) = calloc(*t_size, sizeof(int));

    (*table_minCap) = malloc (sizeof(int[n+1][capacity+1]));
    
    for(int i = 0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){
            ((int (*)[(int)(capacity+1)])*table_minCap)[i][j] = 0;
        }
    }
}

void alloc_ks2(int s_size, int n, int ***table_ks){

    (*table_ks) = malloc (sizeof(int[n+1][s_size+1]));
    for(int i = 0; i<n+1; i++){
        for(int j=0; j<s_size+1; j++){
            ((int (*)[(int)(s_size+1)])*table_ks)[i][j] = 0;
        }
    }
}

int minCap_a(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity, int table_minCap[][capacity+1]){
    // Compute capacity
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    
    // Build table
    for(int i = 0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++) table_minCap[i][j] = capacity;
    }

    for(int i=0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){
            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : 1;

            if(j == 0){
                table_minCap[i][j] = 0;
            }else if(i == 0){
                table_minCap[i][j] = capacity+1;
            } else if(j-w < 0){ 
                table_minCap[i][j] = table_minCap[i-1][j];
            }else if(table_minCap[i-1][j] < table_minCap[i-1][j-w]+w ){
                table_minCap[i][j] = table_minCap[i-1][j];
            } else{
                table_minCap[i][j] = table_minCap[i-1][j-w]+w; 
            }
        }
    }

    
    // Build result arrays
    *s_size = 0;
    *t_size = capacity+1;
    for(int i=0; i<capacity+1; i++)
        if(table_minCap[n][i] <= capacity) (*s_size)++;

    *t = calloc(*t_size, sizeof(int));
    *s = calloc(*s_size, sizeof(int));
    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(table_minCap[n][i] <= capacity){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    return capacity+1;
}

void ks2_a(int *profits, int *weights, int capacity, int n, int s_size, int t_size, int *t, int *s, int mat[n+1][s_size], int* x){
    
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
    int maxprofit = 0;

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
    printf("Result: ");
    for(int i=0; i<n; i++){
            printf("(%d)%d%s", x[i], i, (i+1==n)?".\n":", ");
            maxprofit += x[i]*profits[i];
        }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    #endif
    */
}

void alloc_minCap_d(int **t, int *t_size, double ***table_minCap, int capacity, int n, int *weights){

    *t_size = capacity+1;
    (*t) = calloc(*t_size, sizeof(int));

    (*table_minCap) = malloc (sizeof(double[n+1][capacity+1]));
    
    for(int i = 0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){
            ((int (*)[(int)(capacity+1)])*table_minCap)[i][j] = 0;
        }
    }
}

void alloc_ks2_d(int s_size, int n, double ***table_ks){

    (*table_ks) = malloc (sizeof(double[n+1][s_size+1]));
    for(int i = 0; i<n+1; i++){
        for(int j=0; j<s_size+1; j++){
            ((double (*)[(int)(s_size+1)])*table_ks)[i][j] = 0;
        }
    }
}

int minCap_a_d(int *weights, int n, int **t, int *t_size, int **s, int *s_size, int capacity, double table_minCap[][capacity+1]){
    // Compute capacity
    int computedCapacity = 0;
    for(int i=0; i<n; i++) computedCapacity+=weights[i]; 
    capacity = (computedCapacity>capacity)?capacity:computedCapacity;
    
    // Build table
    for(int i = 0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++) table_minCap[i][j] = capacity;
    }

    for(int i=0; i<n+1; i++){
        for(int j=0; j<capacity+1; j++){
            int w = (i == 0)? 0 : weights[i-1];
            int p = (i == 0)? 0 : 1;

            if(j == 0){
                table_minCap[i][j] = 0;
            }else if(i == 0){
                table_minCap[i][j] = capacity+1;
            } else if(j-w < 0){ 
                table_minCap[i][j] = table_minCap[i-1][j];
            }else if(table_minCap[i-1][j] < table_minCap[i-1][j-w]+w ){
                table_minCap[i][j] = table_minCap[i-1][j];
            } else{
                table_minCap[i][j] = table_minCap[i-1][j-w]+w; 
            }
        }
    }

    
    // Build result arrays
    *s_size = 0;
    *t_size = capacity+1;
    for(int i=0; i<capacity+1; i++)
        if(table_minCap[n][i] <= capacity) (*s_size)++;

    *t = calloc(*t_size, sizeof(double));
    *s = calloc(*s_size, sizeof(double));
    int counter = 0;
    int s_cont = 0;
    
    for(int i=0; i<capacity+1; i++){
        if(table_minCap[n][i] <= capacity){
            (*t)[i] = counter++;
            (*s)[s_cont++] = i;
        }else{
            (*t)[i] = -1;
        }
    }
    return capacity+1;
}

void ks2_a_d(double *profits, int *weights, int capacity, int n, int s_size, int t_size, int *t, int *s, double mat[n+1][s_size], int* x){
    
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
    int maxprofit = 0;
    printf("matrix:\n");

    for(int j = 0; j < s_size; j++) printf("%10d ", s[j]);
    printf("\n");
    for(int j = 0; j < s_size; j++) printf("%3s","_");
    printf("\n");

    for(int j = 0; j < s_size; j++){
        printf("%10d ", s[j]);
    }
    printf("\n");

    printm_d(n+1, s_size, mat);
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
				  Main
   ====================================================================== */

void main(int argc, char *argv[]){
    if(argc < 3) {
        printf("Please specify the file containing the values and the capacity of the knapsack:\nbinaryKnapsack2a values.in 10");
        exit(EXIT_FAILURE);
    }
    
    int capacity = atoi(argv[2]);       //capacità
    //int n = 0;                        //numero elementi

    printf("BEGIN\n");
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

    // V3 ----
    
    int *t;
    int t_size;
    int *s;
    int s_size;
    double **table_minCap;
    double **table_ks;
    int *x3 = calloc(n, sizeof(int));

    start = clock();
    alloc_minCap_d(&t, &t_size, &table_minCap, capacity, n, weights);
    minCap_a_d(weights, n, &t, &t_size, &s, &s_size, capacity, (double (*)[(int)(capacity+1)])table_minCap);
    alloc_ks2_d(s_size, n, &table_ks);
    end = clock();
    cpu_time_used_to_alloc = ((double) (end - start)) / CLOCKS_PER_SEC;

    start = clock();
    ks2_a_d(profits, weights, capacity, n, s_size, t_size, t, s, (double (*)[(int)(s_size+1)])table_minCap, x3);
    end = clock();
    cpu_time_used_alloc = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    #ifdef SHOWRES
    printa(x3, n);
    #endif

    double res1 = 0;
    for(int i=0; i<n; i++){
        res1 += x3[i]*profits[i];
    }
    printf("res ks:\t\t\t%10lf\n", res1);

    // FMINKNAP ----
    itype *p_fminknap;              //profitti
    itype *w_fminknap;              //pesi 
    int *x;                         //solution vector
    SolutionList *s_list;           
    int z = 0;                      //optimal objective value

    x = calloc(n, sizeof(int));
    readitypeValues(argv[1], &p_fminknap, &w_fminknap, &n);

    openPisinger(n);

    start = clock();
    minknap(&s_list, n, p_fminknap, w_fminknap, x, capacity);
    end = clock();
    cpu_time_used_fminknap = ((double) (end - start)) / CLOCKS_PER_SEC;

    #ifdef SHOWRES
    printa(x, n);
    #endif

    closePisinger();

    double res2 = 0;
    for(int i=0; i<n; i++){
        res2 += x[i]*profits[i];
    }
    printf("res fminknap:\t%10lf\n", res2);
   
    // RESULTS ----

    printf("CPU time used with standard implementation:\t\t\t\t\t\t%f\n", cpu_time_used);
    printf("CPU time used with column optimization:\t\t\t\t\t\t\t%f\n", cpu_time_used_opt);
    printf("CPU time used with column optimization and early allocation:\t%f\n", cpu_time_used_alloc);
    printf("CPU time used with to allocate:\t\t\t\t\t\t\t\t\t%f\n", cpu_time_used_to_alloc);
    printf("CPU time used with fminknap:\t\t\t\t\t\t\t\t\t%f\n", cpu_time_used_fminknap);
    printf("CPU time gain with column opt:\t\t\t\t\t\t\t\t\t%f\n", cpu_time_used - cpu_time_used_opt);
    printf("CPU time gain with early allocation:\t\t\t\t\t\t\t%f\n", cpu_time_used - cpu_time_used_alloc);
    printf("Difference between fminknap and ks:\t\t\t\t\t\t\t\t%f\n", ((res2>res1)?res2-res1:res1-res2));

}


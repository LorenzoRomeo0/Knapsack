#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEBUG 0
#define SHOWRESULT 0

// print array
void printa(int a[], size_t size){
    for(int i = 0; i<size; i++)
        printf("%2d ",a[i]);
    printf("\n");
}

// print matrix
void printm (size_t sizei, size_t sizej, int a[sizei][sizej]){
    for(int i=0; i<sizei; i++){
        for(int j=0; j<sizej; j++){
            printf("%3d ", a[i][j]);
        }
        printf("\n");
    }
}

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

void alloc_ks2(int s_size, int n, int ***table_ks, int **res_ks){
    (*res_ks) = calloc(n, sizeof(int));

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

    if(DEBUG){
        printf("s-size: %d\n", *s_size);
        printf("t-size: %d\n", *t_size);
    }
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

// Removes the redundant columns
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

    if(DEBUG){
        printf("s-size: %d\n", *size_s);
        printf("t-size: %d\n", *size_t);
    }
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

// 0-1 knapsack
void ks(int *profits, int *weights, int capacity, int n, short showMatrix){
    
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
    int *res = calloc(n, sizeof(int));
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(mat[i][indexWeight] != mat[i-1][indexWeight]){
            res[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }

    // Result visualization
    int maxprofit = 0;

    if(showMatrix){
        printf("matrix:\n");

        for(int j = 0; j < capacity+1; j++) printf("%3d ", j);
        printf("\n");
        for(int j = 0; j < capacity+1; j++) printf("%3s","_");
        printf("\n");    

        printm(n+1, capacity+1, mat);
        printf("\n");
    }

    if(SHOWRESULT){
        printf("Result: ");
        for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];

        }
        printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    }
}

// 0-1 knapsack with optimized columns
// Invariant: capacity cannot be greater than the sum of all the weights
void ks2(int *profits, int *weights, int capacity, int n, short showMatrix){

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

    int *res = calloc(n, sizeof(int));
    int remainingCapacity = capacity;
    int indexWeight = capacity;

    for(int i=n; i>0; i--){
        if(t[indexWeight]<0){
            indexWeight--;
            i++;
            continue;
        } 
        if(mat[i][t[indexWeight]] != mat[i-1][t[indexWeight]]){
            res[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization
    int maxprofit = 0;

    if(showMatrix){
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
    }

    if(SHOWRESULT){
        printf("Result: ");
        for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
        printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    }
}

void ks2_a(int *profits, int *weights, int capacity, int n, int s_size, int t_size, int *res, int *t, int *s, int mat[n+1][s_size], short showMatrix){
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
            res[i-1] = 1;
            remainingCapacity -= weights[i-1];
            indexWeight = remainingCapacity;      
        }
    }
    
    // Result visualization
    int maxprofit = 0;

    if(showMatrix){
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
    }

    if(SHOWRESULT){
        printf("Result: ");
        for(int i=0; i<n; i++){
            printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
            maxprofit += res[i]*profits[i];
        }
        printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
    }
}

//int readVaues(char* filename, int *size, int **weights, int **profits){
void readVaues(char* filename, int **profits, int **weights, int *size){
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

int main(int argc, char *argv[]){
    
    if(argc < 3) {
        printf("Please specify the file containing the values and the capacity of the knapsack:\nbinaryKnapsack2a values.in 10");
        exit(EXIT_FAILURE);
    }
    
    int *profits;
    int *weights;
    int size = 0;
    readVaues(argv[1], &profits, &weights, &size);

    clock_t start, end;
    double cpu_time_used;
    double cpu_time_used_opt;
    double cpu_time_used_alloc;

    int *t;
    int t_size;
    int *s;
    int s_size;
    int capacity = 30;
    int **table_minCap;
    int **table_ks;
    int *res_ks;
    
    alloc_minCap(&t, &t_size, &table_minCap, capacity, size, weights);
    minCap_a(weights, size, &t, &t_size, &s, &s_size, capacity, (int (*)[(int)(capacity+1)])table_minCap);

    alloc_ks2(s_size, size, &table_ks, &res_ks);
    //ks2(profits, weights, capacity, size, 0);
    //ks2_a(profits, weights, capacity, size, s_size, t_size, res_ks, t, s, (int (*)[(int)(s_size+1)])table_minCap, 0);

    start = clock();
    ks2(profits, weights, atoi(argv[2]), size, 0);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    start = clock();
    ks2(profits, weights, atoi(argv[2]), size, 0);
    end = clock();
    cpu_time_used_opt = ((double) (end - start)) / CLOCKS_PER_SEC;

    start = clock();
    ks2_a(profits, weights, capacity, size, s_size, t_size, res_ks, t, s, (int (*)[(int)(s_size+1)])table_minCap, 0);
    end = clock();
    cpu_time_used_alloc = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("CPU time used with standard implementation:\t\t\t\t\t\t%f\n", cpu_time_used);
    printf("CPU time used with column optimization:\t\t\t\t\t\t\t%f\n", cpu_time_used_opt);
    printf("CPU time used with column optimization and early allocation:\t%f\n", cpu_time_used_alloc);
    printf("CPU time gain:\t\t\t\t\t\t\t\t\t\t\t\t\t%f\n", cpu_time_used - cpu_time_used_opt);
    printf("CPU time gain with early allocation:\t\t\t\t\t\t\t%f\n", cpu_time_used - cpu_time_used_alloc);
 
    return 0;
}
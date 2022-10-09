#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0

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

// Removes the redundant columns
int minCap(int *weights, int n, int **t, int *size_t, int **s, int *size_s) {

    // Compute capacity
    int capacity = 0;
    for(int i=0; i<n; i++) capacity+=weights[i]; 
    
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

    printf("Result: ");
    for(int i=0; i<n; i++){
        printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
        maxprofit += res[i]*profits[i];

    }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
}

// 0-1 knapsack with optimized columns
// Invariant: capacity cannot be greater than the sum of all the weights
void ks2(int *profits, int *weights, int capacity, int n, short showMatrix){

    // Get column values
    int *t;
    int *s;
    int t_size = 0;
    int s_size = 0;
    minCap(weights, n, &t, &t_size, &s, &s_size);

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

    printf("Result: ");
    for(int i=0; i<n; i++){
        printf("(%d)%d%s", res[i], i, (i+1==n)?".\n":", ");
        maxprofit += res[i]*profits[i];
    }
    printf("Maximum profits: %d (for %d capacity)\n", maxprofit, capacity);
}

int main(){
    printf("Test1\n");
    ks( (int[]){1,1,1,1},(int[]){2,3,5,7}, 17, 4, 1);
    ks2((int[]){1,1,1,1},(int[]){2,3,5,7}, 17, 4, 1);
    printf("----\nTest2\n");
    ks( (int[]){1,4,5,7},(int[]){1,3,4,5}, 19, 4, 0);
    ks2((int[]){1,4,5,7},(int[]){1,3,4,5}, 19, 4, 0);
    printf("----\nTest3\n");
    ks( (int[]){1,4,5,7},(int[]){1,3,4,5}, 4, 4, 0);
    ks2((int[]){1,4,5,7},(int[]){1,3,4,5}, 4, 4, 0);
    printf("----\nTest4\n");
    ks( (int[]){5,4,3,2},(int[]){4,3,2,1}, 10, 4, 1);
    ks2((int[]){5,4,3,2},(int[]){4,3,2,1}, 10, 4, 1);
    return 0;
}
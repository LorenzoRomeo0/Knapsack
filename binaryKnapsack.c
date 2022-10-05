#include <stdio.h>
#include <stdlib.h>

// print array
void printa(int a[], size_t size){
    for(int i = 0; i<size; i++)
        printf("%3d ",a[i]);
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
            else if(mat[i-1][j] > mat[i-1][j-w]+p || j-w<0)
                mat[i][j] = mat[i-1][j];
            else
                mat[i][j] = mat[i-1][j-w]+p; 
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

int main(){
    int profits[] = {1,2,5,6};
    int weights[] = {2,3,4,5};
    int capacity = 8;
    int n = 4;

    ks(profits, weights, capacity, n, 1);
    printf("\n");

    ks((int[]){5,4,3,2},(int[]){4,3,2,1},6,4, 1);
    printf("\n");
    ks((int[]){1,4,5,7},(int[]){1,3,4,5},7,4, 1);
    
    printf("\n");
    return 0;
}
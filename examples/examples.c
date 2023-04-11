#include <stdio.h>
#include <stdlib.h>
#include <fminknap_npg.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <fnmatch.h>

void printa(int a[], size_t size){
    /* print array */
    for(int i = 0; i<size; i++)
        printf("%3d ",a[i]);
    printf("\n");
}

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
        }
        aux = current;
        current = prev;
        prev = aux;
    }

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


int main(){
    double profits[] = {1,1,1,2,1,1,1,2,1,1,1,2,1,1,9,6,0,5,4,7};               
    int weights[] =    {1,2,3,6,7,2,2,8,1,3,5,5,8,6,9,6,1,5,3,9};
    int n = 0;
    int capacity = 300;

    int *t_v1 = NULL;
    int *s_v1 = NULL;
    int t_size_v1 = 0;
    int s_size_v1 = 0;
    minCap_opt(weights, n, &t_v1, &t_size_v1, &s_v1, &s_size_v1, capacity);

    printa(t_v1, t_size_v1);
    printa(s_v1, s_size_v1);
}
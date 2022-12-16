#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>

double rand_normal(double mean, double stddev) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);

    static double n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached) {
        double x, y, r;
        do {
          x = 2.0*rand()/RAND_MAX - 1;
          y = 2.0*rand()/RAND_MAX - 1;
         r = x*x + y*y;
        } while (r == 0.0 || r > 1.0);

        double d = sqrt(-2.0*log(r)/r);
        double n1 = x*d;
        n2 = y*d;
        double result = n1*stddev + mean;
        n2_cached = 1;
        return result;
    } else {
        n2_cached = 0;
        return n2*stddev + mean;
    }
    /*
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    srand(tv.tv_usec);
    double val = rand_normal(0, 0.5);
    printf("%f \n", val);
    */
}

void derive_int_instances(char* filename, int instance_n){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(filename, "r");
    int a=0, b=0, c=0;
    char *new_filename;
    FILE *new_fp;
    
    // Read items nr
    int itemsNr = 0;
    read = getline(&line, &len, fp);
    sscanf(line, "%d", &itemsNr);
    
    for(int instance_count = 0; instance_count<instance_n; instance_count++){
        char instance_count_string[12];

        sprintf(instance_count_string, "%d", instance_count);
        new_filename = calloc(strlen(filename)+strlen(instance_count_string), sizeof(char));
        strcat(new_filename, filename);
        strcat(new_filename, instance_count_string);
        
        new_fp = fopen(new_filename, "w");
        
        rewind(fp);
        read = getline(&line, &len, fp);
        sscanf(line, "%d", &itemsNr);
        fprintf(new_fp, "%d\n", itemsNr);
        
        if (fp == NULL)
            exit(EXIT_FAILURE);
        while ((read = getline(&line, &len, fp)) != -1 && itemsNr--) {
            double random_nr = rand_normal(0, 0.5);
            random_nr = (random_nr<0)?-random_nr:random_nr;
            sscanf(line, "%5d %5d %5d", &a, &b, &c);
            //fprintf(new_fp, "%5d %5d %f\n", a, b, c*random_nr);
            fprintf(new_fp, "%5d %lf %5d\n", a, (double)b, c);
        }

        free(new_filename);
        if (ferror(fp) || ferror(new_fp)) {
            printf("File error!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    free(line);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("parameters needed: [filename, instance_number]");
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];
    int instance_nr = atoi(argv[2]);
    derive_int_instances(filename, instance_nr);
    
    return 0;
}



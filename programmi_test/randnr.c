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

int main(void) {
    double mean = 0;
    double stddev = 0.5;

    double n = 0;
    printf("numero_random\n");
    for(int i=0; i<2000; i++){
        n = rand_normal(mean, stddev);
        printf("%f\n", n);
    }
    return 0;
}

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <thread_db.h>

FILE *X_train, *y_train, *X_test;
pthread_t ntid[105];
double lr;
int qwd;
unsigned char x_in[60000][784], y_in2[60000], out[10000];
double x_t[784][60000];
unsigned char x_tes[10000][784], y_tes[10000];
double y_hat[60000][10], y3[60000][10], y_in[60000][10];
double y_hat_tes[10000][10];
double w[784][10], w_grad[784][10]; 

void *xmultiw(void *vptr)
{
    int cnt = *( (int*)vptr);
    for ( int i = (cnt*qwd); i < (cnt*qwd)+qwd; i++ ) {
        double max = -1000000000000000000;
        for ( int j = 0; j < 10; j++ ) {
            double sum1 = 0;
            for ( int k = 0; k < 784; k++ ) {
                sum1 += x_in[i][k]*w[k][j];
            }
            y_hat[i][j] = sum1;
            if ( sum1 > max ) max = sum1;
        }
        double sum = 0.0;
        for ( int j = 0; j < 10; j++) {
            sum += exp(y_hat[i][j] - max);
        }
        double offset = max + log(sum);
        for ( int j = 0; j < 10; j++) {
            y_hat[i][j] = exp(y_hat[i][j] - offset);
        }
    }
    pthread_exit(NULL);
}

int  main(int argc, char **argv)
{
    lr = 0.020;
    int thread_num = atoi(argv[4]);
    qwd = 60000/thread_num;
    FILE *X_train = fopen(argv[1], "r");
    FILE *y_train = fopen(argv[2], "r");
    FILE *X_test = fopen(argv[3], "r");
    FILE *y_test = fopen("y_test", "r");
    fread(&x_in, 1, 60000*784, X_train);
    fread(&y_in2, 1, 60000, y_train);
    fread(&x_tes, 1, 10000*784, X_test);
    fread(&y_tes, 1, 10000, y_test);
    for ( int i = 0; i < 60000; i++ ) {
        y_in[i][y_in2[i]] = 1;
    }
    int err;
    int intarr[105];
    for ( int i = 0; i < thread_num; i++ ) {
        intarr[i] = i;
    }
    for ( int i = 0; i < 60000; i++ ) {
        for ( int j = 0; j < 784; j++ ) {
            x_t[j][i] = x_in[i][j];
        }
    }
    //1 x*w
    for ( int kkk = 0; kkk < 15; kkk++ ) {
        for ( int i = 0; i < thread_num; i++ ) {
            err = pthread_create(&ntid[i], NULL, xmultiw, (void *)&intarr[i]);
        }
        for ( int i = 0; i < thread_num; i++ ) {
            pthread_join(ntid[i], NULL);
        }
        //4 w_grad = xt*(y_hat-y);
        for ( int i = 0; i < 60000; i++ ) {
            for ( int j = 0; j < 10; j++ ) {
                y3[i][j] = y_hat[i][j] - y_in[i][j];
            }
        }
        for ( int i = 0; i < 784; i++ ) {
            for ( int j = 0; j < 10; j++ ) {
                double sum = 0;
                for ( int k = 0; k < 60000; k++ ) {
                    sum += x_t[i][k]*y3[k][j];
                }
                w_grad[i][j] = sum;
            }
        }
        //3
        for ( int i = 0; i < 784; i++ ) {
            for ( int j = 0; j < 10; j++ ) {
                w[i][j] = w[i][j] - lr*w_grad[i][j]; 
            }
        }
    }
    //
    for ( int i = 0; i < 10000; i++ ) {
        double max = -1000000000000000000;
        for ( int j = 0; j < 10; j++ ) {
            double sum1 = 0;
            for ( int k = 0; k < 784; k++ ) {
                sum1 += x_tes[i][k]*w[k][j];
            }
            y_hat_tes[i][j] = sum1;
            if ( sum1 > max ) max = sum1;
        }
        double sum = 0;
        for ( int j = 0; j < 10; j++) {
            sum += exp(y_hat_tes[i][j] - max);
        }
        double offset = max + log(sum);
        for ( int j = 0; j < 10; j++) {
            y_hat_tes[i][j] = exp(y_hat_tes[i][j] - offset);
        }
    }
    //test for correct
    double correct = 0;
    for ( int i = 0; i < 10000; i++ ) {
        unsigned char maxj = 0;
        double max = -10000000;
        for ( unsigned char j = 0; j < 10; j++ ) {
            if( y_hat_tes[i][j] > max) {
                max = y_hat_tes[i][j];
                maxj = j;
            }
        }
        out[i] = maxj;
        if ( maxj == y_tes[i] ) correct++;
    }
    //
    FILE*fp=fopen("result.csv","w");
    fprintf(fp, "id,label\n");
    for ( int i = 0; i < 10000; i++ ) {
        fprintf(fp, "%d,%hhu\n", i, out[i]);
    }
    printf("%lf\n", correct/10000);
    fclose(X_train);
    fclose(y_train);
    fclose(X_test);
    fclose(y_test);
    fclose(fp);
    return 0;
}
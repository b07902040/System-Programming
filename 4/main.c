#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include "scheduler.h"
#define SIGUSR3 SIGWINCH

int main(int argc, char** agrv)
{
    //read the input
    int PP, QQ, RR;
    char cP[10], cQ[10], cthree[10], czero[10];
    int ithree = 3, izero = 0;
    int res[12];
    scanf("%d%d", &PP, &QQ);
    scanf("%d", &RR);
    sprintf(&cP[0], "%d", PP);
    sprintf(&cQ[0], "%d", QQ);
    sprintf(&cthree[0], "%d", ithree);
    sprintf(&czero[0], "%d", izero);
    for ( int i = 0; i < RR; i++ ) {
        scanf("%d", &res[i]);
    }
    //fork
    int piipe[2];
    pipe(piipe);
    pid_t pid;
    pid = fork();
    if ( pid == 0 ) {
        dup2(piipe[1], 1);
        execlp("./hw3", "./hw3", cP, cQ, cthree, czero, NULL);
    }
    FILE *fin = fdopen(piipe[0], "r");
    //sent signal
    close(piipe[1]);
    char get[10000];
    for ( int i = 0; i < RR; i++ ) {
        sleep(5);
        if ( res[i] == 1 ) {
            kill( pid, SIGUSR1 );
            //printf("singal 1 sented\n");
        }
        if ( res[i] == 2 ) {
            kill( pid, SIGUSR2 );
            //printf("singal 2 sented\n");
        }
        if ( res[i] == 3 ) {
            kill( pid, SIGUSR3 );
            //printf("singal 3 sented\n");
        }
        fflush(fin);
        fscanf(fin, "%s", &get[0]);
        int j = 0;
        unsigned long long int k = atoi(&get[0]);
        if ( k != 9 ) {
            unsigned long long int big = 1;
            while ( k >= big ) {
                big *= 10;
            } 
            big /= 10;
            int pr;
            while ( k>1 ) {
                pr = k/big;
                printf("%d ", pr);
                k -= pr*big;
                big /= 10;
            }
            printf("\n");
        }
    }
    char arrrr[10000];
    fflush(fin);
    fscanf(fin, "%s", &arrrr[0]);
    printf("%s\n", arrrr);
    waitpid(pid, NULL, 0);
    close(piipe[0]);
    return 0;
}
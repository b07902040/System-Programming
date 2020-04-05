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

FCB_ptr now1, now2, now3, now4;
jmp_buf mainnn, SCHEDULER;
FILE *fout;
int P, Q, task2;
int kkk;
int mutex;
int idx;
int cnt1, cnt2, cnt3, cnt4;
char arr[10000];
FCB_ptr Current, Head;
sigset_t emptymask, allmask;
int queue[10000];
int cntq;
int flag1, flag2, flag3, flag4;

void funct_1(int name);
void funct_2(int name);
void funct_3(int name);
void funct_4(int name);
void funct_5(int name);
void act1 ( int signo )
{
    sigset_t old;
    //fprintf(stderr, "receive sigusr1\n");
    char ACK[10000];
    int inine = 9;
    sprintf(&ACK[0], "%d\n", inine);
    printf("%s", ACK);
    fflush(stdout);
    sigprocmask( SIG_BLOCK, &allmask, &old);
    longjmp(SCHEDULER, 1);
}
void act2 ( int signo )
{
    sigset_t old;
    //fprintf(stderr, "receive sigusr2\n");
    char ACK[10000];
    int inine = 9;
    sprintf(&ACK[0], "%d\n", inine);
    //
    printf("%s", ACK);
    fflush(stdout);
    sigprocmask( SIG_BLOCK, &allmask, &old);
    longjmp(SCHEDULER, 1);
}
void act3 ( int signo )
{
    sigset_t old;
    //fprintf(stderr, "receive sigusr3\n");
    char ACK[10000];
    for ( int i = 0; i < cntq; i++ ) {
        sprintf(&ACK[i], "%d", queue[i]);
    }
    ACK[cntq] = '\n';
    //
    printf("%s", ACK);
    fflush(stdout);
    Current = Current->Previous;
    sigprocmask( SIG_BLOCK, &allmask, &old);
    longjmp(SCHEDULER, 1);
}

int main(int argc, char** argv)
{
    int testnum = atoi(argv[3]);
    if ( testnum == 1 ) {
        P = atoi(argv[1]);
        Q = atoi(argv[2]);
        task2 = P;
        now1 = malloc(sizeof(FCB));
        now2 = malloc(sizeof(FCB));
        now3 = malloc(sizeof(FCB));
        now4 = malloc(sizeof(FCB));
        Head = malloc(sizeof(FCB));
        Current = malloc(sizeof(FCB));
        if ( setjmp(mainnn) == 0 ) {
            funct_5(5);
        }
        else {
            now1->Name = 1;
            now2->Name = 2;
            now3->Name = 3;
            now4->Name = 4;
            now1->Next = now2;
            now2->Next = now3;
            now3->Next = now4;
            now4->Next = now1;
            now1->Previous = now4;
            now2->Previous = now1;
            now3->Previous = now2;
            now4->Previous = now3;
            Head = now1;
            Current = now4;
            cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
            Scheduler();
        }
    }
    else if ( testnum == 2 ) {
        P = atoi(argv[1]);
        Q = atoi(argv[2]);
        task2 = atoi(argv[4]);
        now1 = malloc(sizeof(FCB));
        now2 = malloc(sizeof(FCB));
        now3 = malloc(sizeof(FCB));
        now4 = malloc(sizeof(FCB));
        Head = malloc(sizeof(FCB));
        Current = malloc(sizeof(FCB));
        if ( setjmp(mainnn) == 0 ) {
            funct_5(5);
        }
        else {
            now1->Name = 1;
            now2->Name = 2;
            now3->Name = 3;
            now4->Name = 4;
            now1->Next = now2;
            now2->Next = now3;
            now3->Next = now4;
            now4->Next = now1;
            now1->Previous = now4;
            now2->Previous = now1;
            now3->Previous = now2;
            now4->Previous = now3;
            Head = now1;
            Current = now4;
            cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
            Scheduler();
        }
    }
    else if ( testnum == 3 ) {
        cntq = 0;
        //register signal
        sigemptyset(&emptymask);
        struct sigaction action1, action2, action3;
        sigset_t oldmask;
        sigemptyset(&allmask);
        sigaddset(&allmask, SIGUSR1);
        sigaddset(&allmask, SIGUSR2);
        sigaddset(&allmask, SIGUSR3);
        action1.sa_handler = act1;
        action2.sa_handler = act2;
        action3.sa_handler = act3;
        action1.sa_flags = 0;
        action2.sa_flags = 0;
        action3.sa_flags = 0;
        action1.sa_mask = allmask;
        action2.sa_mask = allmask;
        action3.sa_mask = allmask;
        sigaction(SIGUSR1, &action1, NULL);
        sigaction(SIGUSR2, &action2, NULL);
        sigaction(SIGUSR3, &action3, NULL);
        //block all signal
        sigprocmask( SIG_BLOCK, &allmask, &oldmask);
        //
        P = atoi(argv[1]);
        Q = atoi(argv[2]);
        task2 = P;
        now1 = malloc(sizeof(FCB));
        now2 = malloc(sizeof(FCB));
        now3 = malloc(sizeof(FCB));
        now4 = malloc(sizeof(FCB));
        Head = malloc(sizeof(FCB));
        Current = malloc(sizeof(FCB));
        if ( setjmp(mainnn) == 0 ) {
            funct_5(5);
        }
        else {
            now1->Name = 1;
            now2->Name = 2;
            now3->Name = 3;
            now4->Name = 4;
            now1->Next = now2;
            now2->Next = now3;
            now3->Next = now4;
            now4->Next = now1;
            now1->Previous = now4;
            now2->Previous = now1;
            now3->Previous = now2;
            now4->Previous = now3;
            Head = now1;
            Current = now4;
            cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
            Scheduler();
        }
    }
    return 0;
}
void funct_1(int name)
{
    int i, j;
    if ( setjmp(now1->Environment) == 0 ) {
        kkk++;
        funct_5(5);
    }
    else {
        while(1) {
            if ( mutex == 0 || mutex == 1 ) {
                mutex = 1;
                if ( cntq != 0 && queue[0] == 1 ) { 
                    for ( int iii = 0; iii < cntq-1; iii++ ) {
                        queue[iii] = queue[iii+1];
                    }
                    cntq--;
                }
                sigset_t old;
                for ( j = 1; j <= P; j++ ) {
                    if( ((cnt1%task2)== 0) && cnt1!=P && cnt1 != 0 && flag1 == 0) {
                        mutex = 0;
                        flag1 = 1;
                        longjmp(SCHEDULER, 1);
                    }
                    else if( cnt1 == P ) {//整個結束
                        mutex = 0;
                        longjmp(SCHEDULER, -2);
                    }
                    flag1 = 0;
                    cnt1++;
                    for ( i = 1; i <= Q; i++ ) {
                        sleep(1);
                        //fprintf(stderr, "hi1\n");
                        arr[idx++] = '1';
                    }
                    sigset_t receive;
                    sigpending(&receive);
                    if( sigismember(&receive, SIGUSR1) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR2) ) {
                        mutex = 0;
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR3) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                }
            }
            else {
                queue[cntq++] = 1;
                longjmp(SCHEDULER, 1);
            }
        }
        //整個結束
        mutex = 0;
        longjmp(SCHEDULER, -2);
    }
}

void funct_2(int name)
{
    int i, j;
    if ( setjmp(now2->Environment) == 0 ) {
        kkk++;
        funct_5(5);
    }
    else {
        while(1) {
            if ( mutex == 0 || mutex == 2 ) {
                mutex = 2;
                if ( cntq != 0 && queue[0] == 2 ) { 
                    for ( int iii = 0; iii < cntq-1; iii++ ) {
                        queue[iii] = queue[iii+1];
                    }
                    cntq--;
                }
                sigset_t old;
                for ( j = 1; j <= P; j++ ) {
                    if( ((cnt2%task2)== 0) && cnt2!=P  && cnt2 != 0 && flag2 == 0) {
                        mutex = 0;
                        flag2 = 1;
                        longjmp(SCHEDULER, 1);
                    }
                    else if( cnt2 == P ) {//整個結束
                        mutex = 0;
                        longjmp(SCHEDULER, -2);
                    }
                    flag2 = 0;
                    cnt2++;
                    for ( i = 1; i <= Q; i++ ) {
                        sleep(1);
                        //fprintf(stderr, "hi2\n");
                        arr[idx++] = '2';
                    }
                    sigset_t receive;
                    sigpending(&receive);
                    if( sigismember(&receive, SIGUSR1) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR2) ) {
                        mutex = 0;
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR3) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                }
            }
            else {
                queue[cntq++] = 2;
                longjmp(SCHEDULER, 1);
            }
        }
        //整個結束
        mutex = 0;
        longjmp(SCHEDULER, -2);
    }
}

void funct_3(int name)
{
    int i, j;
    if ( setjmp(now3->Environment) == 0 ) {
        kkk++;
        funct_5(5);
    }
    else {
        while(1) {
            if ( mutex == 0 || mutex == 3 ) {
                mutex = 3;
                if ( cntq != 0 && queue[0] == 3  ) { 
                    for ( int iii = 0; iii < cntq-1; iii++ ) {
                        queue[iii] = queue[iii+1];
                    }
                    cntq--;
                }
                sigset_t old;
                for ( j = 1; j <= P; j++ ) {
                    if( ((cnt3%task2)== 0) && cnt3!=P && cnt3 != 0 && flag3 == 0 ) {
                        mutex = 0;
                        flag3 = 1;
                        longjmp(SCHEDULER, 1);
                    }
                    else if( cnt3 == P ) {//整個結束
                        mutex = 0;
                        longjmp(SCHEDULER, -2);
                    }
                    flag3 = 0;
                    cnt3++;
                    for ( i = 1; i <= Q; i++ ) {
                        sleep(1);
                        //fprintf(stderr, "hi3\n");
                        arr[idx++] = '3';
                    }
                    sigset_t receive;
                    sigpending(&receive);
                    if( sigismember(&receive, SIGUSR1) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR2) ) {
                        mutex = 0;
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR3) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                }
            }
            else {
                queue[cntq++] = 3;
                longjmp(SCHEDULER, 1);
            }
        }
        //整個結束
        mutex = 0;
        longjmp(SCHEDULER, -2);
    }
}

void funct_4(int name)
{
    int i, j;
    if ( setjmp(now4->Environment) == 0 ) {
        longjmp(mainnn, 1);
    }
    else {
        while(1) {
            if ( mutex == 0 || mutex == 4 ) {
                mutex = 4;
                if ( cntq != 0 && queue[0] == 4  ) { 
                    for ( int iii = 0; iii < cntq-1; iii++ ) {
                        queue[iii] = queue[iii+1];
                    }
                    cntq--;
                }
                sigset_t old;
                for ( j = 1; j <= P; j++ ) {
                    if( ((cnt4%task2)== 0) && cnt4!=P && cnt4 != 0 && flag4 == 0) {
                        mutex = 0;
                        flag4 = 1;
                        longjmp(SCHEDULER, 1);
                    }
                    else if( cnt4 == P ) {//整個結束
                        mutex = 0;
                        longjmp(SCHEDULER, -2);
                    }
                    flag4 = 0;
                    cnt4++;
                    for ( i = 1; i <= Q; i++ ) {
                        sleep(1);
                        //fprintf(stderr, "hi4\n");
                        arr[idx++] = '4';
                    }
                    sigset_t receive;
                    sigpending(&receive);
                    if( sigismember(&receive, SIGUSR1) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR2) ) {
                        mutex = 0;
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                    if( sigismember(&receive, SIGUSR3) ) {
                        sigprocmask( SIG_SETMASK, &emptymask, &old);
                    }
                }
            }
            else {
                queue[cntq++] = 4;
                longjmp(SCHEDULER, 1);
            }
        }
        ///整個結束
        mutex = 0;
        longjmp(SCHEDULER, -2);
    }
}

void funct_5(int name)
{
    int a[10000];
    if(kkk == 0)
        funct_1(1);
    if(kkk == 1)
        funct_2(2);
    if(kkk == 2)
        funct_3(3);
    if(kkk == 3)
        funct_4(4);
    return;
}




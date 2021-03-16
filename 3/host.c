#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
int main(int argc, char** argv)
{
        int host_id = atoi(argv[1]);
        int random_key = atoi(argv[2]);
        int depth = atoi(argv[3]);
        if ( depth==0 ) { //root_host
                char i_fifo[20];
                sprintf(i_fifo, "%s%d%s", "Host", host_id, ".FIFO");
                FILE *fin, *fout;
                fin = fopen(i_fifo, "r+");
                fout = fopen("Host.FIFO", "r+");
                int roottoleft[2], roottoright[2], lefttoroot[2], righttoroot[2];
                pipe(roottoleft);
                pipe(roottoright);
                pipe(lefttoroot);
                pipe(righttoroot);
                //fork
                pid_t pid1, pid2;
                char  cone[10];
                int  ione = 1;
                sprintf(&cone[0], "%d", ione);
                pid1 = fork();
                if ( pid1==0 ) {//left
                        dup2(roottoleft[0], 0);
                        dup2(lefttoroot[1], 1);
                        execlp("./host",  "./host", argv[1],  argv[2], cone, NULL);
                }
                pid2 = fork();
                if ( pid2 == 0 ) {//right
                        dup2(roottoright[0], 0);
                        dup2(righttoroot[1], 1);
                        execlp("./host",  "./host", argv[1],  argv[2], cone, NULL);
                }
                FILE *finl = fdopen(lefttoroot[0], "r");
                FILE *finr = fdopen(righttoroot[0], "r");
                FILE *foutl = fdopen(roottoleft[1], "w");
                FILE *foutr = fdopen(roottoright[1], "w");
                assert(foutl != NULL);
                while(1) {
                        int pl_left[4], pl_right[4];
                        fflush(fin);
                        fscanf( fin, "%d%d%d%d%d%d%d%d", &pl_left[0],&pl_left[1],&pl_left[2],&pl_left[3],&pl_right[0],&pl_right[1],&pl_right[2],&pl_right[3]);
                        //從Hosti.fifo讀player_id
                        fprintf(foutl, "%d %d %d %d\n", pl_left[0], pl_left[1], pl_left[2], pl_left[3]);
                        fflush(foutl);
                        fprintf(foutr, "%d %d %d %d\n", pl_right[0], pl_right[1], pl_right[2], pl_right[3]);//給child player_id
                        fflush(foutr);
                        if(pl_left[0] == -1 ){
                                wait(NULL);
                                wait(NULL);
                                exit(0);
                        }
                        int lid, lmoney, rid, rmoney;
                        int winid, winmoney;
                        for ( int i = 0;i < 10; i++ ) {
                                //fprintf(stderr, "hi\n");
                                fflush(finl);
                                fflush(finr);
                                fscanf(finl, "%d%d", &lid, &lmoney);
                                fscanf(finr, "%d%d", &rid, &rmoney);//child給的id和money
                                if(lmoney>=rmoney) {
                                        winid = lid;
                                        winmoney = lmoney;
                                }
                                else {
                                        winid = rid;
                                        winmoney = rmoney;
                                }
                                if ( i != 9 ) {
                                        fprintf(foutl, "%d\n", winid);//給child1 final winner的id
                                        fflush(foutl);
                                        fprintf(foutr, "%d\n", winid);//給child2 final winner的id
                                        fflush(foutr);
                                }
                        }
                        int outid[20], outrank[20];
                        int inid[9];
                        for ( int i = 1; i <= 4; i++ )
                                inid[i] = pl_left[i-1];
                        for ( int i = 1; i <= 4; i++ )
                                inid[i+4] = pl_right[i-1];
                        for ( int i = 1; i <= 8; i++ ) {
                                outid[i] = inid[i];
                                outrank[i] = 2;
                        }
                        outrank[8] = 1;
                        char finalout[500];
                        sprintf(finalout, "%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n", random_key, 
                        outid[1], outrank[1], outid[2], outrank[2],outid[3], outrank[3],outid[4], outrank[4],
                        outid[5], outrank[5], outid[6], outrank[6],outid[7], outrank[7],outid[8], outrank[8]);
                        fprintf(fout, finalout, strlen(finalout));
                        fflush(fout);
                }
        }
        else if ( depth == 1 ) { //child
                int ctol[2], ctor[2], ltoc[2], rtoc[2];
                pipe(ctol);
                pipe(ctor);
                pipe(ltoc);
                pipe(rtoc);
                pid_t cpid1, cpid2;
                char ctwo[10];
                int itwo = 2;
                sprintf(ctwo, "%d", itwo);
                cpid1 = fork();
                if( cpid1 == 0 ) {
                        dup2(ctol[0], 0);
                        dup2(ltoc[1], 1);
                        close(ctol[0]);
                        close(ctor[1]);
                        close(ltoc[0]);
                        close(rtoc[1]);
                        close(ctol[0]);
                        close(ctor[1]);
                        close(ltoc[0]);
                        close(rtoc[1]);
                        execlp("./host",  "./host", argv[1],  argv[2], ctwo, NULL);
                }
                cpid2 = fork();
                if ( cpid2 == 0 ) {
                        dup2(ctor[0], 0);
                        dup2(rtoc[1], 1);
                        close(ctol[0]);
                        close(ctor[1]);
                        close(ltoc[0]);
                        close(rtoc[1]);
                        close(ctol[0]);
                        close(ctor[1]);
                        close(ltoc[0]);
                        close(rtoc[1]);
                        execlp("./host",  "./host", argv[1],  argv[2], ctwo, NULL);
                }
                FILE *finl = fdopen(ltoc[0], "r");
                FILE *finr = fdopen(rtoc[0], "r");
                FILE *foutl = fdopen(ctol[1], "w");
                FILE *foutr = fdopen(ctor[1], "w");
                while(1) {
                        int pl_left[2], pl_right[2];
                        fflush(stdin);
                        fscanf(stdin, "%d%d%d%d", &pl_left[0], &pl_left[1], &pl_right[0], &pl_right[1]);//root給的player_id
                        fprintf(foutl, "%d %d\n", pl_left[0], pl_left[1]);
                        fflush(foutl);
                        fprintf(foutr, "%d %d\n", pl_right[0], pl_right[1]);//給leaf player_id
                        fflush(foutr);
                        if(pl_left[0] == -1 ) {
                                wait(NULL);
                                wait(NULL);
                                exit(0);
                        }
                        int finalwinid;
                        for ( int i = 0; i < 10; i++ ) {
                                int lid, lmoney, rid, rmoney;
                                int winid, winmoney;
                                fflush(finl);
                                fflush(finr);
                                fscanf(finl, "%d%d", &lid, &lmoney);
                                fscanf(finr, "%d%d", &rid, &rmoney);//leaf給的id和money
                                if(lmoney>=rmoney) {
                                        winid = lid;
                                        winmoney = lmoney;
                                }
                                else {
                                        winid = rid;
                                        winmoney = rmoney;
                                }
                                fprintf(stdout, "%d %d\n", winid, winmoney);//給root winner的id和money
                                fflush(stdout);
                                if ( i != 9 ) {
                                        fflush(stdin);
                                        fscanf(stdin, "%d", &finalwinid);
                                        fprintf(foutl, "%d\n", finalwinid);//給leaf1 final winner的id
                                        fflush(foutl);
                                        fprintf(foutr, "%d\n", finalwinid);//給leaf2 final winner的id
                                        fflush(foutr);
                                }
                        }
                }
        }
        else if ( depth == 2 ) { //leaf
                while(1) {
                        int player_id[2];
                        fflush(stdin);
                        fscanf(stdin, "%d%d", &player_id[0], &player_id[1]);//child給的id
                        int leaftol[2], leaftor[2], ltoleaf[2], rtoleaf[2];
                        if( player_id[0]==-1 ) {
                                
                                exit(0);
                        }
                        pipe(leaftol);
                        pipe(leaftor);
                        pipe(ltoleaf);
                        pipe(rtoleaf);
                        pid_t lpid1, lpid2;
                        char cplayer_id[2][10];
                        sprintf(cplayer_id[0], "%d", player_id[0]);
                        sprintf(cplayer_id[1], "%d", player_id[1]);
                        lpid1 = fork();
                        if ( lpid1 == 0 ) {
                                dup2(leaftol[0], 0);
                                dup2(ltoleaf[1], 1);
                                close(leaftol[0]);
                                close(leaftor[1]);
                                close(ltoleaf[0]);
                                close(rtoleaf[1]);
                                close(leaftol[0]);
                                close(leaftor[1]);
                                close(ltoleaf[0]);
                                close(rtoleaf[1]);
                                execlp("./player", "./player", cplayer_id[0], NULL);
                        }
                        lpid2 = fork();
                        if ( lpid2 == 0 ) {
                                dup2(leaftor[0], 0);
                                dup2(rtoleaf[1], 1);
                                close(leaftol[0]);
                                close(leaftor[1]);
                                close(ltoleaf[0]);
                                close(rtoleaf[1]);
                                close(leaftol[0]);
                                close(leaftor[1]);
                                close(ltoleaf[0]);
                                close(rtoleaf[1]);
                                execlp("./player", "./player", cplayer_id[1], NULL);
                        }
                        FILE *finl = fdopen(ltoleaf[0], "r");
                        FILE *finr = fdopen(rtoleaf[0], "r");
                        FILE *foutl = fdopen(leaftol[1],"w");
                        FILE *foutr = fdopen(leaftor[1], "w");
                        int lid, lmoney, rid, rmoney;
                        int winid, winmoney;
                        int finalwinid;
                        for ( int i = 0; i < 10; i++ ) {
                                fflush(finl);
                                fscanf(finl, "%d%d", &lid, &lmoney);
                                fflush(finr);
                                fscanf(finr, "%d%d", &rid, &rmoney);//player給的id和money
                                if(lmoney>=rmoney) {
                                        winid = lid;
                                        winmoney = lmoney;
                                }
                                else {
                                        winid = rid;
                                        winmoney = rmoney;
                                }
                                fprintf(stdout, "%d %d\n", winid, winmoney);//給child winner的id和money
                                fflush(stdout);
                                if( i != 9 ) {
                                        fflush(stdin);
                                        fscanf(stdin, "%d", &finalwinid);
                                        fprintf(foutl, "%d\n", finalwinid);//給leaf1 final winner的id
                                        fflush(foutl);
                                        fprintf(foutr, "%d\n", finalwinid);//給leaf2 final winner的id
                                        fflush(foutr);
                                }
                        }
                        wait(0);
                        wait(0);
                }
        }
        return 0;
}
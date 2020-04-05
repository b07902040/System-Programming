#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

char inputarray[3010][30];
int cnt_input;
int map[65536];
typedef struct play {
        int t;
        int score;
}Player;
int compare(const void *a, const void *b)
{
      Player one = *(Player *)a;
      Player two = *(Player *)b;
      if(one.score < two.score) return 1;
      else if (one.score > two.score ) return -1;       
      else return 0;                          
}
int  C(int k)
{
        int ret = 1;
        for ( int i = 9; i <= k; i++ )
                ret *= i;
        for ( int i = 9; i <= k; i++ ) 
                ret /= (i-8);
        return ret;
}
void  Combination(int k)
{ 
        for ( int a = 1; a <= k-7; a++ )
        for ( int b = a+1; b <= k-6; b++ )
        for ( int c = b+1; c <= k-5; c++ )
        for ( int d = c+1; d <= k-4; d++ )
        for ( int e = d+1; e <= k-3; e++ )
        for ( int f = e+1; f <= k-2; f++ )
        for ( int g = f+1; g <= k-1; g++ )
        for ( int h = g+1; h <= k; h++ )
        sprintf(&inputarray[cnt_input++][0], "%d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h);
        return;
}
int main(int argc, char** argv)
{
        cnt_input = 1;
        int host_num = atoi(argv[1]);
        int player_num = atoi(argv[2]);
        Player player[player_num+1];
        int rank[player_num+1];
        for ( int i = 0; i <= player_num; i++ ) {
                player[i].score = 0;
                player[i].t = i;
        }
        player[0].score = -1;
        int combinum = C(player_num);
        Combination(player_num);
        //start
        mkfifo("Host.FIFO", 0777);
        FILE *write_fp[host_num+1];
        char build_fifo[host_num+1][20];
        for ( int i = 1; i <= host_num; i++ ) {
                sprintf(build_fifo[i], "%s%d%s", "Host", i, ".FIFO");
                unlink(build_fifo[i]);
                mkfifo(build_fifo[i], 0777);
        }
        for ( int i = 1; i <= host_num; i++ ) {
                int  ran  = (rand()%65536);
                map[ran] = i;
                pid_t pid = fork();
                if( pid == 0 ) {
                        srand(time(NULL));
                        char  random_key[10];
                        char  host_id[10];
                        char  zero[10];
                        int  izero = 0;
                        sprintf(&host_id[0], "%d", i );
                        sprintf(&random_key[0], "%d", ran);
                        sprintf(&zero[0], "%d", izero);
                        execlp("./host",  "./host", host_id,  random_key, zero, NULL);
                }
        }
        for ( int i = 1; i <= host_num; i++ ) {
                write_fp[i] = fopen(build_fifo[i], "w");
        }
        FILE *read_fp;
        read_fp = fopen("Host.FIFO\0", "r");
        int j = 1, get = 1;
        for ( int i = 1; i <= host_num; i++ ) {
                fprintf(write_fp[i], &inputarray[j][0], strlen(inputarray[j]));
                fflush(write_fp[i]);
                j++;
                if(i>=combinum) break;
        }
        int r_random;
        int r_plid[8];
        int r_plrank[8];
        while ( j <= combinum ) {
                fscanf(read_fp, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", 
                &r_random, &r_plid[0], &r_plrank[0], &r_plid[1], &r_plrank[1], &r_plid[2], &r_plrank[2], &r_plid[3], &r_plrank[3]
                                     , &r_plid[4], &r_plrank[4], &r_plid[5], &r_plrank[5], &r_plid[6], &r_plrank[6], &r_plid[7], &r_plrank[7] );
                for ( int i = 0; i < 8; i++ ) {
                        player[r_plid[i]].score += (8-r_plrank[i]);
                }
                get++;
                fprintf(write_fp[map[r_random]], &inputarray[j][0], strlen(inputarray[j]));
                fflush(write_fp[map[r_random]]);
                j++;
        }
        while ( get <= combinum ) {
                fflush(read_fp);
                fscanf(read_fp, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", 
                &r_random, &r_plid[0], &r_plrank[0], &r_plid[1], &r_plrank[1], &r_plid[2], &r_plrank[2], &r_plid[3], &r_plrank[3]
             , &r_plid[4], &r_plrank[4], &r_plid[5], &r_plrank[5], &r_plid[6], &r_plrank[6], &r_plid[7], &r_plrank[7] );
                for ( int i = 0; i < 8; i++ ) {
                        player[r_plid[i]].score += (8-r_plrank[i]);
                }
                get++;
        }
        //print rank
        qsort(player, player_num+1, sizeof(Player), compare);
        int rankk = 1;
        for ( int i = 0; i < player_num; i++ ) {
                if ( i != 0 ) {
                        if ( player[i-1].score != player[i].score ) {
                                rankk++;
                        }
                }
                rank[player[i].t] = rankk;
        }
        for ( int i = 1; i <= player_num; i++ ) {
                printf("%d %d\n", i, rank[i]);
        }
        //end
        char end[30];
        sprintf(&end[0], "%s", "-1 -1 -1 -1 -1 -1 -1 -1\n");
        for ( int i = 1; i <= host_num; i++ ) 
                fprintf(write_fp[i], &end[0], strlen(inputarray[i-1]));
        fclose(read_fp);
        for ( int i  = 1; i <= host_num; i++ )
                fclose(write_fp[i]);
        for ( int i = 1; i <= host_num; i++ ) 
                wait(NULL);
        unlink("Host.FIFO");
        for ( int i = 1; i <= host_num; i++ ) {
                unlink(build_fifo[i]);
        }
        return 0;
}
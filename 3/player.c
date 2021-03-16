#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>       

int main(int argc, char** argv)
{
        int winner_id;
        int player_id = atoi(argv[1]);
        char buf[100];
        sprintf(buf, "%d %d\n", player_id, player_id*100);
        for ( int i = 0; i < 9; i++ ) {
                printf("%d %d\n", player_id, player_id*100);
                fflush(stdout);
                scanf("%d", &winner_id);
        }
        printf("%d %d\n", player_id, player_id*100);
        fflush(stdout);
        return 0;
}
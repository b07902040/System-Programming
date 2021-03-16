#define _GNU_SOURCE
#include <unistd.h> 
#include <sched.h> 
#include <sys/syscall.h>
#include <sys/wait.h>
#include <bits/stdc++.h>
#define MAXP 30
#define low_priority 40
#define high_priority 80

using namespace std;
//typedef enum __bool { false = 0, true = 1, } bool;

typedef struct pppp{
    char name[32];
    int readyt, exet;
    pid_t pid;
}Process;

static int cases, my_time, POLICY, in_cpu, cnt, pre_time, pre_in_cpu;
Process P[MAXP];
queue<int> RR;

void fork_process(int idx) {
    int pid = fork();
    if ( pid < 0 ) {
    	fprintf(stderr, "Fork error\n");
    }
    if ( pid == 0 ) { //child
    	int child_pid = getpid();
        unsigned long int startt, endt, startnt, endnt;
        syscall(334, true, &child_pid, &startt, &startnt, &endt, &endnt);//sys_get_time
        //printf("%d\n", P[idx].exet);
        for ( int i = 0; i < P[idx].exet; i++ ) {
            volatile unsigned long j;		
	        for (j = 0; j < 1000000UL; j++);
        }
        syscall(334, false, &child_pid, &startt, &startnt, &endt, &endnt);//sys_get_time
        fprintf(stdout, "%s %d\n", P[idx].name, child_pid);
    	fflush(stdout);
		exit(0);
    }
    //parent
    P[idx].pid = pid;
    cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);
	CPU_SET(1, &cpu_mask);
	if (sched_setaffinity(pid, sizeof(cpu_mask), &cpu_mask) < 0) {
		fprintf(stderr, "sched_setaffinity error\n");
		exit(1);
	}
    return;
}
pid_t Scheduler() { // FIFO:0, RR:1, SJF:2, PSJF:3 
	if ( in_cpu != -1 && (POLICY == 0 || POLICY == 2) )
		return in_cpu;
    pid_t select = -1;
    int mint = INT_MAX;
    if ( in_cpu == -1 && (POLICY == 0 || POLICY == 1) ) {
        if ( RR.empty() ) {
            return -1;
        }
        else {
            select = RR.front();
            RR.pop();
        }
    }
    else if ( POLICY == 1 ) {
		if ( (my_time-pre_time)%500 == 0 ) {
			RR.push(in_cpu);
            select = RR.front();
            RR.pop();
		}
		else
			return in_cpu;
	}
	else if ( POLICY == 2 || POLICY == 3 ) {
		for ( int i = 0; i < cases; i++ ) {
			if ( P[i].pid == -1 || P[i].exet <= 0 )
				continue;
            if ( P[i].exet < mint ) {
                mint = P[i].exet;
                select = i;
            }
		}
	}
	return select;
}
int main()
{
    //input
    char strPOLICY[10];
    scanf("%s", strPOLICY);

    if ( strPOLICY[0] == 'F' )
        POLICY = 0;
    else if ( strPOLICY[0] == 'R' )
        POLICY = 1;
    else if ( strPOLICY[0] == 'S' )
        POLICY = 2;
    else if ( strPOLICY[0] == 'P' )
        POLICY = 3;
    else 
        fprintf(stderr, "POLICY error\n");
    scanf("%d", &cases);
    char in_name[MAXP][32];

    int in_readyt[MAXP], in_exet[MAXP];
    for ( int i = 0; i < cases; i++ ) {
        scanf("%s %d %d\n", in_name[i], &in_readyt[i], &in_exet[i]);
    }
   	//sort
    for ( int i = 0; i < cases-1; i++ ) {
        for ( int j = 0; j < cases-i-1; j++ ) {
            if ( in_readyt[j] > in_readyt[j+1] ) {
                int tempr = in_readyt[j];
                int tempe = in_exet[j];
                char tempn[32];
                strcpy(tempn, in_name[j]);
                in_readyt[j] = in_readyt[j+1];
                in_readyt[j+1] = tempr;
                in_exet[j] = in_exet[j+1];
                in_exet[j+1] = tempe;
                strcpy(in_name[j], in_name[j+1]);
                strcpy(in_name[j+1], tempn);
            }
        }
    }
    for ( int i = 0; i < cases; i++ ) {
        P[i].pid = -1;
        strcpy(P[i].name, in_name[i]);
        P[i].readyt = in_readyt[i];
        P[i].exet = in_exet[i];
    }
    
    /*for ( int i = 0; i < cases; i++ ) {
    	printf("%s %d %d %d\n", P[i].name, P[i].readyt, P[i].exet, P[i].pid);
    }*/
    
    // set parent zero cpu
    cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);
	CPU_SET(0, &cpu_mask);
	if (sched_setaffinity(getpid(), sizeof(cpu_mask), &cpu_mask) < 0) {
		fprintf(stderr, "sched_setaffinity error\n");
		exit(1);
	}
    //start
    my_time = 0;
    in_cpu = -1;
    pre_in_cpu = -1;
    pre_time = 0;
    int ready_cnt = 0;
    bool active = true;
    while(cnt < cases) {
        if (in_cpu != -1 && P[in_cpu].exet == 0) {
        	//printf("hi\n");
			waitpid(P[in_cpu].pid, NULL, 0);
			P[in_cpu].pid = -1;
			//printf("%s end at %d\n", P[in_cpu].name, my_time);
			pre_in_cpu = in_cpu;
			in_cpu = -1;
			cnt++;
		}
        if (active) {
            for ( int i = ready_cnt; i < ready_cnt+1; i++ ) {
                if ( P[i].readyt == my_time ) {
                	//printf("ready %d at %d\n", i, my_time);
                    fork_process(i);
                    struct sched_param param;
                    param.sched_priority = low_priority;
                    sched_setscheduler(P[i].pid, SCHED_FIFO, &param);
                    RR.push(i);
                    ready_cnt++;
                    if ( ready_cnt == cases ) {
                        active = false;
                        break;
                    }
                }
            }
        }
        // Select next process
		pid_t next = Scheduler();
		if (next != -1 && next != in_cpu) {
			// Context Switch
			//printf("run %s at %d\n", P[next].name, my_time); 
			struct sched_param param;
            param.sched_priority = high_priority;
            sched_setscheduler(P[next].pid, SCHED_FIFO, &param);
            param.sched_priority = low_priority;
            sched_setscheduler(P[in_cpu].pid, SCHED_FIFO, &param);
			in_cpu = next;
			pre_time = my_time;
		}
        if ( in_cpu != -1 ) {
        	//printf("%d\n", P[0].exet);
            P[in_cpu].exet--;
        }
        volatile unsigned long j;		
	    for (j = 0; j < 1000000UL; j++);
        my_time++;
    /*
        if ( (my_time%100) == 0 ) {
        	//printf("%d\n", in_cpu);
        	//printf("time:%d\n", my_time/500);
        	//printf("%d\n", P[in_cpu].exet);
        }
        */
    }
    //printf("main:%d\n", in_cpu);
    return 0;
}
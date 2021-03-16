#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h:>
#include <linux/linkage.h>

asmlinkage long my_gettime(
    bool front, pid_t pid, 
    unsigned long int *startt,
    unsigned long int *startnt,
    unsigned long int *endt,
    unsigned long int *endnt) {
    struct timespec T;
    getnstimeofday(&T);
    if (front) {
        *startt = T.tv_sec;
        *startnt = T.tv_nsec;
    } 
    else {
        *endt = t.tv_sec;
        *endnt = t.tv_nsec;
        printk("[project1] %d %lu.%09lu %lu.%09lu\n",*pid, *startt, *startnt, *endt, *endnt);
    }
    return 0;
}
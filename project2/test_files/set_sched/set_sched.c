#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h> 
#include <sched.h> 

#define SCHED_NORMAL        0
#define SCHED_FIFO      1
#define SCHED_RR        2
#define SCHED_BATCH     3
#define SCHED_IDLE      5
#define SCHED_WRR       6
static void change_scheduler()
{
    int ret, tmp;
    struct sched_param param;
    pid_t pid;
    int policy,oldpolicy;
    printf("Please input the process id (PID) you want to modify: ");
    scanf("%d", &tmp);
    pid = tmp;
    // struct task_struct *p = find_task_by_vpid(pid);
    if (!pid) {
        perror("This PID is invalid. Exiting...");
        exit(-1);
    }

    printf("Please input the choice of Scheduling algorithms: (0-NORMAL, 1-FIFO, 2-RR, 6-WRR):\n");
    scanf("%d", &policy);
   // printf("%d",policy);
    if(policy != 0 && policy != 1 && policy!= 2&& policy!=6)
    {
        perror("This schedule policy is wrong. Exiting...");
        exit(-1);
    }

    printf("This process's priority is: ");
    scanf("%d",&tmp);
    if(policy!=6)
        param.sched_priority = tmp;
    else
        param.sched_priority = 0;

    printf("Changing Scheduler for PID %d\n", pid);


    ret = sched_setscheduler(pid, policy, &param);
    if (ret < 0) {
        perror("Changing scheduler failed. Exiting...");
        exit(-1);
    }

}

int main() {
    change_scheduler();
    printf("Switch successed!\n");
}

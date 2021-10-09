#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<string.h>
// 
  
int main(){
    pid_t child_pid = fork();  
    // fork : if father,return UID; If son,return 0 
    if(child_pid < 0){
        printf("Fork failure!\n");
        return -1;
    }
    else if(child_pid == 0){
        pid_t pid = getpid();
        printf("519021910343 Child : %d.\n",pid);
        execl("/data/misc/ptreeARM","ptreeARM",NULL);
    }
    else if(child_pid>0){

        pid_t pid = getpid();
        printf("519021910343 Parent : %d.\n",pid);
    }

    return 0;
}
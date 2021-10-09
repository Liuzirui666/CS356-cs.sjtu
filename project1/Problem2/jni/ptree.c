#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<string.h>
// 
#define __NR_pstreecall 356

struct prinfo{
	pid_t parent_pid;	//process id of parent
	pid_t pid;		//process id
	pid_t first_child_pid;	//pid of youngest child
	
	pid_t next_sibling_pid;	//pid of older sibling
	long state;		//current state of process
	long uid;		//user id of process owner
	char comm[64];		//name of program executed
	int level;      // used to calculate the number of tabs needed in problem2
};


void Print_Tree(struct prinfo *buf,int *nr)
{
	int i,j;	
	//print the tree
	printf("There are %d processes.\n",*nr);
	for(i = 0;i < *nr;++i){
		for(j = 0;j < buf[i].level;++j)
			printf("\t");
		printf("%s,%d,%ld,%d,%d,%d,%d\n",buf[i].comm,buf[i].pid,buf[i].state,buf[i].parent_pid,
		buf[i].first_child_pid,buf[i].next_sibling_pid,buf[i].uid);
	}
}

int main(){
	struct prinfo *buf = malloc(1050*sizeof(struct prinfo));
	int *nr = malloc(sizeof(int));
	if(buf == NULL || nr == NULL){
		printf("Allocation to memory failed!\n");
		exit(-1);
	}

	//  the first parameter is for system call
	// other parameters are those you want to send in
	syscall(__NR_pstreecall,buf,nr);

	Print_Tree(buf,nr);

	free(buf);
	free(nr);
	
	return 0;
}

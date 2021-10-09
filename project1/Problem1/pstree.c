#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/unistd.h>
#include<linux/list.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/syscalls.h>
#include<linux/gfp.h>
// 
MODULE_LICENSE("Dual BSD/GPL");
#define __NR_pstreecall 356

struct prinfo{
	pid_t parent_pid;	//process id of parent
	pid_t pid;		//process id
	pid_t first_child_pid;	//pid of youngest child
	
	pid_t next_sibling_pid;	//pid of older sibling
	long state;		//current state of process
	long uid;		//user id of process owner
	char comm[64];		//name of program executed
	int level; // used to calculate the number of tabs needed in problem2
};
static int (*oldcall)(void);
//used to extract information from struct task_struct to prinfo   
void extract(struct task_struct *ts,struct prinfo *pf,int level){
	pf->parent_pid = (ts->parent) ?ts->parent->pid :0;
	pf->pid = ts->pid;
	pf->state = ts->state;
	pf->uid = ts->cred->uid;
	pf->level=level;
	get_task_comm(pf->comm,ts);
	//  /home/lzr/os/ndk/android-ndk-linux/android-kernel/kernel/goldfish/include/linux/ sched.h, cred.h
	//
	//sched.h file line 1377, list.h file line 186 & line 345
	//cred.h file line 125
	if(list_empty(&(ts->children))){
		pf->first_child_pid = 0;
	}
	else{
		//list_entry is used to find which task_struct  line_head pointer is in
		pf->first_child_pid = list_entry((&ts->children)->next,struct task_struct,sibling)->pid;
	}

	if(list_empty(&(ts->sibling))){
		pf->next_sibling_pid = 0;
	}
	else{
		//The next can be sibling or parent, if parent , 
		//(&ts->sibling)->next points to its parent's children
		pid_t uncertain_pid = list_entry((&ts->sibling)->next,struct task_struct,children)->pid;
		pid_t next_sibling_pid = list_entry((&ts->sibling)->next,struct task_struct,sibling)->pid;
		if(next_sibling_pid == pf->parent_pid || uncertain_pid == pf->parent_pid){
			pf->next_sibling_pid = 0;
		}
		else{
			pf->next_sibling_pid = next_sibling_pid;
		}
	}

}

//DFS 
void DFS(struct task_struct *ts,struct prinfo *buf,int *nr,int level)
{
	struct task_struct *tmp;
	struct list_head *now =  (&ts->children)->next;

	extract(ts,&buf[*nr],level);
	*nr = *nr + 1;

	list_for_each(now, &(ts->children)){ 
        tmp = list_entry(now, struct task_struct, sibling);
        DFS(tmp, buf, nr, level+1);
    }

}

static int pstree(struct prinfo *buf,int *nr)
{
	
	//  kcalloc(size_t n, size_t size, gfp_t flags) : allocate array memory and set to zero
	//  kzalloc(size_t size, gfp_t flags) : allocate memory and set to zero
	struct prinfo *process_buf = kcalloc(1010, sizeof(struct prinfo),GFP_KERNEL);
	int *process_nr = kzalloc(sizeof(int),GFP_KERNEL);
	if(process_buf == NULL || process_nr == NULL){
		printk("Allocation to memory failed!\n");
		return -1;
	}
	*process_nr = 0;
	
	read_lock(&tasklist_lock);
	DFS(&init_task,process_buf,process_nr,0);
	read_unlock(&tasklist_lock);

	//  unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
	if(copy_to_user(buf,process_buf,1010*sizeof(struct prinfo))){
		printk("Copy from kernel to user failed!\n");
		return -1;
	}
	if(copy_to_user(nr,process_nr,sizeof(int))){
		printk("Copy from kernel to user failed!\n");
		return -1;
	}

	// set memory free
	kfree(process_nr);
	kfree(process_buf);

	return 0;
}

static int addsyscall_init(void)
{
	long *syscall = (long*)0xc000d8c4;
	oldcall = (int(*)(void))(syscall[__NR_pstreecall]);
	syscall[__NR_pstreecall] = (unsigned long )pstree;
	printk(KERN_INFO "module load!\n");
	return 0;
}

static void addsyscall_exit(void)
{
	long *syscall = (long*)0xc000d8c4;
	syscall[__NR_pstreecall] = (unsigned long )oldcall;
	printk(KERN_INFO "module exit\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);

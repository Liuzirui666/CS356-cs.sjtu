#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
// 
int cur_customer;
int max_cook, max_cashier, max_customer, max_rack;
int *cook_id;
int *cashier_id;
int *customer_id;

sem_t sem_killer, sem_new_order, sem_not_empty, sem_not_full, sem_cashier_ready, mutex;
pthread_t *thread_cook;
pthread_t *thread_cashier;
pthread_t *thread_customer;
pthread_t *thread_killer;

int Atoi(char *str) {
    int i = 0;
    int result = 0;
    while(str[i]<='9' && str[i] >= '0'){
        result = result*10 + (str[i] - '0');
        i++;
    }
    return result;
}

void new_exit(){ 
    pthread_exit(NULL);
}

void *killer(){
    sem_wait(&sem_killer);
   
    int i;
    for(i = 0;i < max_cook;++i){
        pthread_kill(thread_cook[i],SIGQUIT);
    }
    for(i = 0;i < max_cashier;++i){
        pthread_kill(thread_cashier[i],SIGQUIT);
    }

    return;
}

void *cook(void *argv){
    int num = *(int*)argv;
    signal(SIGQUIT,new_exit); 

    while(1){
        sem_wait(&sem_not_full);
        //cook makes hamburgers and places them on the rack
        sleep(1);
        printf("Cook:[%d] makes a burger.\n", num);
        sem_post(&sem_not_empty);
    }

    return;
}

void *cashier(void *argv){
    int num = *(int*)argv;
    signal(SIGQUIT,new_exit); 
    
    while(1){
        sem_wait(&sem_new_order);
        //cashier accept an order
        printf("Cashier:[%d] accepts an order.\n",num);
        sem_wait(&sem_not_empty);
        //cashier get a burger from the rack
        sleep(1);
        printf("Cashier:[%d] takes a burger to a customer.\n",num);

        sem_wait(&mutex);
	    cur_customer--;	
        if(cur_customer==0){
            sem_post(&sem_killer);
        }
        sem_post(&mutex);

        sem_post(&sem_cashier_ready);
        sem_post(&sem_not_full);
        
    }

    return;
}

void *customer(void *argv){
    int num = *(int*)argv;
    sleep(rand()%num);
    printf("Customer:[%d] comes.\n",num);
    //customer approaches a cashier and order food
    sem_wait(&sem_cashier_ready);
    sem_post(&sem_new_order);
    //** sem_wait(&sem_cashier_ready);
    //cusmoter orders a hamburger

    return;
}

int main(int argc, char **argv){
    if (argc != 5)
    {
        printf("Error! Not correct parameters!");
        exit(-1);
    }

    max_cook = Atoi(argv[1]);
    max_cashier = Atoi(argv[2]);
    max_customer = cur_customer = Atoi(argv[3]);
    max_rack = Atoi(argv[4]);

    if(max_cook<1 || max_cashier<1 || max_customer<1 || max_rack<1){
        printf("The number of cook,cashier,customer,rack can not be satiefied!");
        exit(-1);
    }

    sem_init(&sem_killer,0,0);
    sem_init(&sem_new_order,0,0);
    sem_init(&sem_not_empty,0,0);
    sem_init(&sem_not_full,0,max_rack);
    sem_init(&mutex,0,1);
    sem_init(&sem_cashier_ready,0,max_cashier);  // should be set right

    cook_id = malloc(max_cook * sizeof(int));
    cashier_id = malloc(max_cashier * sizeof(int));
    customer_id = malloc(max_customer * sizeof(int));    

    thread_cook = malloc(max_cook * sizeof(pthread_t));
    thread_cashier = malloc(max_cashier * sizeof(pthread_t));
    thread_customer = malloc(max_customer * sizeof(pthread_t));
    thread_killer = malloc(sizeof(pthread_t));
    pthread_create(thread_killer,NULL,killer,NULL);

    printf("Cooks:[%d], Cashiers:[%d], Customers:[%d]\n",max_cook,max_cashier,max_customer);
    printf("Begin run.\n");
    
    int i;
    for(i = 0;i < max_cook;++i){
        cook_id[i] = i + 1;
        pthread_create(thread_cook + i,NULL,cook,cook_id + i);
    }
    for(i = 0;i < max_cashier;++i){
        cashier_id[i] = i + 1;
        pthread_create(thread_cashier + i,NULL,cashier,cashier_id + i);
    }
    for(i = 0;i < max_customer;++i){
        customer_id[i] = i + 1;
        pthread_create(thread_customer + i,NULL,customer,customer_id + i);
    }

    pthread_join(*thread_killer,NULL);
    for(i = 0;i < max_cook;++i){
        pthread_join(thread_cook[i],NULL);
    }
    for(i = 0;i < max_cashier;++i){
        pthread_join(thread_cashier[i],NULL);
    }
    for(i = 0;i < max_customer;++i){
        pthread_join(thread_customer[i],NULL);
    }

    printf("End run.\n");

    free(cook_id);
    free(cashier_id);
    free(customer_id);

    free(thread_cook);
    free(thread_cashier);
    free(thread_customer);
    free(thread_killer);

    sem_destroy(&sem_killer);
    sem_destroy(&sem_new_order);
    sem_destroy(&sem_not_empty);
    sem_destroy(&sem_not_full);
    sem_destroy(&sem_cashier_ready);
    sem_destroy(&mutex);

    return 0;
}

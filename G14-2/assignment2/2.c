#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include "../assignment1/mysem.h"

int terminate = 0, number = 0; 

mysem_t  *t_terminate,mutex,full,empty;

void set_sem_value(mysem_t *mutex, int n) {
    mutex->init = 0;
    mutex->semid = semget(IPC_PRIVATE,1,S_IRWXU);
    mysem_init(mutex, n);
}

void isPrime(int n) {
    int i;
    
    for (i = 2; i <= n - 1; i++) {
        if (n%i == 0) {
            printf("%d 0\n", n);
            return;
        }
    }
    printf("%d 1\n", n);
	return;
}

void *foo(void *arg){
    int i = *(int *)arg, job=0;
	
	while(terminate==0){
		mysem_down(&full);
		mysem_down(&mutex);
		job=number;
		mysem_up(&mutex);
		mysem_up(&empty);	

		if(job!=-1) 
			isPrime(job);
	}

	printf("THREAD %d BREAK LOOP\n",i);	
	mysem_up(&full);
	mysem_up(&t_terminate[i]);
	free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    int workers, i, input=0, *temp;
 
    if(argc != 2) {
        printf("Give an argument\n");
        return 1;
    }
    workers = atoi(argv[1]);
    pthread_t th[workers];

	t_terminate = malloc(workers*sizeof(mysem_t));

	set_sem_value(&mutex, 1);
	set_sem_value(&empty, 1);
	set_sem_value(&full, 0);
    for(i = 0; i < workers; i++) {
		set_sem_value(&t_terminate[i], 0);
    }
    
    for(i = 0; i < workers; i++) {
		temp = malloc(sizeof(int));
        *temp = i;
        pthread_create(&th[i], NULL, foo, temp);   
    }

    while(input!=-1) {
		scanf("%d", &input);
		mysem_down(&empty);
		mysem_down(&mutex);
		number = input;
		mysem_up(&mutex);
		mysem_up(&full);
    }

	
	
	terminate = 1;
    mysem_up(&full);
	mysem_up(&empty);
	for(i = 0; i < workers; i++){
		mysem_down(&t_terminate[i]);
        mysem_destroy(&t_terminate[i]);
    }  
    
    
    mysem_destroy(&mutex);
    mysem_destroy(&full);
    mysem_destroy(&empty);
    
    free(t_terminate);
    
    return 0;
}

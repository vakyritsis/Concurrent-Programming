#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include "../assignment1/mysem.h"
#define N 100

struct mystruct {
    int i;
    int direction;
};


int bridge_size, bridge_direction=-1, bridge_cars=0, waiters[2]={0,0},crosses=0;
mysem_t mutex, bridge_sem[2], *join;

void *car(void* arg){
    struct mystruct *temp = (struct mystruct *)arg;
    int car_direction =  temp->direction;
    int i = temp->i;
        
    mysem_down(&mutex);
    
    // wait on bridge when u can't enter
	if((bridge_direction == car_direction || bridge_direction==-1) && bridge_cars < bridge_size && 
	(bridge_cars+crosses < 2*bridge_size || waiters[1-car_direction]==0) && waiters[car_direction]==0){
		bridge_direction = car_direction;
		bridge_cars++;
		mysem_up(&mutex);
	}
	else{
		waiters[car_direction]++;
		mysem_up(&mutex);
		mysem_down(&bridge_sem[car_direction]);
		waiters[car_direction]--;
		bridge_cars++;
		bridge_direction = car_direction;
		if(waiters[car_direction]>0 && bridge_cars<bridge_size && 
		(bridge_cars+crosses < 2*bridge_size || waiters[1-car_direction]==0)){
			mysem_up(&bridge_sem[car_direction]);	
		}
		else{
			mysem_up(&mutex);	
		}
	}
	
    //drive on bridge
	printf("Bridge cars :%d\n",bridge_cars);
	if(car_direction == 0)
    	printf("\x1B[31mgoing in bridge %d dir:%d\n", i, car_direction);
	else
		printf("\x1B[34mgoing in bridge %d dir:%d\n", i, car_direction);
	printf("\x1B[37m");
    sleep(1);
    //exit bridge

    mysem_down(&mutex);
	crosses++;
    bridge_cars--;
    if(waiters[bridge_direction] != 0 && (bridge_cars+crosses<2*bridge_size || waiters[1-car_direction]==0)){
        mysem_up(&bridge_sem[car_direction]);
    }
    else if (bridge_cars==0 && waiters[1-car_direction]!=0 &&
	(waiters[car_direction]==0 || waiters[1-car_direction]+crosses >= 2*bridge_size ) ){
		bridge_direction = 1-car_direction;
		crosses=0;
        mysem_up(&bridge_sem[1-car_direction]);
    }
	else if(bridge_cars==0 && waiters[1-car_direction]==0 && waiters[car_direction]==0){
		car_direction=-1;
		crosses =0;	
		mysem_up(&mutex);
	}
	else{
		mysem_up(&mutex);
	}
	printf("exit %d\n", i);
    
    mysem_up(&join[i]);
    
    free(arg);
    return NULL;
}



void set_sem_value(mysem_t *mutex, int n) {
    mutex->init = 0;
    mutex->semid = semget(IPC_PRIVATE,1,S_IRWXU);
    mysem_init(mutex, n);
}


int main(int argc, char* argv[]){
    int  i, car_direction, numberofcars, delay, sum = 0;
    pthread_t th[N];
    struct mystruct *temp;
    
    if(argc != 2){
        printf("Invalid argument\n");
        return 0;
    }
	bridge_size = atoi(argv[1]);
    
    join = malloc(sizeof(mysem_t) * N);
    for(i = 0; i < N; i++)
        set_sem_value(&join[i], 0);
    
    set_sem_value(&mutex, 1);
    set_sem_value(&bridge_sem[0], 0);
    set_sem_value(&bridge_sem[1], 0);
    i = 0;
	while(1){
		scanf("%d %d %d", &car_direction, &numberofcars, &delay);
		if(car_direction == -1)
			break;
        sum += numberofcars;
		sleep(delay);
		for(; i < sum; i++){
            temp = malloc(sizeof(struct mystruct));
            temp->direction = car_direction;
            temp->i = i;
            pthread_create(&th[i], NULL, car, temp);
		}
	}
    for(i = 0; i < sum ; i++) {
        mysem_down(&join[i]);
        mysem_destroy(&join[i]);
	}
    
    free(join);
    mysem_destroy(&mutex);
    mysem_destroy(&bridge_sem[0]);
    mysem_destroy(&bridge_sem[1]);

    return 0;
}

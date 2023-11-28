#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define N 100

struct mystruct {
    int i;
    int direction;
};

pthread_mutex_t mutex;
pthread_cond_t q[2];
int capacity, cars  = 0, bridge_direction = -1, waiter[2] = {0,0}, crosses = 0;

void* car(void* arg){
	struct mystruct *temp = (struct mystruct *)arg;
    int direction =  temp->direction;
    int i = temp->i, j;

	//enter bridge
	pthread_mutex_lock(&mutex);
	while(cars == capacity || (cars > 0 && direction != bridge_direction) || 
	(crosses+cars >= 2*capacity && waiter[1-direction] != 0)){
		waiter[direction]++;
		pthread_cond_wait(&q[direction], &mutex);
		waiter[direction]--;
	}
	cars++;
	bridge_direction = direction;
	/*
	if(waiter[direction]>0 && cars<capacity && 
	(cars+crosses < 2*capacity || waiter[1-direction]==0)){
		pthread_cond_signal(&q[direction]);	
	}
	*/
	pthread_mutex_unlock(&mutex);

	printf("Bridge cars :%d\n",cars);
	if(direction == 0)
    	printf("\x1B[31mgoing in bridge %d dir:%d\n",i, direction);
	else
		printf("\x1B[36mgoing in bridge %d dir:%d\n",i, direction);
	printf("\x1B[37m");
	sleep(1);

	//exit bridge
	pthread_mutex_lock(&mutex);
	crosses++;
    cars--;
    
	if(waiter[direction] > 0  && ( cars+crosses < 2*capacity || waiter[1-direction] == 0)){
		pthread_cond_signal(&q[direction]);
	}
	else if(cars == 0 && waiter[1-direction] > 0 &&
	(waiter[direction]==0 || waiter[1-direction]+crosses >= 2*capacity ) ){
		bridge_direction = 1 - direction;
		crosses = 0;
		for(j=0; j<capacity;j++)
			pthread_cond_signal(&q[1-direction]);
	}
	else if(cars == 0 && waiter[1-direction] == 0 && waiter[direction] == 0){
		bridge_direction = -1;
		crosses = 0;
	}
	pthread_mutex_unlock(&mutex);
	printf("exit %d\n",i);
	free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    int  i = 0, car_direction, numberofcars, delay, sum = 0;
    pthread_t th[N];
	struct mystruct *temp;
    
    if(argc != 2) {
        printf("Give an argument\n");
        return 1;
    }
    capacity = atoi(argv[1]);
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&q[0], NULL);
    pthread_cond_init(&q[1], NULL);

    while(1) {
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
	for(i = 0; i < sum;i++)
		pthread_join(th[i], NULL);
	
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&q[0]);
    pthread_cond_destroy(&q[1]);

    return 0;
}

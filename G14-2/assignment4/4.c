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

int car_size, ride = 0;
double car_ride =10;
int boarded, unboarded;
mysem_t board, depart, unboard, arrive, mutex1, mutex2;
mysem_t join_car;

void* rollercoaster(){
	while(ride < car_ride){
		sleep(1); //load passengers
		printf("Loading\n");
		mysem_up(&board);
		printf("Waitng to depart\n");
		mysem_down(&depart);
		
		printf("Ready to depart\n");
		sleep(2); //run

		sleep(1); //unload passenger
		printf("Car arrived\n");
		mysem_up(&arrive);
		printf("Unloading\n");
		mysem_down(&unboard);
		printf("ENT OF RIDE :%d\n",ride);
		ride++;
	}
	mysem_up(&join_car);
	return NULL;
}

void* passenger(){
	mysem_down(&board);

	mysem_down(&mutex1);
	boarded++;
	printf("boarded :%d\n", boarded);
	if(boarded == car_size) {
		mysem_up(&depart);
		boarded = 0;
	}
	else
		mysem_up(&board);
	mysem_up(&mutex1);

	mysem_down(&arrive);

	mysem_down(&mutex2);
	unboarded++;
	if(unboarded == car_size) {
		mysem_up(&unboard);
		unboarded = 0;
	}
	else
		mysem_up(&arrive);
	mysem_up(&mutex2);
	
	return NULL;
}

void set_sem_value(mysem_t *mutex, int n) {
    mutex->init = 0;
    mutex->semid = semget(IPC_PRIVATE,1,S_IRWXU);
    mysem_init(mutex, n);
}

int main(int argc, char *argv[]) {
    pthread_t car_th, th[N];
	int passengers_num , delay, i = 0, sum = 0;

    if(argc != 2){
    	printf("Invalid argument\n");
        return 0;
    }

    car_size = atoi(argv[1]);
	boarded = 0;
    
    set_sem_value(&join_car, 0);
    set_sem_value(&mutex1, 1);
    set_sem_value(&mutex2, 1);
    set_sem_value(&board, 0);
    set_sem_value(&depart, 0);
    set_sem_value(&unboard, 0);
    set_sem_value(&arrive, 0);

	pthread_create(&car_th, NULL, rollercoaster, NULL);
	while(1) {
		scanf("%d %d", &passengers_num, &delay);
		if(passengers_num == -1)
			break;
        sum += passengers_num;
		sleep(delay);
		for(; i < sum; i++){
            pthread_create(&th[i], NULL, passenger, NULL);
		}
	}
	car_ride = (double)sum/car_size;
	printf(" total rides = %f\n",car_ride);
	
    mysem_down(&join_car);
	mysem_destroy(&mutex1);
	mysem_destroy(&mutex2);
	mysem_destroy(&board);
	mysem_destroy(&depart);
	mysem_destroy(&arrive);
	mysem_destroy(&unboard);
    
    return 0;
}

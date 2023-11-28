#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define N 100

struct mystruct {
    int i;
    int direction;
};

int car_size, ride = 0;
double car_ride = 10;
int boarded = 0, waiting = 0;
pthread_mutex_t mutex;
pthread_cond_t board, full, unboard;

void* rollercoaster(){
	int i;
	while(ride < car_ride){
		//load
		pthread_mutex_lock(&mutex);
		printf("Loading\n");
		for(i = 0; i < car_size; i++)
			pthread_cond_signal(&board);
		waiting = 1;
		printf("Waitng to depart\n");
		pthread_cond_wait(&full, &mutex);
		pthread_mutex_unlock(&mutex);

		printf("Ready to depart\n");
		sleep(3);

		//unload
		pthread_mutex_lock(&mutex);
		printf("Car arrived\n");
		pthread_cond_broadcast(&unboard);
		printf("Unloading\n");
		pthread_cond_wait(&full, &mutex);	
		pthread_mutex_unlock(&mutex);
		printf("ENT OF RIDE :%d\n",ride);
		
		ride++;
	}
	return NULL;
}

void* passenger(){
	//take ride
	pthread_mutex_lock(&mutex);
	if(waiting == 0)
		pthread_cond_wait(&board, &mutex);
	boarded++;
	printf("boarded :%d\n", boarded);
	if(boarded == car_size) {
		pthread_cond_signal(&full);
		waiting = 0;
	}
	
	pthread_cond_wait(&unboard, &mutex);
	boarded--;
    
	if(boarded == 0)
		pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
    
	return NULL;
}


int main(int argc, char *argv[]) {
    pthread_t car_th, th[N];
	int passengers_num , delay, i = 0, sum = 0;

    if(argc != 2){
    	printf("Invalid argument\n");
        return 0;
    }

    car_size = atoi(argv[1]);
    
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&board, NULL);
    pthread_cond_init(&full, NULL);
    pthread_cond_init(&unboard, NULL);

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

	printf("total rides = %f\n",car_ride);

	
    pthread_join(car_th,NULL);
    
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&board);
	pthread_cond_destroy(&full);
	pthread_cond_destroy(&unboard);
    
    return 0;
}

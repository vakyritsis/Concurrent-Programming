#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define N 100

pthread_mutex_t mutex;
pthread_cond_t q[2];
int capacity, inside = 0, turn,waiter[2] = {0,0};

void man_enter(){
	pthread_mutex_lock(&mutex);
	while(inside == capacity || (inside > 0 && turn != 0) || (waiter[1] != 0)){
		waiter[0]++;
		pthread_cond_wait(&q[0], &mutex);
		waiter[0]--;
	}
	inside++;
	turn = 0;
	pthread_mutex_unlock(&mutex);	
}
void woman_enter(){
	pthread_mutex_lock(&mutex);
	while(inside == capacity || (inside > 0 && turn != 1)){
		waiter[1]++;
		pthread_cond_wait(&q[1], &mutex);
		waiter[1]--;
	}
	inside++;
	turn = 1;
	pthread_mutex_unlock(&mutex);
}
void man_exit(){
	int i;
	pthread_mutex_lock(&mutex);
	inside--;
	if(waiter[0] > 0 && waiter[1] == 0)
		pthread_cond_signal(&q[0]);
	else if(inside == 0 && waiter[1] > 0)
		for(i = 0; i < capacity; i++)
			pthread_cond_signal(&q[1]);
	pthread_mutex_unlock(&mutex);
}
void woman_exit(){
	int i;
	pthread_mutex_lock(&mutex);
	inside--;
	if(waiter[1] > 0)
		pthread_cond_signal(&q[1]);
	else if(inside==0){
		for(i=0; i < capacity; i++)
			pthread_cond_signal(&q[0]);
	}
	pthread_mutex_unlock(&mutex);	
}

void* man(void *arg){
	int i = *(int *)arg;
	//get in wc
	man_enter();

	printf("Man %d enters wc \n",i);
	sleep(1);
	
	//exit wc
	man_exit();
	printf("Man %d exits wc \n",i);
	free(arg);
    return NULL;
}

void* woman(void *arg){
	int i = *(int *)arg;
	//get in wc
	woman_enter();
	
	printf("\x1B[35m");
	printf("WOMAN %d enters wc \n",i);
	printf("\x1B[37m");
	sleep(1);
	
	//exit wc
	woman_exit();

	printf("\x1B[35m");
	printf("WOMAN %d exits wc \n",i);
	printf("\x1B[37m");
	free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    int gender, gender_number, delay, sum = 0, i = 0, *temp;
    pthread_t th[N];
    
    if(argc != 2) {
        printf("Give an argument\n");
        return 1;
    }
    capacity = atoi(argv[1]);
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&q[0], NULL);
    pthread_cond_init(&q[1], NULL);

    while(1) {
		scanf("%d %d %d", &gender, &gender_number, &delay);
		if(gender == -1)
			break;
        sum += gender_number;
		sleep(delay);
		for(; i < sum; i++) {
            temp = (int*)malloc(sizeof(int));
            *temp = i;
			if(gender==0)
            	pthread_create(&th[i], NULL, man, temp);
			else
				pthread_create(&th[i], NULL, woman, temp);	
		}
	}
	for(i = 0; i<sum;i++)
		pthread_join(th[i], NULL);
	
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&q[0]);
    pthread_cond_destroy(&q[1]);

    return 0;
}

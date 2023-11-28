#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int number, terminate, count = 0;
pthread_mutex_t mutex;
pthread_cond_t full,empty;

void isPrime(int n,int t) {
    int i;
    
    for (i = 2; i <= n - 1; i++) {
        if (n%i == 0) {
            printf("thread %d %d 0\n",t, n);
            return;
        }
    }
    printf("thread %d %d 1\n",t, n);
	return;
}

void *foo(void *arg) {
    int i = *(int *)arg, job=0;
	
	while(1){
		pthread_mutex_lock(&mutex);
		while(count == 0){
			pthread_cond_wait(&empty, &mutex);
		}	
		
		job=number;
		count--;
        
		if(count == 0)
			pthread_cond_signal(&full);
		pthread_mutex_unlock(&mutex);
		if(job != -1)
			isPrime(job,i);
		if(terminate == 1)
			break;
	} 
	
	count = 1;
	pthread_cond_signal(&empty);
	printf("THREAD %d BREAK LOOP\n",i);	
	free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    int workers, i, input = 0, *temp;
 
    if(argc != 2) {
        printf("Give an argument\n");
        return 1;
    }
    workers = atoi(argv[1]);
    pthread_t th[workers];
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&full, NULL);
    pthread_cond_init(&empty, NULL);

    for(i = 0; i < workers; i++) {
		temp = malloc(sizeof(int));
        *temp = i;
        pthread_create(&th[i], NULL, foo, temp);   
    }

    while(input !=- 1) {
		scanf("%d", &input);

		pthread_mutex_lock(&mutex);
		while(count == 1)
			pthread_cond_wait(&full, &mutex);
		number = input;
		count++;
		if(count == 1)
			pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
    }

	terminate = 1;
	count = 1;
	pthread_cond_signal(&empty);
	printf("MAIN THREAD BREAK\n");

	for(i = 0; i < workers; i++)
		pthread_join(th[i],NULL);
		
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&full);
    pthread_cond_destroy(&empty);
    return 0;
}

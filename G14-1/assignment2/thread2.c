#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

volatile int terminate = 0; 

struct poppy {
    int number; // the value to check if it is prime
    int available;
    int t_terminate;
    int create;
};

volatile struct poppy *array = NULL;

void isPrime(int n) {
    int i;
    printf("%d ", n);
    for (i = 2; i <= n - 1; i++) {
        if (n%i == 0) {  
            printf("0\n");
            return;
        }
    }
    printf("1\n");
}

int isAvailable(int workers) {
    int i, check = -1;
    while(check == -1) {
        for(i = 0; i < workers; i++) {
            if(array[i].available == 1) {
                check = i;
                break;
            }
        }
    }
    return check;
}

void *foo(void *arg){
    int i = *(int *)arg;
    array[i].create=1;
    //printf("hi from thread %d\n", i);
    while(1) {
        while(1){
            if(array[i].number != 0 || terminate == 1)
                break;
        }
        if(terminate == 1) {
            //printf("terminate\n");
            break;
        }
        isPrime(array[i].number);
        array[i].number = 0;
        array[i].available = 1;
    }
    array[i].t_terminate = 1;
    //printf("bye from thread %d\n", i);
    return NULL;
}

int main(int argc, char *argv[]) {
    int workers, i, input, nextAvailable;
    clock_t t;
    
    t = clock();
    
    if(argc != 2) {
        printf("Give an argument\n");
        return 1;
    }
    workers = atoi(argv[1]);
    pthread_t th[workers];
    array = calloc(workers, sizeof(struct poppy));
    
    for(i = 0; i < workers; i++) {
        array[i].available = 1;
        array[i].number = 0;
        array[i].t_terminate = 0;
        array[i].create = 0;
    }
    
    for(i = 0; i < workers; i++) {
        pthread_create(&th[i], NULL, foo, &i);
        while(1){
            if(array[i].create == 1)
                break;
        }
    }
 
    while(1) {
        scanf("%d", &input);
        if(input == -1)
            break;
        nextAvailable = isAvailable(workers);
        array[nextAvailable].number = input;    
        array[nextAvailable].available = 0;
    }
    
    for(i = 0; i < workers; i++){
        while(array[i].available != 1){
        }
    }
    terminate = 1;
    for(i = 0; i < workers; i++){
        while(array[i].t_terminate != 1);
    }
    
    free((void*)array);
    
    t = clock() - t;
    
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("The program took %f seconds to execute\n", time_taken);
    
    return 0;
}

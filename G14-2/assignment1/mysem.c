#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h> 
#include <string.h>
#include <unistd.h>
#include "mysem.h"
/*
int mysem_init(mysem_t *s, int n); Αρχικοποίηση σηματοφόρου με τιμή n. Επιστρέφει 1 για επιτυχία, 0 αν
το n<0 ή -1 αν ο σηματοφόρος είναι ήδη αρχικοποιημένος.
int mysem_down(mysem_t *s); Μείωση σηματοφόρου κατά 1. Επιστρέφει 1 για επιτυχία ή -1 αν ο
σηματοφόρος δεν έχει αρχικοποιηθεί.
int mysem_up(mysem_t *s); Αύξηση σηματοφόρου κατά 1. Επιστρέφει 1 για επιτυχία, 0 αν ο
σηματοφόρος είναι ήδη 1 ή -1 αν ο σηματοφόρος δεν έχει αρχικοποιηθεί.
int mysem_destroy(mysem_t *s); Καταστρέφει τον σηματοφόρο. Επιστρέφει 1 για επιτυχία ή -1 αν ο
σηματοφόρος δεν έχει αρχικοποιηθεί
*/

int mysem_init(mysem_t *s, int n){
    if(s->init == 1){
        printf("already init\n");
        return -1;
    }
    else if(n < 0){
        printf("trying to init negative value\n");
        return 0;
    }
    if(semctl(s->semid,0,SETVAL,n) == -1){
        printf( "%s\n", strerror(errno) );    
    }
    s->init = 1;
    return 1;    
} 

int mysem_down(mysem_t *s){
    if(s->init == 0){
        printf("not init\n");
        return -1;
    }
    s->op.sem_num = 0; s->op.sem_op = -1; s->op.sem_flg = 0;
    semop(s->semid,&s->op,1);
    return 1;
}

int mysem_up(mysem_t *s){
    if(s->init == 0){
        printf("not init\n");
        return -1;
    }
    if(semctl(s->semid, 0, GETVAL) == 1){
        //printf("already up\n");
        return 0;
    } 
    s->op.sem_num = 0; s->op.sem_op = 1; s->op.sem_flg = 0;
    semop(s->semid,&s->op,1);
    return 1;
}

int mysem_destroy(mysem_t *s){
    
    if(semctl(s->semid,0,IPC_RMID) == -1){
         printf( "%s\n", strerror(errno) );    
    }
    if(s->init == 0){
        printf("not init\n");
        return -1;
    }
    
    return 1;
}
 

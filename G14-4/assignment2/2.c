#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#define MEM 8192
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct thr_t{
	int id; 
	ucontext_t ctx;
}thr_t;

typedef struct node {
	struct node *next;
	thr_t *data;
} node; /*Node for queue*/

typedef struct queue {
	struct node *front;
	struct node *rear;
	int count;
} queue;

thr_t th1,th2;
queue ready_q, finish_q;
thr_t main_t;
thr_t *current;
ucontext_t main_context;
int cancel_current;

void queue_init (queue *que) {
	que->front = que->rear = NULL;
	que->count = 0;
}

int enque(queue *que, thr_t *thread){
	node *temp = (node *) malloc(sizeof(node));

    if (temp == NULL) {
		return 0;
	}
	temp->data = thread;
	temp->next = NULL;

    if (que->rear){
        que->rear->next = temp;
        que->rear = temp;
    }
    else{
        que->rear = temp;
        que->front = temp;
    }
	que->count++;

	return 1;
}

thr_t *deque(queue *que){
    if (que->front == NULL) 
		return NULL;
		
    node *temp = (node *) malloc(sizeof(node));
    temp = que->front;
    que->front = que->front->next;
    thr_t *ret = NULL;
	ret = temp->data;
    free(temp);
	que->count--;
    
	return ret;	
}

int que_size (queue *que) {
    return que->count;
}

thr_t *get_q_ele (queue *que, int num) {
    int i=0;
	thr_t *data = 0;
	node *temp = que->front;
    if (que->count <= 0) {
    	return 0;
    }
	while (temp) {
    	if (i == num) {
            data = temp->data;
			return data;
        }
        temp = temp->next;
		i++;
    }
	
	return 0 ;
}

int remove_q_ele(queue *que, thr_t *thread) {
    node *curr = que->front;
    node *prev = NULL;
    if (curr == NULL) 
		return 0;

	if (que->count <= 0) {
    	return 0;
  	}
	while (curr) {
        if (curr->data->id == thread->id) {
            if (prev == NULL) {
                que->front = curr->next;
            } 
			else{
                prev->next = curr->next;
            }
			if (curr == que->rear) {
                que->rear = prev;
            }
            free(curr);
			que->count--;
                return 1;
            }
        prev = curr;
        curr = curr->next;
    }

    return 0;
}

void start_time() {
	ualarm(100,0);
}

void stop_time() {
	ualarm(0,0);
}

void scheduler() {  
	thr_t *prev, *next;
    //stop_time();
	ualarm(0,0);
	getcontext(&main_context);
	prev = current;
	if(cancel_current == 0){
		enque(&ready_q, prev);
	}
	else{
		cancel_current = 0;
	}
	next = deque(&ready_q);
	if (next == NULL) {
	    printf("No thread present in ready queue\n");
		exit(EXIT_SUCCESS);
	}
	current = next;
	ualarm(100,0);
	swapcontext(&prev->ctx,&next->ctx);
    //start_time();
}

void my_threads_init(){
	queue_init(&ready_q);
	queue_init(&finish_q);
	/*Setting up signal handler*/
	signal(SIGALRM, scheduler);
	//start_time();
	ualarm(100, 0);
	
	main_t.id = -1;
	if ( getcontext(&(main_t.ctx)) == -1) {
		printf("Error while getting context...exiting\n");
		exit(EXIT_FAILURE);
	}
	current = &main_t;
}

int thr_cancel (thr_t thread) {
	/* Remove the thread from the ready queue. */
	int found = 0, i;
	thr_t* thr;

  	for (i = 0; i < que_size(&ready_q); ++i) {
    	thr = get_q_ele(&ready_q, i);
    	if(thr->id == thread.id){
      		if (!remove_q_ele(&ready_q, thr)) {
        		return 1;
      		} else {
        		found = 1;
        		break;
      		}
    	}
  	}

	if (thread.id == current->id) {
	/* If it is the current thread, reschedule. */
		cancel_current = 1;
		scheduler();
		return 0;
	}

	if (!found) {
		return 1;
	}

	return 0;
}

void thr_exit(void *retval) {
	//current->retval = retval;
	enque(&finish_q, current);
	thr_cancel(*current);
}

void thread_run (void* (*start_routine)(void*), void* arg) {
  thr_exit(start_routine(arg));
  return;
}

void my_threads_create(thr_t *thr, void(body)(void*) ,void *arg){
	stop_time();
	thr->id	= que_size(&ready_q);
	getcontext(&thr->ctx);
	thr->ctx.uc_link = &main_context;
	thr->ctx.uc_stack.ss_sp = malloc(MEM);
	thr->ctx.uc_stack.ss_size = MEM;
	makecontext(&thr->ctx, (void*) thread_run, 2, body, arg);
	enque(&ready_q, thr);
	start_time();
}

int my_threads_join(thr_t *thr, void **status ){
	int i;
	if (thr == current) {
		return 1;
	}
	thr_t *th = 0;
	while(1){
		for (i = 0; i < que_size(&finish_q); i++) {
      		thr = get_q_ele(&finish_q, i);
     		if (th == thr) {
        		remove_q_ele(&finish_q, thr);	
      		}				
    	}
		stop_time();
		start_time();
		scheduler();
	}
}

int j=0;
void thread1(){
	int i;
	printf("hi from thread 1\n");
	for(i=0; i<10; i++){
        sleep(1);
		printf("thread1\n");
		j++;
	}
}

void thread2(){
	int i;
	printf("hi from thread 2\n");
	for(i=0; i<10; i++){
		printf("thread2\n");
		j++;
	}
}

int main(int argc, char *argv[]){
	my_threads_init();
	
	my_threads_create(&th1, thread1, NULL);
	my_threads_create(&th2, thread2, NULL);
	
	my_threads_join(&th1, NULL);
	my_threads_join(&th2, NULL);
	return 0;
}

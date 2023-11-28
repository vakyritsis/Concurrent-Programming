#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#define MEM 8192
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

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
	ualarm(10000,0);
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
	ualarm(10000,0);
	swapcontext(&prev->ctx,&next->ctx);
    //start_time();
}

void my_threads_init(){
	queue_init(&ready_q);
	queue_init(&finish_q);
	/*Setting up signal handler*/
	signal(SIGALRM, scheduler);
	//start_time();
	ualarm(10000, 0);
	
	main_t.id = -1;
	if ( getcontext(&(main_t.ctx)) == -1) {
		printf("Error while getting context...exiting\n");
		exit(EXIT_FAILURE);
	}
	current = &main_t;
}

int yield() {
	stop_time();
	start_time();
	scheduler();
	return 0;
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

// TYPE : int 0, char 1, string 2, scan_int 3, scan char 4, scan_string 5

typedef struct t_node{
    int  type;
    int d;
    char c;
    char s[100];
    struct t_node *next;
}t_node;

typedef struct tuple{
    t_node *head;
    t_node *tail;
    struct tuple *next;
}tuple;

typedef struct tuple_space{
    tuple *head;
    tuple *tail;
}tuple_space;

tuple_space space;

int tuple_out(char *fmt, ...){
    t_node *temp;
    tuple *t = malloc(sizeof(tuple));
    t->head = t->tail = NULL;
    char * traverse;
    va_list arg;
    va_start(arg, fmt);
    for(traverse = fmt; *traverse != '\0'; traverse++){
            switch(*traverse){
                case 'c':
                    temp = malloc(sizeof(t_node));
                    temp->type = 1;
                    temp->c =  (va_arg(arg, int));
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
                case 'd':
                    temp = malloc(sizeof(t_node));
                    temp->type = 0;
                    temp->d =  (va_arg(arg, int));
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
                case 's':
                    temp = malloc(sizeof(t_node));
                    temp->type = 2;
                    strcpy(temp->s,va_arg(arg, char*));
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
            }
        }
        va_end(arg);
    

    if(space.head == NULL){
        space.head = t;
        space.tail = t;
    }
    else{
        space.tail->next = t;
        space.tail = space.tail->next;
    }
    return 1;
}

int cmp(tuple *a, tuple *b){
    t_node *temp1,*temp2;
    temp1 = a->head;
    temp2 = b->head;
    while(temp1!=NULL && temp2!=NULL){
        if(temp1->type != temp2->type && temp1->type+3 != temp2->type)
            return 0;

        switch(temp2->type){
            case 1:
                if(temp1->c != temp2->c)
                    return 0;
                break;
            case 0:
                if(temp1->d != temp2->d)
                    return 0;
                break;
            case 2:
                if(strcmp(temp1->s ,temp2->s))
                    return 0;
                break;
            case 3:
                temp2->d = temp1->d;
                break;
            case 4:
                temp2->c = temp1->c;
                break;
            case 5:
                strcpy(temp2->s,temp1->s);
                break;
        }
        temp1 = temp1->next;
        temp2 = temp2->next;
    }
    return (temp1==NULL && temp2==NULL);
}

void print(tuple t){
    t_node *temp;
    if(t.head == NULL){
        printf("Empty\n");
        return;
    }

    temp = t.head;
    while(temp != NULL){
        switch(temp->type){
            case 0:
                printf("%d ",temp->d);
                break;
            case 1:
                printf("%c ",temp->c);
                break;
            case 2:
                printf("%s ",temp->s);
                break;
        }
        temp = temp->next;
    }
}

int tuple_in(char *fmt,...){
	stop_time();
    t_node *temp;
    tuple *t = malloc(sizeof(tuple));
    char * traverse;
    va_list arg;
    va_start(arg, fmt);
    for(traverse = fmt; *traverse != '\0'; traverse++){
        if(*traverse != '%'){
            switch(*traverse){
                case 'c':
                    temp = malloc(sizeof(t_node));
                    temp->type = 1;
                    temp->c =  (va_arg(arg, int));
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
                case 'd':
                    temp = malloc(sizeof(t_node));
                    temp->type = 0;
                    temp->d =  (va_arg(arg, int));
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
                case 's':
                    temp = malloc(sizeof(t_node));
                    temp->type = 2;
                    va_arg(arg, char*);
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;

            }
        }
        else{
            traverse++;
            switch(*traverse){
                case 'c':
                    temp = malloc(sizeof(t_node));
                    temp->type = 4;
                    *va_arg(arg, int*) = temp->c;
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
                case 'd':
                    temp = malloc(sizeof(t_node));
                    temp->type = 3;
                    *va_arg(arg, int*) = temp->d;
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
                case 's':
                    temp = malloc(sizeof(t_node));
                    temp->type = 5;
                    va_arg(arg, char**);
                    temp->next = NULL;
                    if(t->head == NULL){
                        t->head = temp;
                        t->tail = temp;
                    }
                    else{
                        t->tail->next = temp;
                        t->tail = t->tail->next;
                    }
                    break;
            }
        }
    }
    va_end(arg);

    
    tuple *curr, *prev;
	while(1){
		if(space.head == NULL){
			printf("Not found\n");
			return 0;
		}
		curr = space.head;
		prev = space.head;
		while(curr!=NULL){
			if(cmp(curr, t)){    
				t_node *temp1;
				temp1 = t->head;
				va_start(arg, fmt);
				for(traverse = fmt; *traverse != '\0'; traverse++){
					if(*traverse != '%'){
						va_arg(arg, void*);
						temp1 = temp1->next;
					}
					else{
						traverse++;
						switch(*traverse){
							case 'c':
								*va_arg(arg, int*) = temp1->c;
								temp1 = temp1->next;
								break;
							case 'd':
								*va_arg(arg, int*) = temp1->d;
								temp1 = temp1->next;
								break;
							case 's':
								strcpy(va_arg(arg, char*),temp->s);
								temp1 = temp1->next;                    
						}
					}
				}
				va_end(arg);
				//delete curr tuple
				if(curr == space.head)
					space.head = space.head->next;
				else{
					prev->next = curr->next;
					curr->next = NULL;
				}
				//delete_tuple(curr);
				start_time();
				return 1;
			}
			prev = curr;
			curr = curr->next;
		}
		start_time();
		yield();
		stop_time();
	}
    return 0;
}

void print_space(tuple_space s){
    tuple * temp;

    if(s.head == NULL){
        printf("Empty space\n");
        return;
    }
    printf("Tuple space:\n");
    temp = s.head;
    while(temp != NULL){
        print(*temp);
        printf("\n");
        temp = temp->next;
    }
}
//end of threadlib

#define N 100

struct mystruct {
    int i;
    int direction;
};
int capacity, cars  = 0, bridge_direction = -1, waiter[2] = {0,0}, crosses = 0;

void car(void* arg){
	struct mystruct *temp = (struct mystruct *)arg;
    int direction =  temp->direction;
    int i = temp->i, j;
	int flag = 0;
	//enter bridge
	tuple_in("s","mutex");
	if(cars == capacity || (cars > 0 && direction != bridge_direction) || 
	(crosses+cars >= 2*capacity && waiter[1-direction]!=0)){
		waiter[direction]++;
		tuple_out("s","mutex");
		tuple_in("d", direction);
		tuple_in("s","mutex");
		waiter[direction]--;
		flag=1;
	}
	cars++;
	if(flag == 0){
		if(bridge_direction==-1)
			tuple_in("d", -1);
		else{
			tuple_in("d", direction);
		}	
	}
	bridge_direction = direction;
	/*
	if(waiter[direction]>0 && cars<capacity && 
	(cars+crosses < 2*capacity || waiter[1-direction]==0)){
		pthread_cond_signal(&q[direction]);	
	}
	*/
	tuple_out("s","mutex");

	printf("Bridge cars :%d\n",cars);
	if(direction == 0)
    	printf("\x1B[31mgoing in bridge %d dir:%d\n",i, direction);
	else
		printf("\x1B[36mgoing in bridge %d dir:%d\n",i, direction);
	printf("\x1B[37m");
	sleep(1);

	//exit bridge
	tuple_in("s","mutex");
	crosses++;
    cars--;
	if(waiter[direction] > 0  && ( cars+crosses<2*capacity || waiter[1-direction]==0)){
		tuple_out("d", direction);
	}
	else if(cars == 0 && waiter[1-direction] > 0 &&
	(waiter[direction]==0 || waiter[1-direction]+crosses >= 2*capacity ) ){
		bridge_direction = 1 - direction;
		crosses = 0;
		for(j=0; j<capacity;j++)
			tuple_out("d", 1-direction);
	}
	else if(cars == 0 && waiter[1-direction] == 0 && waiter[direction] == 0){
		bridge_direction = -1;
		crosses = 0;
	}
	tuple_out("s","mutex");
	printf("exit %d\n",i);
	free(arg);

}

int main(int argc, char* argv[]){
	int  i = 0, car_direction, numberofcars, delay, sum = 0, j;
    struct mystruct *temp;
	thr_t th[N];

	if(argc != 2) {
        printf("Give an argument\n");
        return 1;
    }
    capacity = atoi(argv[1]);
	my_threads_init();
	tuple_out("s","mutex");
	for(j=0; j<capacity;j++){
		tuple_out("d",-1);	
	}
	
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
            my_threads_create(&th[i], car, temp);
		}
	}

    return 0;
}

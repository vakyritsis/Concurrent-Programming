#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#define MEM SIGSTKSZ
#define BUFF_SIZE 64


int stop = 0;
//ucontext_t t1, t2, Main;
typedef void (*Fun)(void *arg);

typedef struct coro{
	ucontext_t ctx;
	Fun func; //user function for coroutine execution
    void *arg; //parameter of func
	int state;  // idle | ready | executing | suspended
}co_t;

typedef struct ringBuffer{
	char *buff; // the array that we store the bytes
    int start; // the index we read from
    int end;  // the index we write at
    int size; // elements in the buffer
} ringBuffer;

ringBuffer r_buff;
co_t t1, t2, Main;

int mycoroutines_init(co_t *main){
	getcontext(&Main.ctx);
	return 0;
}

int mycoroutines_create(co_t *co, void(body)(void*) ,void *arg){
    getcontext(&co->ctx);
	co->ctx.uc_link = &Main.ctx;
	co->ctx.uc_stack.ss_sp = malloc(MEM);
	co->ctx.uc_stack.ss_size = MEM;
	//create
	makecontext(&co->ctx, (void*) body, 1, arg);
	return 0;
}

int mycoroutines_switchto(co_t *start ,co_t *co){
	swapcontext(&start->ctx, &co->ctx);
	return 0;
}

void mycoroutines_destroy(co_t* co){
	free(co->ctx.uc_stack.ss_sp);
}

void thread1(void *arg){
	char *name = (char *)arg;
	FILE *fd;
	int res;
	char ch;
	fd = fopen(name, "r");

	while(1){
		if((r_buff.start+1) % r_buff.size == r_buff.end ){
			mycoroutines_switchto(&t1, &t2);
		}
		res = fread(&ch, sizeof(char), 1, fd);
		//printf("%c\n",ch);
		if(res==0){
			break;
		}
		r_buff.buff[r_buff.start] = ch;
		r_buff.start = (r_buff.start+1) % r_buff.size;
	}
	stop = 1;
	mycoroutines_switchto(&t1, &t2);
}

void thread2(void* arg){
	FILE *fd;
    char ch;

	fd = fopen("temp", "w+");
	while(1){
		if(r_buff.end == r_buff.start){
			mycoroutines_switchto(&t2, &t1);
			//continue;
		}
		ch = r_buff.buff[r_buff.end];
		if(r_buff.end == r_buff.start && stop==1){
			printf("2 break\n");
			break;
		}
		r_buff.end = (r_buff.end + 1) % r_buff.size;
		fwrite(&ch, sizeof(char), 1, fd);
	}
}

int main(int argc, char *argv[]){
	
    if(argc != 2) {
        printf("Give file name as an argument\n");
        return -1;
    }
	mycoroutines_init(&Main);
    
	mycoroutines_create(&t1, thread1, argv[1]);
	mycoroutines_create(&t2, thread2, NULL);

	r_buff.start = r_buff.end = 0;
	r_buff.buff = malloc(BUFF_SIZE*sizeof(char));
	r_buff.size = BUFF_SIZE;

	mycoroutines_switchto(&Main, &t1);
	printf("completed\n");
	free(r_buff.buff);
	mycoroutines_destroy(&t1);
	mycoroutines_destroy(&t2);
	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define PIPE_NUM 2

typedef struct ringBuffer{
	char *buff; // the array that we store the bytes
    int start; // the index we read from
    int end;  // the index we write at
    int size; // the size of the buffer
    int count; // elements in the buffer
	int openWrite; // bolean variable: 1 if the pipe is open for writting, 0 if closed
	int openRead;  // bolean variable: 1 if the pipe is open for reading, 0 if closed
	int writestart;
} ringBuffer;

int pipeID[PIPE_NUM];
volatile ringBuffer rBuff[PIPE_NUM];
int indexx;
int turn;
int wantP = 0 ,wantQ = 0;
int turn2;
int wantP2 = 0 ,wantQ2 = 0;

int pipe_open(int size) {
	int i;
	rBuff[indexx].buff = (char*)malloc(sizeof(char) * size); // na ginei free
	for(i = 0; i < size; i++)
		rBuff[indexx].buff[i] = '\0'; // the character '\0' means that the slot in the array is empty
	rBuff[indexx].start = 0;
	rBuff[indexx].end = 0;
	rBuff[indexx].size = size;
	rBuff[indexx].count = 0;
	rBuff[indexx].openWrite = 1;
	rBuff[indexx].openRead = 1;
    rBuff[indexx].writestart = 0;
	int id = indexx + 1;

	return id;
}

int pipe_writeDone(int p) {
	int i;
	
	for(i = 0; i < PIPE_NUM; i++) {
		if(pipeID[i] == p)
			break;
	}
	if(i == PIPE_NUM)
		return -1;

	if(rBuff[p - 1].openWrite == 1) {
		rBuff[p - 1].openWrite = 0;
		return 1;
	}
	else 
	return -1;
}

int pipe_write(int p, char	c) { 
	int i;

	for(i = 0; i < PIPE_NUM; i++) {
		if(pipeID[i] == p)
			break;
	}
	if(i == PIPE_NUM)
		return -1;

	if(rBuff[p - 1].openWrite == 0)
		return -1;

	rBuff[p-1].buff[rBuff[p-1].end] = c;
	rBuff[p-1].end = (rBuff[p-1].end + 1) % rBuff[p-1].size;
	rBuff[p-1].count++;
	
	return 1;
}

int pipe_read(int p, char *c) {
	int i;

	if(rBuff[p-1].count == 0 && rBuff[p-1].openWrite == 0) {
		// close pipe
		rBuff[p-1].openRead = 0;
		return 0;
	}

	for(i = 0; i < PIPE_NUM; i++) {
		if(pipeID[i] == p)
			break;
	}
	if(i == PIPE_NUM)
		return -1;

	*c = rBuff[p-1].buff[rBuff[p-1].start];
	rBuff[p-1].buff[rBuff[p-1].start] = '\0';
	rBuff[p-1].start = (rBuff[p-1].start + 1) % rBuff[p-1].size;
	rBuff[p-1].count--;

	return 1;
}

void *thread1(void *arg) {
    FILE *fd, *out;
    char ch;
    int res;
    int *fin = (int*)arg;

    fd = fopen("file1", "r");
	out = fopen("out", "w");

    while(1) {
		while(1){
			if(rBuff[0].size != (rBuff[0].count))
				break;
		}
        res = fread(&ch, sizeof(char), 1, fd);
        if(res == 0) 
            break;
		wantP=1;
		turn=1;
		while (wantQ && (turn != 0));
        pipe_write(pipeID[0], ch);
		rBuff[0].writestart = 1;	
		wantP = 0;
    }
	wantP=1;
	turn=1;
	while (wantQ && (turn != 0));
    pipe_writeDone(pipeID[0]);
	wantP = 0;

	while(rBuff[0].openRead == 1);

	while(1) {
		while(1){
			if(rBuff[1].count != 0 && rBuff[1].openWrite == 1){
				break;
			}
			else if(rBuff[1].writestart == 1 && rBuff[1].openWrite == 0){
				break;
			}
		}
		wantQ2=1;
		turn2=0;
		while (wantP2 && (turn2!=1));
        res = pipe_read(pipeID[1], &ch);
		if(res == 0) 
            break;
        fwrite(&ch, sizeof(char), 1, out);
		wantQ2 = 0;
    }

	(*fin) = 1;
    return NULL;
}

void *thread2(void *arg) {
    FILE *fd;
    char ch;
    int res;
	int *fin = (int*)arg;
    fd = fopen("temp", "w+");
    while(1) {
		while(1){
			if(rBuff[0].count != 0 && rBuff[0].openWrite == 1){
				break;
			}
			else if(rBuff[0].writestart == 1 && rBuff[0].openWrite == 0){
				break;
			}
		}
		wantQ=1;
		turn=0;
		while (wantP && (turn!=1));
        res = pipe_read(pipeID[0], &ch);
		if(res == 0) 
            break;
        fwrite(&ch, sizeof(char), 1, fd);
		wantQ = 0;
    }
    
	rewind(fd);
	while(1) {
		while(1){
			if(rBuff[1].size != (rBuff[1].count))
				break;
		}
        res = fread(&ch, sizeof(char), 1, fd);
        if(res == 0) 
            break;
		wantP2=1;
		turn2=1;
		while (wantQ2 && (turn2 != 0));
        pipe_write(pipeID[1], ch);
		rBuff[1].writestart = 1;	
		wantP2 = 0;
    }
	wantP2=1;
	turn2=1;
	while (wantQ2 && (turn2 != 0));
    pipe_writeDone(pipeID[1]);
	wantP2 = 0;

	(*fin) = 1;
    return NULL;
}


int main(int argc, char *argv[]) {
	int thread1_fin , thread2_fin;
    pthread_t t1, t2;
    thread1_fin = 0;
	thread2_fin = 0;
	indexx = 0;
	pipeID[0] = pipe_open(64);
	indexx++;
	pipeID[1] = pipe_open(64);
    pthread_create(&t1, NULL, thread1, &thread1_fin);
    pthread_create(&t2, NULL, thread2, &thread2_fin);
	while(thread1_fin == 0);
	while(thread2_fin == 0);
    
    free(rBuff[0].buff);
    free(rBuff[1].buff);
	return 0;
}

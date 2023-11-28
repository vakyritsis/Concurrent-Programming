#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#define x 100

//struct for thread arguments
struct thread_info {
    FILE *fp;
    int size;
    FILE* out;
	int finish;
};

//make a file with random integers
/*FILE *make(int size){
    int i,number;
    FILE *input;
    srand(time(NULL));   
    
    input = fopen("test", "w");
    for(i=0; i<size; i++){
        number =  rand()%10000; 
        printf("%d ",number);
        fwrite(&number, sizeof(int), 1, input);
    }
    fclose(input);
    printf("\n");
    return input;
}
*/
int file_size(char *file_name) {
    int size;
    
    FILE *fd = fopen(file_name, "rb");
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fclose(fd);
    
    return size / sizeof(int);
}

void merge(int arr[], int p, int q, int r) {

    int n1 = q - p + 1;
    int n2 = r - q;

    int L[n1], M[n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[p + i];
    for (int j = 0; j < n2; j++)
        M[j] = arr[q + 1 + j];

     int i, j, k;
    i = 0;
    j = 0;
    k = p;

    while (i < n1 && j < n2) {
        if (L[i] <= M[j]) {
            arr[k] = L[i];
            i++;
        } 
        else {
            arr[k] = M[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = M[j];
        j++;
        k++;
    }
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {

    int m = l + (r - l) / 2;

    mergeSort(arr, l, m);
    mergeSort(arr, m + 1, r);

    merge(arr, l, m, r);
  }
}

void *thread(void *arg){
    int a[64], i, num;
    int l, r; //integer from left and right files for merge 
    int j,k; //left and right file counters
    pthread_t th_left, th_right;
    
    struct thread_info *in= arg;
    struct thread_info left, right;
    in->out = tmpfile();
    left.fp = tmpfile();
    right.fp = tmpfile();
    
    if((*in).size <= 64){
        rewind(in->fp);
        fread(a, sizeof(int), in->size, in->fp);
        mergeSort(a,0, in->size - 1);
        fwrite(a, sizeof(int), in->size, in->out);
        in->finish = 1;
        return NULL;
    }
    
    // split into 2 files
    rewind(in->fp);
    for(i=0; i < in->size; i++){
        fread(&num, sizeof(int), 1, in->fp);
        if(i < in->size/2)
            fwrite(&num, sizeof(int), 1, left.fp);
        else
            fwrite(&num, sizeof(int), 1, right.fp);
    }
    
    // call mergesort for the two sub-files
    left.finish = 0;
    left.size = in->size/2;
    right.finish = 0;
    right.size = in->size - in->size/2;
    
    pthread_create(&th_left , NULL, thread, &left);
    pthread_create(&th_right , NULL, thread, &right);

    while(left.finish == 0);
    while(right.finish == 0);
    //merge the 2 sorted files 
    j = 0;
    k = 0;
    rewind(left.out);
    rewind(right.out);
    fread(&l, sizeof(int), 1, left.out);
    fread(&r, sizeof(int), 1, right.out);
    while(j < in->size/2 && k < in->size - in->size/2){
        if(l <= r){
            fwrite(&l, sizeof(int), 1, in->out);
            fread(&l, sizeof(int), 1, left.out);
            j++;
        }
        else{
            fwrite(&r, sizeof(int), 1, in->out);	
            fread(&r, sizeof(int), 1, right.out);
            k++;
        }
    }
    while(j < in->size/2){
        fwrite(&l, sizeof(int), 1, in->out);
        fread(&l, sizeof(int), 1, left.out);
        j++;
    }
    while(k < in->size - in->size/2){
        fwrite(&r, sizeof(int), 1, in->out);	
        fread(&r, sizeof(int), 1, right.out);
        k++;
    }
    in->finish = 1;
    return NULL;
}

int main(int argc, char* argv[]){
    struct thread_info in;
    FILE *output;
    int i, number;
    
    if(argc != 2) {
        printf("GIVE AN ARGUMENT\n");
        return -1;
    }
    
    in.size = file_size(argv[1]);
    in.finish = 0;
    //in.fp = make(x);
    in.fp = fopen(argv[1], "rb+");
    output = fopen("out", "wb+");
    pthread_t th;

    pthread_create(&th , NULL, thread, &in);
    while(in.finish == 0);
    rewind(in.out);
    for(i = 0; i < in.size; i++){
        fread(&number, sizeof(int), 1, in.out);
        fwrite(&number, sizeof(int), 1, output);
    }
    
    rewind(output);
    for(i = 0; i < in.size; i++){
        fread(&number, sizeof(int), 1, output);
        printf("%d ",number);
    }
    printf("\n");
    fclose(in.fp);
    fclose(output);
    return 0;
}

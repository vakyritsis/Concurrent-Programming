#ifndef __MYSEM_   /* Include guard */
#define __MYSEM_

typedef struct {
    int semid;
    int init;
    struct sembuf op;
}mysem_t;

int mysem_init(mysem_t *s, int n);

int mysem_down(mysem_t *s);

int mysem_up(mysem_t *s);

int mysem_destroy(mysem_t *s);

#endif // __MYSEM_
 

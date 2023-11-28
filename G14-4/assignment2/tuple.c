#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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
	//stop time
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
	//while(1){}
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
			//start time
            return 1;
        }
		prev = curr;
        curr = curr->next;
    }
	//yield()
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

int main(int argc, char* argv[]){
    space.head = space.tail = NULL;
    int d=0;
    char s[10];
    tuple_out("cds",'a',4,"paiktes");
    tuple_out("c",'b');
    tuple_out("s","alex");
    tuple_out("cds",'c',2,"alex");
    int r = tuple_in("c%d%s",'a',&d,s);
    printf("int %d, string %s\n",d,s);
    printf("Found %d?\n", r);
    print_space(space);
    return 0;
}

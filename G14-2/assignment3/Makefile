CC = gcc
CCFLAGS = -Wall -g
OBJ = 3.o mysem.o
3: $(OBJ)
	$(CC) $(OBJ) -o 3 -lpthread
3.o: 3.c ../assignment1/mysem.h
	$(CC) $(CCFLAGS) -c  3.c
mysem.o: ../assignment1/mysem.c ../assignment1/mysem.h
	$(CC) $(CCFLAGS) -c ../assignment1/mysem.c

.PHONY: clean
clean:
	rm *.o

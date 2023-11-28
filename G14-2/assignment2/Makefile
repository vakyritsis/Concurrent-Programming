CC = gcc
CCFLAGS = -Wall -g
OBJ = 2.o mysem.o
2: $(OBJ)
	$(CC) $(OBJ) -o 2 -lpthread
2.o: 2.c ../assignment1/mysem.h
	$(CC) $(CCFLAGS) -c  2.c
mysem.o: ../assignment1/mysem.c ../assignment1/mysem.h
	$(CC) $(CCFLAGS) -c ../assignment1/mysem.c

.PHONY: clean
clean:
	rm *.o

CC = gcc
CCFLAGS = -Wall -g
OBJ = 4.o mysem.o
4: $(OBJ)
	$(CC) $(OBJ) -o 4 -lpthread
4.o: 4.c ../assignment1/mysem.h
	$(CC) $(CCFLAGS) -c  4.c
mysem.o: ../assignment1/mysem.c ../assignment1/mysem.h
	$(CC) $(CCFLAGS) -c ../assignment1/mysem.c

.PHONY: clean
clean:
	rm *.o



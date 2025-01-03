CC = mpicc
CFLAGS = -Wall -pthread

OBJ = main.o lamport.o arbitration.o utils.o

all: program

program: $(OBJ)
	$(CC) $(CFLAGS) -o program $(OBJ)

main.o: main.c lamport.h arbitration.h utils.h
	$(CC) $(CFLAGS) -c main.c

lamport.o: lamport.c lamport.h
	$(CC) $(CFLAGS) -c lamport.c

arbitration.o: arbitration.c arbitration.h
	$(CC) $(CFLAGS) -c arbitration.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o program

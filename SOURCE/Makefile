CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

all: mishell

mishell: child.o mishell.o
	$(CC) $(CFLAGS) -o mishell child.o mishell.o

child.o: child.c
	$(CC) $(CFLAGS) -c child.c

mishell.o: mishell.c
	$(CC) $(CFLAGS) -c mishell.c

clean:
	rm -f *.o mishell
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

mishell:
	$(CC) $(CFLAGS) mishell.c -o mishell
	./mishell

clean:
	rm -f mishell *.o
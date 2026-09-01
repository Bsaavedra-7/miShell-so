CC = gcc
CFLAGS = -Wall -Wextra -std-gnu11

all: $(TARGET)

$(TARGET): mishell.c
	$(CC) $(CFLAGS) -o $(TARGET) mishell.c

clean:
	rm -f $(TARGET) *.o
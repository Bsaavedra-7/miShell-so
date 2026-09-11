CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -c
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
INC_DIRS := $(shell find -type d)

all:
	$(CC) $(CFLAGS) $(SRCS) 
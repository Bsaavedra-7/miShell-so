CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11
NAME = mishell
SRCS_DIR = SOURCE
SRCS = $(wildcard $(SRCS_DIR)/*.c)
OBJS = $(addprefix $(SRCS_DIR)/, $(SRCS:$(SRCS_DIR)/%.c=%.o))
INC_DIRS := $(shell find $(SRCS_DIR) -type d)

all: $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

$(OBJS) : 
	$(MAKE) -C $(SRCS_DIR)


.PHONY: clean

clean:
	rm -f $(NAME)
	rm -f $(OBJS)
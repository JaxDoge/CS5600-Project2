CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = scheduler
SRCS = main.c scheduler.c process.c input_parser.c utilities.c queue.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
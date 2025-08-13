CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = allocate.c task1.c task2.c task3.c task4.c
OBJS = $(SRCS:.c=.o)

TARGET = allocate

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)




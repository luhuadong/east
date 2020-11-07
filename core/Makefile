CC = gcc
CFLAGS = -Wall -std=gnu99

INC = -I./inc
SRCS=$(wildcard *.c */*.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

FLAG = -g
CFLAGS := -lev -lm -lpthread
NAME=$(wildcard *.c)
#TARGET=$(patsubst %.c, %, $(NAME))
TARGET=east

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o:%.c
	$(CC) -o $@ -c $(CFLAGS) $< -g $(INC)

clean:
	rm -rf $(TARGET) $(OBJS)

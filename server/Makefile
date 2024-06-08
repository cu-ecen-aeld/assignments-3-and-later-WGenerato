CC = gcc
CFLAGS = -Wall -Wextra -Werror

TARGET = aesdsocket
SRCS = aesdsocket.c
OBJS = $(SRCS:.c=.o)
DESTINATION = /var/tmp/aesdsocketdata

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(DESTINATION)


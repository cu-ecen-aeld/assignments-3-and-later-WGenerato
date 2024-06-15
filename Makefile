# Define variables
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Werror
TARGET = aesdsocket

# Define the build process
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -f $(TARGET)

.PHONY: all clean

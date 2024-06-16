# Define the compiler and flags
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror
TARGET := aesdsocket

# Define the source directory
SRC_DIR := server

# Default target to build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRC_DIR)/$(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_DIR)/$(TARGET).c

# Clean target to remove the executable
clean:
	rm -f $(TARGET)

# Declare the phony targets
.PHONY: all clean


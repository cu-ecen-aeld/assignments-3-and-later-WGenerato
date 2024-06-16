# Define the cross-compiler and flags
CROSS_COMPILE := /home/tt20/aesd-assignment-4/buildroot/output/host/bin/aarch64-buildroot-linux-uclibc-
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror
TARGET := aesdsocket

# Default target to build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): server/$(TARGET).c
	@echo "Using compiler: $(CC)"
	@echo "Using flags: $(CFLAGS)"
	$(CC) $(CFLAGS) -o $(TARGET) server/$(TARGET).c

# Clean target to remove the executable
clean:
	rm -f $(TARGET)

# Declare the phony targets
.PHONY: all clean



CC := gcc



ifdef CROSS_COMPILE
    CC := $(CROSS_COMPILE)gcc
endif


SRC := writer.c
OBJ := $(SRC:.c=.o)

 
all: writer


writer: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f writer $(OBJ)


.PHONY: all clean

CC      = gcc
CFLAGS  = -Wall -Wextra -g 

# Files
SRCS = src/main.c src/threads.c src/shared_mem.c
OBJS = $(SRCS:.c=.o)
TARGET = proc

# Default target: build the executable
all: $(TARGET)

# Link object files into the executable related to nfs_manager
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

run: $(TARGET)
	./$(TARGET) 2

valgrind: $(TARGET)
	valgrind ./$(TARGET) 2

clean:
	rm -f $(TARGET) $(OBJS)

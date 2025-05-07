# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Target executable
TARGET = multi-bench

# Source files
SRCS = multi-bench.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
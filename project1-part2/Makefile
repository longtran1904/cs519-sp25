# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Source and output file
SRC = IPC-pipe.c
OUT = IPC-pipe

# Default target
all: $(OUT)

# Build the executable
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

# Compile with DEBUG enabled
debug: CFLAGS += -DDEBUG=1 -g
debug: $(OUT)

# Run the program
run: $(OUT)
	./$(OUT)

# Clean up compiled files
clean:
	rm -f $(OUT)
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Source and output files
SRC_PIPE = IPC-pipe.c
OUT_PIPE = IPC-pipe
SRC_SHMEM = IPC-shmem.c
OUT_SHMEM = IPC-shmem

# Default target
all: $(OUT_PIPE) $(OUT_SHMEM)

# Build the executables
$(OUT_PIPE): $(SRC_PIPE)
	$(CC) $(CFLAGS) $(SRC_PIPE) -o $(OUT_PIPE)

$(OUT_SHMEM): $(SRC_SHMEM)
	$(CC) $(CFLAGS) $(SRC_SHMEM) -o $(OUT_SHMEM)

# Compile with DEBUG enabled
# debug: CFLAGS += -fsanitize=address -DDEBUG=1 -g
debug: CFLAGS += -DDEBUG=1 -g
debug: $(OUT_PIPE) $(OUT_SHMEM)

# Run the IPC-pipe program
run-pipe: $(OUT_PIPE)
	./$(OUT_PIPE)

# Run the IPC-shmem program
run-shmem: $(OUT_SHMEM)
	./$(OUT_SHMEM)

# Clean up compiled files
clean:
	rm -f $(OUT_PIPE) $(OUT_SHMEM)
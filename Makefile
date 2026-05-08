# Compiler to use
CC = gcc

# Compiler flags (-Wall for warnings, -lpthread for thread support)
CFLAGS = -Wall -lpthread

# The name of the final executable
TARGET = banker

# Source files
SRCS = bankers_algo.c

# Default target: compile the program
all: $(TARGET)

# Link and compile the executable
$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS)

# Clean up the built files
clean:
	rm -f $(TARGET)
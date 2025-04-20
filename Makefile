# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Source files
SRC = src/main.c src/shell.c src/alias.c src/history.c src/utils.c

# Output binary
TARGET = Shell

# Default rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Clean + recompile
rebuild: clean all

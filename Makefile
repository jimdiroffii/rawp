CC = gcc
CFLAGS = -Wall -Wextra -g -I./src # -I./src allows #include "header.h"
LDFLAGS =
TARGET = tcp_sender_macos

SRC_DIR = src
BUILD_DIR = build

# Find all .c files in SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Replace src/ with build/ and .c with .o for object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Rule to link the program
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(@D) # Ensure build directory for the target exists
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Linking complete: $@"

# Rule to compile .c files into .o files
# $< is the first prerequisite (the .c file)
# $@ is the target (the .o file)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D) # Ensure build directory for .o files exists
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled: $< -> $@"

# Clean target
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

.PHONY: all clean
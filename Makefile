CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/socket_program

$(shell mkdir -p $(BUILD_DIR))

$(TARGET): $(SRC_DIR)/socket.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: run
run: $(TARGET)
	./$(TARGET)
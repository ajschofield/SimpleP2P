CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC
LDFLAGS = -shared

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

SRCS = $(SRC_DIR)/socket.c $(SRC_DIR)/peer.c $(SRC_DIR)/discovery.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/lib_simplep2p.so

$(shell mkdir -p $(BUILD_DIR))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -I$(INCLUDE_DIR) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)



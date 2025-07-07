CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

# Directories
SRC_DIR = src
INC_DIR = include
EXAMPLES_DIR = examples
TESTS_DIR = tests
BIN_DIR = binaries

# Source files
SOURCES = $(SRC_DIR)/stft.c $(SRC_DIR)/kiss_fft.c
HEADERS = $(INC_DIR)/stft.h $(SRC_DIR)/kiss_fft.h

# Targets
.PHONY: all clean examples tests

all: examples

examples: $(BIN_DIR)/stft_example $(BIN_DIR)/example

$(BIN_DIR)/stft_example: $(EXAMPLES_DIR)/stft_example.c $(SOURCES)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/example: $(EXAMPLES_DIR)/example.c $(SOURCES)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

tests: $(BIN_DIR)/test_stft

$(BIN_DIR)/test_stft: $(TESTS_DIR)/test_stft.c $(SOURCES)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)/*

install:
	cp $(INC_DIR)/stft.h /usr/local/include/
	cp $(SRC_DIR)/libstft.so /usr/local/lib/ 2>/dev/null || true

.PHONY: run-example
run-example: $(BIN_DIR)/stft_example
	cd $(BIN_DIR) && ./stft_example
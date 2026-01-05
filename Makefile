# Melvin: Emergent Intelligence System
# Makefile for building all components

CC = gcc
CFLAGS = -O3 -march=native -mtune=native -flto -Wall -Wextra
LDFLAGS = -lm -lpthread

# Source directory
SRC_DIR = src

# Object files
OBJS = $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o

# Library object (for tools)
$(SRC_DIR)/melvin_lib.o: $(SRC_DIR)/melvin.c $(SRC_DIR)/melvin.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/melvin.c -o $(SRC_DIR)/melvin_lib.o

# Targets
all: melvin_standalone

# Library version (no main)
libmelvin.a: $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o
	ar rcs libmelvin.a $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o

# Standalone melvin (with main function)
melvin_standalone: CFLAGS += -DMELVIN_STANDALONE
melvin_standalone: $(SRC_DIR)/melvin_standalone.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o
	$(CC) $(CFLAGS) -o melvin_standalone $(SRC_DIR)/melvin_standalone.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o $(LDFLAGS)

# Alias for backward compatibility
melvin: melvin_standalone

# Individual object files
$(SRC_DIR)/melvin.o: $(SRC_DIR)/melvin.c $(SRC_DIR)/melvin.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/melvin.c -o $(SRC_DIR)/melvin.o

$(SRC_DIR)/melvin_standalone.o: $(SRC_DIR)/melvin.c $(SRC_DIR)/melvin.h
	$(CC) $(CFLAGS) -DMELVIN_STANDALONE -c $(SRC_DIR)/melvin.c -o $(SRC_DIR)/melvin_standalone.o

$(SRC_DIR)/melvin_in_port.o: $(SRC_DIR)/melvin_in_port.c $(SRC_DIR)/melvin_in_port.h $(SRC_DIR)/melvin.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/melvin_in_port.c -o $(SRC_DIR)/melvin_in_port.o

$(SRC_DIR)/melvin_out_port.o: $(SRC_DIR)/melvin_out_port.c $(SRC_DIR)/melvin_out_port.h $(SRC_DIR)/melvin.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/melvin_out_port.c -o $(SRC_DIR)/melvin_out_port.o

# Tools directory
TOOLS_DIR = tools

# Tools targets
melvin_feed: $(TOOLS_DIR)/melvin_feed.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_lib.o
	$(CC) $(CFLAGS) -o melvin_feed $(TOOLS_DIR)/melvin_feed.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_lib.o $(LDFLAGS)

melvin_watch: $(TOOLS_DIR)/melvin_watch.o $(SRC_DIR)/melvin_lib.o
	$(CC) $(CFLAGS) -o melvin_watch $(TOOLS_DIR)/melvin_watch.o $(SRC_DIR)/melvin_lib.o $(LDFLAGS)

# Tool object files
$(TOOLS_DIR)/melvin_feed.o: $(TOOLS_DIR)/melvin_feed.c $(SRC_DIR)/melvin.h $(SRC_DIR)/melvin_in_port.h
	$(CC) $(CFLAGS) -c $(TOOLS_DIR)/melvin_feed.c -o $(TOOLS_DIR)/melvin_feed.o

$(TOOLS_DIR)/melvin_watch.o: $(TOOLS_DIR)/melvin_watch.c $(SRC_DIR)/melvin.h
	$(CC) $(CFLAGS) -c $(TOOLS_DIR)/melvin_watch.c -o $(TOOLS_DIR)/melvin_watch.o

# Build all tools
all-tools: melvin_feed melvin_watch

# Build everything (including tools)
all: melvin_standalone all-tools

clean:
	rm -f melvin melvin_standalone libmelvin.a $(OBJS) $(SRC_DIR)/melvin_standalone.o *.m
	rm -f melvin_feed melvin_watch $(TOOLS_DIR)/*.o

test: melvin_standalone
	@echo "Running basic test..."
	@echo "hello world" > test_input.txt
	./melvin_standalone test_input.txt test_brain.m
	@echo "Test complete."

# Test program for large datasets
test_dataset: tests/test_dataset.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o
	$(CC) $(CFLAGS) -o tests/test_dataset tests/test_dataset.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o $(LDFLAGS)

tests/test_dataset.o: tests/test_dataset.c $(SRC_DIR)/melvin.h $(SRC_DIR)/melvin_in_port.h
	$(CC) $(CFLAGS) -c tests/test_dataset.c -o tests/test_dataset.o

# Test program for hello world association learning
test_hello_world: tests/test_hello_world.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o
	$(CC) $(CFLAGS) -o tests/test_hello_world tests/test_hello_world.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o $(LDFLAGS)

tests/test_hello_world.o: tests/test_hello_world.c $(SRC_DIR)/melvin.h $(SRC_DIR)/melvin_in_port.h
	$(CC) $(CFLAGS) -c tests/test_hello_world.c -o tests/test_hello_world.o

# Test program for simple association learning
test_association_simple: tests/test_association_simple.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o
	$(CC) $(CFLAGS) -o tests/test_association_simple tests/test_association_simple.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o $(LDFLAGS)

tests/test_association_simple.o: tests/test_association_simple.c $(SRC_DIR)/melvin.h $(SRC_DIR)/melvin_in_port.h
	$(CC) $(CFLAGS) -c tests/test_association_simple.c -o tests/test_association_simple.o

# Test program for self-supervised learning
test_self_supervised: tests/test_self_supervised.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o
	$(CC) $(CFLAGS) -o tests/test_self_supervised tests/test_self_supervised.o $(SRC_DIR)/melvin.o $(SRC_DIR)/melvin_in_port.o $(SRC_DIR)/melvin_out_port.o $(LDFLAGS)

tests/test_self_supervised.o: tests/test_self_supervised.c $(SRC_DIR)/melvin.h $(SRC_DIR)/melvin_out_port.h
	$(CC) $(CFLAGS) -c tests/test_self_supervised.c -o tests/test_self_supervised.o

.PHONY: all clean test melvin_standalone test_dataset test_hello_world test_association_simple test_self_supervised


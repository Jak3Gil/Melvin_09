# Melvin: Emergent Intelligence System
# Makefile for building all components

CC = gcc
CFLAGS = -O3 -march=native -mtune=native -flto -Wall -Wextra
LDFLAGS = -lm -lpthread

# Object files
OBJS = melvin.o melvin_in_port.o melvin_out_port.o

# Targets
all: melvin_standalone

# Library version (no main)
libmelvin.a: melvin.o melvin_in_port.o melvin_out_port.o
	ar rcs libmelvin.a melvin.o melvin_in_port.o melvin_out_port.o

# Standalone melvin (with main function)
melvin_standalone: CFLAGS += -DMELVIN_STANDALONE
melvin_standalone: melvin_standalone.o melvin_in_port.o melvin_out_port.o
	$(CC) $(CFLAGS) -o melvin_standalone melvin_standalone.o melvin_in_port.o melvin_out_port.o $(LDFLAGS)

# Alias for backward compatibility
melvin: melvin_standalone

# Individual object files
melvin.o: melvin.c melvin.h
	$(CC) $(CFLAGS) -c melvin.c -o melvin.o

melvin_standalone.o: melvin.c melvin.h
	$(CC) $(CFLAGS) -DMELVIN_STANDALONE -c melvin.c -o melvin_standalone.o

melvin_in_port.o: melvin_in_port.c melvin_in_port.h melvin.h
	$(CC) $(CFLAGS) -c melvin_in_port.c -o melvin_in_port.o

melvin_out_port.o: melvin_out_port.c melvin_out_port.h melvin.h
	$(CC) $(CFLAGS) -c melvin_out_port.c -o melvin_out_port.o

clean:
	rm -f melvin melvin_standalone libmelvin.a $(OBJS) melvin_standalone.o *.m

test: melvin_standalone
	@echo "Running basic test..."
	@echo "hello world" > test_input.txt
	./melvin_standalone test_input.txt test_brain.m
	@echo "Test complete."

# Test program for large datasets
test_dataset: test_dataset.o melvin.o melvin_in_port.o melvin_out_port.o
	$(CC) $(CFLAGS) -o test_dataset test_dataset.o melvin.o melvin_in_port.o melvin_out_port.o $(LDFLAGS)

test_dataset.o: test_dataset.c melvin.h melvin_in_port.h
	$(CC) $(CFLAGS) -c test_dataset.c -o test_dataset.o

# Test program for hello world association learning
test_hello_world: test_hello_world.o melvin.o melvin_in_port.o melvin_out_port.o
	$(CC) $(CFLAGS) -o test_hello_world test_hello_world.o melvin.o melvin_in_port.o melvin_out_port.o $(LDFLAGS)

test_hello_world.o: test_hello_world.c melvin.h melvin_in_port.h
	$(CC) $(CFLAGS) -c test_hello_world.c -o test_hello_world.o

# Test program for simple association learning
test_association_simple: test_association_simple.o melvin.o melvin_in_port.o melvin_out_port.o
	$(CC) $(CFLAGS) -o test_association_simple test_association_simple.o melvin.o melvin_in_port.o melvin_out_port.o $(LDFLAGS)

test_association_simple.o: test_association_simple.c melvin.h melvin_in_port.h
	$(CC) $(CFLAGS) -c test_association_simple.c -o test_association_simple.o

# Test program for self-supervised learning
test_self_supervised: test_self_supervised.o melvin.o melvin_in_port.o melvin_out_port.o
	$(CC) $(CFLAGS) -o test_self_supervised test_self_supervised.o melvin.o melvin_in_port.o melvin_out_port.o $(LDFLAGS)

test_self_supervised.o: test_self_supervised.c melvin.h melvin_out_port.h
	$(CC) $(CFLAGS) -c test_self_supervised.c -o test_self_supervised.o

.PHONY: all clean test melvin_standalone test_dataset test_hello_world test_association_simple test_self_supervised


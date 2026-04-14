CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -D_GNU_SOURCE
TARGET = mini_supervisor
CRASHER = crasher
OBJS = main.o logging.o

.PHONY: all clean test

all: $(TARGET) $(CRASHER)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c sysmini.h logging.h
	$(CC) $(CFLAGS) -c main.c

logging.o: logging.c logging.h sysmini.h
	$(CC) $(CFLAGS) -c logging.c

$(CRASHER): crasher.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) $(CRASHER) *.log

test: all
	@echo "=== Testing crasher program ==="
	@echo "Running crasher mode 0 (should crash quickly):"
	@timeout 10s ./$(CRASHER) 0 || echo "Crasher exited as expected"
	@echo ""
	@echo "=== Ready to test supervisor ==="
	@echo "Run: ./$(TARGET)"
	@echo "Then try commands like:"
	@echo "  ./crasher 0    (crashes in 3-8 seconds)"
	@echo "  ./crasher 1    (exits with error)"
	@echo "  ./crasher 3    (long-running with rare crashes)"
	@echo "  ls -la         (normal command)"
	@echo "  status         (show current process)"
	@echo "  stop           (stop supervised process)"
	@echo "  quit           (exit supervisor)"

install: all
	@echo "Binaries built successfully!"
	@echo "- $(TARGET): The main supervisor program"
	@echo "- $(CRASHER): Test program for crashing"
	@echo ""
	@echo "Try: make test"
CC      = gcc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -pedantic
SRC     = src/sorting.c src/utils.c src/benchmark.c src/memtrack.c
LDFLAGS = -Wl,--wrap=malloc,--wrap=free

all: sorting

sorting: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_sorting: tests/test_sorting.c src/sorting.c src/utils.c
	$(CC) $(CFLAGS) -o $@ $^

test: test_sorting
	./test_sorting

run: sorting
	./sorting

full: sorting
	./sorting --full

clean:
	rm -f sorting test_sorting

.PHONY: all test run full clean

CFLAGS = -Wall -Wextra -march=native -pipe -pthread -std=c11

all: build

build: $(basename $(wildcard *.c))

debug: CFLAGS += -g
debug: build

test: build
	@find test/* -exec ./sudoku_verifier {} $(shell nproc) \; | less

clean:
	@rm $(basename $(wildcard *.c))

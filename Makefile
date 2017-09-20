SRC = $(basename $(wildcard *.c))
CFLAGS = -Wall -Wextra -march=native -pipe -pthread -std=c11 -O3

all: $(SRC)

debug: CFLAGS += -g
debug: all

test: $(addsuffix .test, $(basename $(wildcard tests/*.in)))
%.test: %.in %.out /usr/bin/cmp all
	@./$(SRC) $< $(shell nproc) | tail -1 | cmp -s $(word 2, $?) -

clean:
	@rm $(SRC)

CFLAGS = -Wall -Wextra -march=native -pipe -lpthread

all: build

build: $(basename $(wildcard *.c))

debug: CFLAGS += -g
debug: build

test: build
	@for f in test/*_grid_* ; do echo $$f ; ./sudoku_verifier $$f ; done | less

clean:
	@rm $(basename $(wildcard *.c))

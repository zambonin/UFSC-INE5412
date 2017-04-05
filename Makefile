CFLAGS = -Wall -Wextra -march=native -pipe -Os -s

all: build

build: $(basename $(wildcard *.c))

test: build
	@for f in test/*_grid_* ; do echo $$f ; ./sudoku_verifier $$f ; done | less

clean:
	@rm $(basename $(wildcard *.c))

CFLAGS = -Wall -Wextra -march=native -pipe -Os -s

all: build

build: $(basename $(wildcard *.c))

test:

clean:
	@rm $(basename $(wildcard *.c))

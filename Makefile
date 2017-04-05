CFLAGS = -Wall -Wextra -march=native -pipe -Og

all: build

build: $(basename $(wildcard *.c))

test:

clean:
	@rm $(basename $(wildcard *.c))

.POSIX:

all: fas dev block.img

fas: fas.c
	gcc $^ -o $@

dev: dev.c vm.h dbg.h io.h
	gcc $< -o $@

block.img: test.fas fas
	./fas $<

kernel: kernel.fas fas
	./fas $<

test: fas dev block.img
	./dev

clean:
	rm -f fas dev

.PHONY: clean all test kernel

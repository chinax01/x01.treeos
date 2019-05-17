
all: build

build: loader.s boot.s 
	nasm boot.s -o boot.bin
	nasm loader.s -o loader.bin 

#loader.s: loader.S
#	gcc -E loader.S > loader.s 
	
clean:
	rm -rf *.bin
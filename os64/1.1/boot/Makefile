
default: build

build:
	nasm boot.s -o boot.bin
	nasm loader.s -o loader.bin 

clean:
	rm -rf *.bin
include Makefile.header

all: Image

Image: build
	dd if=boot/boot.bin of=Image bs=512 count=1 

build:
	(cd boot; make)

clean:
	rm -rf Image 
	(cd boot; make clean)
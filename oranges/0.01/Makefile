all: run 

Image:
	nasm -f bin boot/bootsect.s -o Image
	
run: Image
	qemu-system-x86_64 Image

clean:
	rm -rf Image 
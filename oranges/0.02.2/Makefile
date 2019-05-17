all: Image

AS = nasm
AsFlags = -i include/

Image: boot/bootsect.s
	$(AS) $(AsFlags) boot/bootsect.s -o Image.com
	
copyImage:
	sudo mount -o loop b.img /mnt 
	sudo cp Image.com /mnt
	sudo umount /mnt
	
run: 
	bochs

clean:
	rm -rf Image.com
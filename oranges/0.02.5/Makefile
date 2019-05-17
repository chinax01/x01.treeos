all: Image

AS = nasm
AsFlags = -i include/

Image: boot/bootsect.s
	$(AS) $(AsFlags) boot/bootsect.s  -o Image.com
	
copy: Image
	sudo mount -o loop b.img /mnt 
	sudo cp Image.com /mnt
	sudo umount /mnt

clean:
	rm -rf Image.com
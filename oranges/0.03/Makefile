AS = nasm
AsFlags = -i include/
LD = ld 
LdFlags = -m elf_i386 
CC = gcc 
CcFlags = -c -m32

all: copy 

boot.bin: boot/bootsect.s
	$(AS) $(AsFlags) boot/bootsect.s  -o boot.bin

loader.bin: boot/loader.s 
	$(AS) $(AsFlags) boot/loader.s -o loader.bin

pm.bin: boot/pm.s 
	$(AS) $(AsFlags) boot/pm.s -o pm.bin 

copy: boot.bin loader.bin 
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt 
	sudo cp loader.bin /mnt 
	sudo umount /mnt

look:
	xxd -u -a -g 1 -c 16 -s +0x4200 -l 0x11 a.img 

clean:
	rm -rf *.bin *.o

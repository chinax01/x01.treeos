AS = nasm
AsFlags = -i include/ -f elf 
LD = ld 
LdFlags = -m elf_i386 -s 
CC = gcc 
CcFlags = -c -m32

all: copy 

boot.bin: boot/bootsect.s
	$(AS) boot/bootsect.s  -o boot.bin -i include/

loader.bin: boot/loader.s 
	$(AS) boot/loader.s -o loader.bin -i include/

kernel.bin: boot/kernel.s 
	$(AS) $(AsFlags) boot/kernel.s -o kernel.o 
	$(LD) $(LdFlags) kernel.o -o kernel.bin 
	
copy: boot.bin loader.bin kernel.bin 
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt 
	sudo cp loader.bin /mnt 
	sudo cp kernel.bin /mnt
	sudo umount /mnt

look:
	xxd -u -a -g 1 -c 16 -s +0x4200 -l 0x11 a.img 

clean:
	rm -rf *.bin *.o
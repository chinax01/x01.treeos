AS = nasm
AsFlags = -i include/ -f elf 
LD = ld 
LdFlags = -m elf_i386 -s -Ttext 0x30400
CC = gcc 
CcFlags = -c -m32 -nostdinc -I include -g -Wall -fno-builtin -fno-stack-protector 
CPP = gcc -E -m32 -nostdinc -I include

Objs = kernel/kernel.o kernel/start.o kernel/i8259.o kernel/global.o  \
	 kernel/protect.o kernel/main.o kernel/clock.o kernel/syscall.o  kernel/keyboard.o \
	 kernel/tty.o kernel/console.o kernel/printf.o kernel/proc.o kernel/systask.o \
	 kernel/hd.o fs/main.o fs/open.o fs/misc.o lib/kliba.o lib/klib.o lib/open.o lib/close.o 

.c.s:
	$(CC) $(CcFlags) -S -o $*.s $<
.s.o:
	$(AS) $(AsFlags) -o $*.o $<
.c.o:
	$(CC) $(CcFlags) -c -o $*.o $<

all: build

boot.bin: boot/bootsect.s
	$(AS) -i include/ -o $@ $<

loader.bin: boot/loader.s 
	$(AS) -i include/ -o $@ $<

kernel/kernel.o: kernel/kernel.s 
kernel/syscall.o: kernel/syscall.s 
kernel/start.o: kernel/start.c 
kernel/i8259.o: kernel/i8259.c 
kernel/global.o: kernel/global.c 
kernel/protect.o: kernel/protect.c 
kernel/main.o: kernel/main.c 
kernel/clock.o: kernel/clock.c 
kernel/keyboard.o: kernel/keyboard.c 
kernel/tty.o: kernel/tty.c 
kernel/console.o: kernel/console.c 
kernel/printf.o: kernel/printf.c 
kernel/proc.o: kernel/proc.c 
kernel/systask.o: kernel/systask.c 
kernel/hd.o: kernel/hd.c 
fs/main.o: fs/main.c 
fs/open.o: fs/open.c 
fs/misc.o: fs/misc.c 

lib/kliba.o: lib/kliba.s 
lib/klib.o: lib/klib.c 
lib/open.o: lib/open.c
lib/close.o: lib/close.c 

kernel.bin: $(Objs)
	$(LD) $(LdFlags) -o $@ $(Objs)
	
build: boot.bin loader.bin kernel.bin 
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt 
	sudo cp loader.bin /mnt 
	sudo cp kernel.bin /mnt
	sudo umount /mnt

look:
	xxd -u -a -g 1 -c 16 -s 0 -l 512 c.img 

clean:
	rm -rf $(Objs) *.bin

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in kernel/*.c;do echo -n "kernel/";$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make
	
### Dependencies:
kernel/clock.o: kernel/clock.c include/const.h include/type.h include/proto.h \
 include/global.h
kernel/global.o: kernel/global.c include/const.h include/type.h include/proto.h \
 include/global.h
kernel/i8259.o: kernel/i8259.c include/const.h include/type.h include/proto.h \
 include/global.h
kernel/keyboard.o: kernel/keyboard.c include/const.h include/type.h \
 include/proto.h include/global.h
kernel/main.o: kernel/main.c include/const.h include/type.h include/proto.h \
 include/global.h
kernel/protect.o: kernel/protect.c include/const.h include/type.h \
 include/proto.h include/global.h
kernel/start.o: kernel/start.c include/const.h include/type.h include/proto.h \
 include/global.h

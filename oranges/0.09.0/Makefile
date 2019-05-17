ENTRYPOINT	= 0x1000

AS = nasm
AsFlags = -i include/ -f elf 
LD = ld 
LdFlags = -m elf_i386 -s -Ttext $(ENTRYPOINT)
CC = gcc 
CcFlags = -c -m32 -I include -g -Wall -fno-builtin -fno-stack-protector 
CPP = gcc -E -m32 -I include

Objs = kernel/kernel.o kernel/start.o kernel/i8259.o kernel/global.o  \
	 kernel/protect.o kernel/main.o kernel/clock.o kernel/syscall.o  kernel/keyboard.o \
	 kernel/tty.o kernel/console.o kernel/printf.o kernel/proc.o kernel/systask.o \
	 kernel/hd.o fs/main.o fs/open.o fs/misc.o  fs/read_write.o fs/unlink.o \
	 mm/main.o \
	 lib/kliba.o lib/klib.o lib/open.o lib/close.o lib/read.o lib/write.o lib/unlink.o  \
	 lib/fork.o lib/getpid.o lib/wait.o lib/exit.o 

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
fs/read_write.o: fs/read_write.c 
fs/unlink.o: fs/unlink.c 

mm/main.o: mm/main.c 

lib/kliba.o: lib/kliba.s 
lib/klib.o: lib/klib.c 
lib/open.o: lib/open.c
lib/close.o: lib/close.c 
lib/read.o: lib/read.c 
lib/write.o: lib/write.c 
lib/unlink.o: lib/unlink.c 
lib/fork.o: lib/fork.c 
lib/getpid.o: lib/getpid.c 
lib/wait.o: lib/wait.c 
lib/exit.o: lib/exit.c 

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
	(for i in lib/*.c;do echo -n "lib/";$(CPP) -M $$i;done) >> tmp_make
	(for i in fs/*.c;do echo -n "fs/";$(CPP) -M $$i;done) >> tmp_make
	(for i in mm/*.c;do echo -n "mm/";$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make
	
### Dependencies:
kernel/clock.o: kernel/clock.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/console.o: kernel/console.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/global.o: kernel/global.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/hd.h include/global.h
kernel/hd.o: kernel/hd.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/hd.h
kernel/i8259.o: kernel/i8259.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/keyboard.o: kernel/keyboard.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/keyboard.h include/keymap.h
kernel/main.o: kernel/main.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/printf.o: kernel/printf.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/proc.o: kernel/proc.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/protect.o: kernel/protect.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/start.o: kernel/start.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/systask.o: kernel/systask.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
kernel/tty.o: kernel/tty.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/keyboard.h
lib/close.o: lib/close.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
lib/fork.o: lib/fork.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
lib/getpid.o: lib/getpid.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
lib/klib.o: lib/klib.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h /usr/include/elf.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
 /usr/lib/gcc/x86_64-linux-gnu/5/include/stdint.h /usr/include/stdint.h \
 /usr/include/bits/wchar.h /usr/include/bits/auxv.h
lib/open.o: lib/open.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h
lib/read.o: lib/read.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/unlink.o: lib/unlink.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/write.o: lib/write.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
fs/main.o: fs/main.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/hd.h
fs/misc.o: fs/misc.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
fs/open.o: fs/open.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
fs/read_write.o: fs/read_write.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
fs/unlink.o: fs/unlink.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
mm/main.o: mm/main.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h

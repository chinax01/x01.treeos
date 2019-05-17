ENTRYPOINT	= 0x1000

AS = nasm
AsFlags = -i include/ -f elf 
LD = ld 
LdFlags = -m elf_i386 -s -Ttext $(ENTRYPOINT)
CC = gcc 
CcFlags = -c -m32 -I include -g -Wall -fno-builtin -fno-stack-protector 
CPP = gcc -E -m32 -I include

Objs = kernel/kernel.o kernel/start.o kernel/i8259.o kernel/global.o  \
	 kernel/protect.o kernel/main.o kernel/clock.o  kernel/keyboard.o \
	 kernel/tty.o kernel/console.o kernel/proc.o kernel/systask.o kernel/hd.o \
	 kernel/kliba.o kernel/klib.o \
	 fs/main.o fs/open.o fs/misc.o  fs/read_write.o fs/unlink.o \
	 mm/main.o mm/exec.o 
LibObjs = lib/open.o lib/close.o lib/read.o lib/write.o lib/unlink.o  \
	 lib/fork.o lib/getpid.o lib/wait.o lib/exit.o lib/syscall.o  \
	 lib/misc.o lib/string.o lib/printf.o  lib/exec.o  lib/stat.o 
CommandObjs = command/echo.o command/start.o 

.c.s:
	$(CC) $(CcFlags) -S -o $*.s $<
.s.o:
	$(AS) $(AsFlags) -o $*.o $<
.c.o:
	$(CC) $(CcFlags) -c -o $*.o $<

all: clean build crt echo inst 

build: boot.bin loader.bin kernel.bin 
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt 
	sudo cp loader.bin /mnt 
	sudo cp kernel.bin /mnt
	sudo umount /mnt

boot.bin: boot/bootsect.s
	$(AS) -i include/ -o $@ $<

loader.bin: boot/loader.s 
	$(AS) -i include/ -o $@ $<

kernel.bin: $(Objs) $(LibObjs)
	$(LD) $(LdFlags) -o $@ $(Objs) $(LibObjs)

kernel/kernel.o: kernel/kernel.s 
kernel/start.o: kernel/start.c 
kernel/i8259.o: kernel/i8259.c 
kernel/global.o: kernel/global.c 
kernel/protect.o: kernel/protect.c 
kernel/main.o: kernel/main.c 
kernel/clock.o: kernel/clock.c 
kernel/keyboard.o: kernel/keyboard.c 
kernel/tty.o: kernel/tty.c 
kernel/console.o: kernel/console.c 
kernel/proc.o: kernel/proc.c 
kernel/systask.o: kernel/systask.c 
kernel/hd.o: kernel/hd.c 
kernel/kliba.o: kernel/kliba.s 
kernel/klib.o: kernel/klib.c 

fs/main.o: fs/main.c 
fs/open.o: fs/open.c 
fs/misc.o: fs/misc.c 
fs/read_write.o: fs/read_write.c 
fs/unlink.o: fs/unlink.c 

mm/main.o: mm/main.c 
mm/exec.o: mm/exec.c 

lib/syscall.o: lib/syscall.s 
lib/string.o: lib/string.s 
lib/open.o: lib/open.c
lib/close.o: lib/close.c 
lib/read.o: lib/read.c 
lib/write.o: lib/write.c 
lib/unlink.o: lib/unlink.c 
lib/fork.o: lib/fork.c 
lib/getpid.o: lib/getpid.c 
lib/wait.o: lib/wait.c 
lib/exit.o: lib/exit.c 
lib/misc.o: lib/misc.c 
lib/printf.o: lib/printf.c 
lib/exec.o: lib/exec.c 
lib/stat.o: lib/stat.c 

command/echo.o: command/echo.c 
command/start.o: command/start.s 

show:
	egrep -e '^ROOT_BASE' include/load.inc | sed -e 's/.*0x//g'
	egrep -e '#define[[:space:]]*INSTALL_START_SECT' include/config.h | sed -e 's/.*0x//g'
	ls -l inst.tar | awk -F " " '{print $$5}'

inst:
	tar vcf inst.tar kernel.bin echo
	dd if=inst.tar of=c.img seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' include/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l inst.tar | awk -F " " '{print $$5}'` conv=notrunc

crt: $(LibObjs)
	 ar rcs lib/crt.a $(LibObjs)

echo: $(CommandObjs)
	$(LD) $(LdFlags)  -o echo $(CommandObjs) lib/crt.a 

look:
	xxd -u -a -g 1 -c 16 -s 0 -l 512 c.img 

clean:
	rm -rf $(Objs) $(LibObjs) $(CommandObjs) *.bin lib/crt.a *.tar

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
 include/global.h include/stdio.h
kernel/console.o: kernel/console.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/global.o: kernel/global.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/hd.h include/global.h include/stdio.h
kernel/hd.o: kernel/hd.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h include/hd.h
kernel/i8259.o: kernel/i8259.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/keyboard.o: kernel/keyboard.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h include/keyboard.h include/keymap.h
kernel/klib.o: kernel/klib.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h /usr/include/elf.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-32.h \
 /usr/lib/gcc/x86_64-linux-gnu/5/include/stdint.h /usr/include/stdint.h \
 /usr/include/bits/wchar.h /usr/include/bits/auxv.h
kernel/main.o: kernel/main.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/proc.o: kernel/proc.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/protect.o: kernel/protect.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/start.o: kernel/start.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/systask.o: kernel/systask.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
kernel/tty.o: kernel/tty.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h include/keyboard.h
lib/close.o: lib/close.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/exec.o: lib/exec.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/exit.o: lib/exit.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/fork.o: lib/fork.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/getpid.o: lib/getpid.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/misc.o: lib/misc.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/open.o: lib/open.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/printf.o: lib/printf.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/read.o: lib/read.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/stat.o: lib/stat.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/unlink.o: lib/unlink.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/wait.o: lib/wait.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
lib/write.o: lib/write.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h
fs/main.o: fs/main.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h include/hd.h
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
mm/exec.o: mm/exec.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h /usr/include/elf.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-32.h \
 /usr/lib/gcc/x86_64-linux-gnu/5/include/stdint.h /usr/include/stdint.h \
 /usr/include/bits/wchar.h /usr/include/bits/auxv.h
mm/main.o: mm/main.c /usr/include/stdc-predef.h include/const.h \
 include/config.h include/type.h include/proto.h include/fs.h \
 include/global.h include/stdio.h

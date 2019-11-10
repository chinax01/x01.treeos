include Makefile.header

LdFlags += -Ttext 0 -e start_32
CFlags += -I../include

ARCHIVES = kernel/kernel.o fs/fs.o 
DRIVERS =
MATH =
LIBS = lib/lib.a 

all: Image

Image: build kernel.bin 
	dd if=boot/boot.bin of=Image bs=512 count=1 
	dd if=kernel.bin of=Image bs=512 seek=1

kernel.bin: boot/head.o init/main.o $(ARCHIVES) $(DRIVERS) $(MATH) $(LIBS) 
	$(LD) $(LdFlags) -o $@ $^
	cp -f $@ kernel.tmp 
	$(STRIP) kernel.tmp
	$(OBJCOPY) -O binary -R .note -R .comment kernel.tmp $@ 
	rm -f kernel.tmp 

build:
	(cd boot; make)
	(cd init; make)
	(cd kernel; make)
	(cd fs; make)
	(cd lib; make)

clean:
	rm -rf Image kernel.bin 
	(cd boot; make clean)
	(cd init; make clean)
	(cd kernel; make clean)
	(cd fs; make clean)
	(cd lib; make clean)
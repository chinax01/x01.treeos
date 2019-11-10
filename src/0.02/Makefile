include Makefile.header

LdFlags += -Ttext 0 -e start_32
CFlags += -I../include

ARCHIVES = kernel/kernel.o 
DRIVERS =
MATH =
LIBS =

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

clean:
	rm -rf Image 
	(cd boot; make clean)
	(cd init; make clean)
	(cd kernel; make clean)
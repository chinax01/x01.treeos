include Makefile.header

RAMDISK = #-DRAMDISK=512

LDFLAGS += -Ttext 0 -e startup_32
CFLAGS += $(RAMDISK) 

ROOT_DEV = 0x0301
SWAP_DEV = 0x0304

ARCHIVES = 
DRIVERS =
MATH =
LIBS =
DIRS = boot 

.s.o:
	$(CC) $(CFLAGS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

all: clean Image

Image: mkdirs system
	dd if=boot/bootsect bs=512 count=1 of=Image
	dd if=boot/setup seek=1 bs=512 count=4 of=Image
	dd if=system seek=5 bs=512  of=Image
	sync

mkdirs:
	@for i in $(DIRS); do make -C $$i; done

system: boot/head.o #init/main.o $(ARCHIVES) $(DRIVERS) $(MATH) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^
	cp -f $@ kernel.tmp 
	$(STRIP) kernel.tmp
	$(OBJCOPY) -O binary -R .note -R .comment kernel.tmp $@ 
	rm -f kernel.tmp 

clean:
	@rm -f Image system
	@for i in $(DIRS); do make clean -C $$i; done
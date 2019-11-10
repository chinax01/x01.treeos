include ../Makefile.header

LdFlags += -Ttext 0 -e _start

build: boot.bin head.o 

boot.s: boot.S 
	$(CPP) -traditional boot.S -o boot.s 

boot.bin: boot.s
	$(AS) boot.s -o boot.o 
	$(LD) $(LdFlags) boot.o -o $@
	objcopy -R .pdr -R .comment -R .note -S -O binary $@
	
head.s: head.S 
	$(CPP) -traditional -o $@ $<
	
head.o:	head.s 
	$(AS) -o $@ $<

clean: 
	rm -rf *.o boot.bin *.s 

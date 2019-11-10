include ../Makefile.header

LdFlags += -Ttext 0 -e _start

all: boot.bin

boot.s: boot.S 
	$(CPP) -traditional boot.S -o boot.s 

boot.bin: boot.s
	$(AS) boot.s -o boot.o 
	$(LD) $(LdFlags) boot.o -o $@
	objcopy -R .pdr -R .comment -R .note -S -O binary $@
	
clean: 
	rm -rf *.o boot.bin *.s 

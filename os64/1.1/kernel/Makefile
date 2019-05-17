build: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system:	head.o main.o 
	ld -b elf64-x86-64 -o system head.o main.o -T kernel.lds 

main.o:	main.c
	gcc  -mcmodel=large -fno-builtin -m64 -c main.c

head.o:	head.S
	gcc -E  head.S > head.s
	as --64 -o head.o head.s

clean:
	rm -rf *.o *.s system kernel.bin 
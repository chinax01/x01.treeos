CC_FLAGS = -mcmodel=large -fno-builtin -fno-stack-protector -m64 -c

build: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system: head.o entry.o main.o printk.o trap.o memory.o interrupt.o task.o
	ld -b elf64-x86-64 -z muldefs -o system head.o entry.o main.o printk.o trap.o memory.o interrupt.o task.o -T Kernel.lds 

head.o:	head.S
	gcc -E  head.S > head.s
	as --64 -o head.o head.s

entry.o: entry.S
	gcc -E  entry.S > entry.s
	as --64 -o entry.o entry.s
	
main.o:	main.c
	gcc  $(CC_FLAGS) main.c
	
printk.o: printk.c
	gcc  $(CC_FLAGS) printk.c

trap.o: trap.c
	gcc  $(CC_FLAGS) trap.c

memory.o: memory.c
	gcc  $(CC_FLAGS) memory.c

interrupt.o: interrupt.c
	gcc  $(CC_FLAGS) interrupt.c
	
task.o: task.c
	gcc  $(CC_FLAGS) task.c	

clean:
	rm -rf *.o system kernel.bin head.s 
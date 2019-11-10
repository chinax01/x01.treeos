include ../Makefile.header

LdFlags += -r
CFlags += -finline-functions -I../include
CPP += -I../include

.c.s:
	$(CC) $(CFlags) -S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFlags) -c -o $*.o $<

OBJS = sched.o mktime.o tty_io.o console.o keyboard.o shell.o serial.o \
	traps.o printk.o asm.o exit.o  vsprintf.o system_call.o  \
	fork.o hd.o rs_io.o 

build: kernel.o 

kernel.o: $(OBJS)
	$(LD) $(LdFlags) -o $@ $(OBJS)
	sync

keyboard.s: keyboard.S 
	$(CPP) -traditional keyboard.S -o keyboard.s 

asm.s: asm.S 
	$(CPP) -traditional asm.S -o asm.s 

system_call.s: system_call.S 
	$(CPP) -traditional -o $@ $<

rs_io.s: rs_io.S 
	$(CPP) -traditional -o $@ $<

clean:
	rm -rf *.o *.s 
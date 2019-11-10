include ../Makefile.header

LdFlags += -r
CFlags += -I../include
CPP += -I../include

.c.s:
	$(CC) $(CFlags) -S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFlags) -c -o $*.o $<

OBJS = sched.o 

build: kernel.o 

kernel.o: $(OBJS)
	$(LD) $(LdFlags) -o $@ $(OBJS)
	sync

clean:
	rm -rf *.o *.s 
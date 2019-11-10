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

OBJS = ctype.o errno.o  write.o _exit.o close.o dup.o  \
	execve.o setsid.o string.o 

build: lib.a

lib.a: $(OBJS)
	$(AR) rcs lib.a $(OBJS)
	sync

clean:
	rm -rf *.o *.s *.a 
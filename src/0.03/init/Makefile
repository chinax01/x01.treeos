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

build: main.o 

clean:
	rm -rf *.o *.s 
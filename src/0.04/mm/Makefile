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

OBJS = memory.o page.o 

build: mm.o 

mm.o: $(OBJS)
	$(LD) $(LdFlags) -o $@ $(OBJS)
	sync

#system_call.s: system_call.S 
#	$(CPP) -traditional -o $@ $<

clean:
	rm -rf *.o 
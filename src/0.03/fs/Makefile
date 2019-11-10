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

OBJS = exec.o buffer.o 

build: fs.o 

fs.o: $(OBJS)
	$(LD) $(LdFlags) -o $@ $(OBJS)
	sync

#system_call.s: system_call.S 
#	$(CPP) -traditional -o $@ $<

clean:
	rm -rf *.o *.s 
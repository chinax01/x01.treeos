include ../Makefile.header

LDFLAGS += -Ttext 0

DelObjs = bootsect bootsect.o head.o 

.s.o:
	$(CC) $(CFLAGS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

all: bootsect head.o 

bootsect: bootsect.o
	$(LD) $(LDFLAGS) -o $@ $<
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $@

bootsect.o: bootsect.s 
head.o: head.s 

clean:
	@rm -f $(DelObjs)
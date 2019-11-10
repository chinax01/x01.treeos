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

OBJS = exec.o buffer.o inode.o super.o  block_dev.o  bitmap.o truncate.o file_table.o  \
		namei.o  file_dev.o pipe.o char_dev.o read_write.o  \
		open.o stat.o fcntl.o ioctl.o tty_ioctl.o 

build: fs.o 

fs.o: $(OBJS)
	$(LD) $(LdFlags) -o $@ $(OBJS)
	sync

clean:
	rm -rf $(OBJS) fs.o 
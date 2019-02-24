include ../Makefile.header

CFLAGS += -I../include
CPP += -I../include

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

main.o: main.c 

clean:
	rm -f main.o

dep:
	@sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	@(for i in *.c;do echo -n "init/";$(CPP) -M $$i;done) >> tmp_make
	@cp tmp_make Makefile
	@rm -f tmp_make

### Dependencies:
init/main.o: main.c ../include/unistd.h ../include/sys/stat.h \
 ../include/sys/types.h ../include/sys/time.h ../include/time.h \
 ../include/sys/times.h ../include/sys/utsname.h ../include/sys/param.h \
 ../include/sys/resource.h ../include/utime.h ../include/stddef.h \
 ../include/stdarg.h ../include/fcntl.h ../include/string.h \
 ../include/linux/tty.h ../include/termios.h ../include/linux/sched.h \
 ../include/linux/head.h ../include/linux/fs.h ../include/linux/mm.h \
 ../include/linux/kernel.h ../include/signal.h ../include/asm/system.h \
 ../include/asm/io.h

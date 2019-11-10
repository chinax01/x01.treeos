# 相关目录需作相应调整
SubDir = 1.1

all: build #disk copyall

build:
	(cd $(SubDir)/boot; make)
	(cd $(SubDir)/kernel; make)
	#(cd $(SubDir)/user; make)

disk:
	dd if=$(SubDir)/boot/boot.bin of=a.img bs=512 count=1 conv=notrunc 

copyall:
	sudo mount a.img /media/ -t vfat -o loop
	sudo cp $(SubDir)/boot/loader.bin  /media/
	sudo cp $(SubDir)/kernel/kernel.bin /media/
	#sudo cp $(SubDir)/user/init.bin /media/
	sync
	sudo umount /media/

clean:
	(cd $(SubDir)/boot; make clean)
	(cd $(SubDir)/kernel; make clean)
	#(cd $(SubDir)/user; make clean)

dif:
	diff -rwyp $(SubDir)/kernel/head.S 5/kernel/head.S > diff.txt 

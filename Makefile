#MODULE="dmp"
obj-m += dmp.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install: all
	sudo insmod dmp.ko
	
setup:
	sudo dmsetup create zero1  --table "0 512 zero"
	sudo dmsetup create dmproxy --table "0 512 dmp /dev/mapper/zero1 0"
	ls -al /dev/mapper/*

unload:
	sudo dmsetup remove dmproxy
	sudo dmsetup remove zero1
	sudo rmmod dmp.ko
		
test: clean install setup
	sudo dd if=/dev/random of=/dev/mapper/dmproxy bs=4k count=1
	sudo dd of=/dev/null if=/dev/mapper/dmproxy bs=4k count=1
	cat /sys/module/dmproxy/stat/volumes
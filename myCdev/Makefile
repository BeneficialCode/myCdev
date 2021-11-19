obj-m = myCdev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	rm *.ko *.symvers *.order *.mod.c *.o

# Got help from corey.b: https://forums.raspberrypi.com/viewtopic.php?t=151806
CR_C := arm-linux-gnueabihf-
MODULES := /home/tristan/Repos/raspberrypi-linux/
obj-m += helloworld.o

all:
	make ARCH=arm CROSS_COMPILE=$(CR_C) -C $(MODULES) M=$(shell pwd) modules

clean:
	make ARCH=arm CROSS_COMPILE=$(CR_C) -C $(MODULES) M=$(shell pwd) clean

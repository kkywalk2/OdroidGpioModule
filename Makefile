KDIR = /root/odroid/linux
PWD = $(shell pwd)
obj-m = gpioled_module.o
CROSS = /opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

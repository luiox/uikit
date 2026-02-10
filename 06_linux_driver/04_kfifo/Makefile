KERNELDIR := /home/canrad/linux/imx6ull/kernel
CURRENT_PATH := $(shell pwd)

ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc

obj-m := kfifo_demo.o

build: module

module:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

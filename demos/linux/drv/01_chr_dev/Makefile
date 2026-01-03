KERNELDIR := /home/canrad/linux/imx6ull/kernel
CURRENT_PATH := $(shell pwd)

ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc

obj-m := drv_src/drv.o

# Final output directory
OUT := $(CURRENT_PATH)/bin

all: module app

module:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	@mkdir -p $(OUT)
	@if [ -f drv_src/drv.ko ]; then cp drv_src/drv.ko $(OUT); fi

app:
	@mkdir -p $(OUT)
	$(CC) app_src/app.c -o $(OUT)/app

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -rf $(OUT)/*
	rm -f ledApp

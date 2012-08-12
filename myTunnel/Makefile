KERNELDIR := /home/carelife/android_icecream/android_kernel/CyanogenMod
PWD :=$(shell pwd)
ARCH=arm
CROSS_COMPILE=arm-eabi-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
obj-m := public4over6.o
modules:
	$(MAKE) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules  
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean



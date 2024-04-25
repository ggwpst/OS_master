MODULE_NAME = kfetch_mod_312552047
ifneq ($(KERNELRELEASE),)
	obj-m := $(MODULE_NAME).o
else
	PWD:=$(shell pwd)
	KVERSION := $(shell uname -r)
	KERNEL_DIR:=/lib/modules/$(KVERSION)/build

all: 
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean: 
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
load:
	sudo insmod ./$(MODULE_NAME).ko
unload:
	sudo rmmod ./$(MODULE_NAME).ko
endif



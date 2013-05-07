KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := cpc.o
cpc-objs := cpc_mod.o cpc_rt.o cpc_queue.o cpc_netlink.o

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp cpc.ko ../

.PHONY:
	clean

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

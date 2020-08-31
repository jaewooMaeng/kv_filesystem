KDIR = /lib/modules/$(shell uname -r)/build

obj-m := testing.o

all : 
	$(MAKE) -C $(KDIR) SUBDIRS=$(shell pwd) modules

clean : 
	$(MAKE) -C $(KDIR) SUBDIRS=$(shell pwd) modules

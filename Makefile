KDIR = /lib/modules/$(shell uname -r)/build

obj-m := testing.o

all : 
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules;

clean : 
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean;

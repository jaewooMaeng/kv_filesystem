TARGET = kfs

# Specify here your kernel dir:
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# To implement some macros declaration after satement is required
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

obj-m += $(TARGET).o

kfs-objs := kvfs.o super.o inode.o dir.o file.o

default:
	make -C $(KDIR) SUBDIRS=$(shell pwd) modules

clean:
	make -C $(KDIR) SUBDIRS=$(shell pwd) clean

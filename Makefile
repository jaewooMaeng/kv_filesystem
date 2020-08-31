TARGET = kfs

# Specify here your kernel dir:
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# To implement some macros declaration after satement is required
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

obj-m := kfs.o

kfs-objs += kvfs.o
kfs-objs += super.o
kfs-objs += inode.o
kfs-objs += dir.o
kfs-objs += file.o

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean

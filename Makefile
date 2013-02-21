.PHONY: all test clean tools

all: test update_image

update_image: kernel initrd
	@echo Updating floppy.img
	@./update.sh

test: 
	@+make -C test

tools:
	@+make -C tools

kernel:
	@+make -C src

clean:
	@+make -C tools clean
	@+make -C test clean
	@+make -C src clean

initrd: tools initrd_index
	@tools/initrd_build initrd_index


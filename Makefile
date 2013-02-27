.PHONY: all test clean tools

all: release

release: BUILD_TYPE = release
release: test update_image

debug: BUILD_TYPE = debug
debug: test update_image

update_image: kernel initrd
	@echo Updating floppy.img
	@./scripts/update.sh

test: 
	@+make -C test

tools:
	@+make -C tools

kernel:
	@+make -C src $(BUILD_TYPE)

clean:
	@+make -C tools clean
	@+make -C test clean
	@+make -C src clean

initrd: tools initrd_index
	@tools/initrd_build initrd_index


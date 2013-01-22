OBJDIR=obj

CC = g++
AS = nasm
LD = ld

CC_SOURCES := $(wildcard *.cc) $(wildcard */*.cc)
CC_SOURCES := $(filter-out $(wildcard tools/*.cc), $(CC_SOURCES))
CC_SOURCES := $(filter-out $(wildcard test/*.cc), $(CC_SOURCES))
AS_SOURCES := $(wildcard *.s) $(wildcard */*.s)
AS_SOURCES := $(filter-out $(wildcard tools/*.s), $(AS_SOURCES))
CC_OBJECTS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.o))
AS_OBJECTS := $(addprefix $(OBJDIR)/,$(AS_SOURCES:.s=.o))
OBJECTS := $(AS_OBJECTS) $(CC_OBJECTS)

INITRD_BUILD = tools/initrd_build

EXECUTABLE = kernel
MODULES = initrd

CXXFLAGS = -Wall -Werror -Wextra -O0 -iquote. \
					 -nostdlib -nodefaultlibs \
					 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
				   -m32 -g
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf

all: test update_image 

update_image: $(EXECUTABLE) $(MODULES) 
	@echo Updating floppy.img
	@./update.sh

test:
	@make -C test

clean:
	-rm $(OBJECTS) $(EXECUTABLE)
	@make -C tools clean
	@make -C test clean

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

initrd: $(INITRD_BUILD) initrd_index
	$(INITRD_BUILD) initrd_index

$(INITRD_BUILD):
	@make -C tools initrd_build

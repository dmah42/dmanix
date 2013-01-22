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

CC_DEPS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.d))

EXECUTABLE = kernel
MODULES = initrd
INITRD_BUILD = tools/initrd_build

CXXFLAGS = -Wall -Werror -Wextra -O0 -iquote. \
					 -nostdlib -nodefaultlibs \
					 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
				   -m32 -g
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf

.PHONY: all test clean tools

.PRECIOUS: $(OBJDIR)/%.d

all: test update_image

update_image: $(EXECUTABLE) $(MODULES)
	@echo Updating floppy.img
	@./update.sh

test: 
	@+make -C test

tools:
	@+make -C tools

clean:
	-rm -f $(CC_DEPS) $(OBJECTS) $(EXECUTABLE)
	@+make -C tools clean
	@+make -C test clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean)))
	-include $(CC_DEPS)
endif
#include $(CC_DEPS)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.d: %.cc
	$(CC) $(CCFLAGS) -MF$@ -MG -MM -MP -MT$(addprefix $(OBJDIR)/,$(<:.cc=.o)) $<

$(OBJDIR)/%.o: %.cc $(OBJDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

initrd: tools initrd_index
	$(INITRD_BUILD) initrd_index



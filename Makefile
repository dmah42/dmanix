OBJDIR=obj

CC = g++
AS = nasm
LD = ld

# TODO: Use wildcard for dir, exclude tools.
CC_SOURCES := $(wildcard *.cc) \
							$(wildcard fs/*.cc) \
							$(wildcard interrupt/*.cc) \
							$(wildcard memory/*.cc)
AS_SOURCES := $(wildcard *.s) \
							$(wildcard fs/*.s) \
							$(wildcard interrupt/*.s) \
							$(wildcard memory/*.s)
CC_OBJECTS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.o))
AS_OBJECTS := $(addprefix $(OBJDIR)/,$(AS_SOURCES:.s=.o))
OBJECTS := $(AS_OBJECTS) $(CC_OBJECTS)

INITRD_BUILD = tools/initrd_build

EXECUTABLE = kernel
MODULES = initrd

CXXFLAGS = -Wall -Werror -Wextra -O0 -I. \
					 -nostdlib -nodefaultlibs \
					 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
				   -m32 -g
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf

all: $(EXECUTABLE) $(MODULES)

clean:
	-rm $(OBJECTS) $(EXECUTABLE)
	@make -C tools clean

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

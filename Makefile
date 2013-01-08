OBJDIR=obj

CC = g++
AS = nasm
LD = ld

CC_SOURCES := $(wildcard *.cc)
AS_SOURCES := $(wildcard *.s)
CC_OBJECTS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.o))
AS_OBJECTS := $(addprefix $(OBJDIR)/,$(AS_SOURCES:.s=.o))
OBJECTS := $(AS_OBJECTS) $(CC_OBJECTS)

EXECUTABLE = kernel

CXXFLAGS=-Wall -Werror -Wno-main -Os \
				 -nostdlib -nostdinc \
				 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
				 -m32 
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf

all: $(EXECUTABLE)

clean:
	-rm $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.cc
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

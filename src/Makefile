CC = g++
AS = nasm
LD = ld

CXXFLAGS = -Wall -Werror -Wextra -iquote. -iquote../common \
		-nostdlib -nodefaultlibs \
		-fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
		-m32

LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf

# TODO: add debug/release to OBJDIR
OBJDIR=../obj

CC_SOURCES := $(wildcard *.cc) $(wildcard */*.cc)
AS_SOURCES := $(wildcard *.s) $(wildcard */*.s)
CC_OBJECTS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.o))
AS_OBJECTS := $(addprefix $(OBJDIR)/,$(AS_SOURCES:.s=.o))
OBJECTS := $(AS_OBJECTS) $(CC_OBJECTS)

CC_DEPS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.d))

EXECUTABLE = ../kernel

.PHONY: all clean check_mode release debug

.PRECIOUS: $(OBJDIR)/%.d

all: release

release: CXXFLAGS += -DNDEBUG -O3
release: $(EXECUTABLE)

debug: CXXFLAGS += -DDEBUG -O0 -g
debug: $(EXECUTABLE)

clean:
	-rm -f $(CC_DEPS) $(OBJECTS) $(EXECUTABLE)

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
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -MF$@ -MG -MM -MP -MT$(addprefix $(OBJDIR)/,$(<:.cc=.o)) $<

$(OBJDIR)/%.o: %.cc $(OBJDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

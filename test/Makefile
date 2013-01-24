OBJDIR = obj

CC = g++
AR = ar

CC_SOURCES := $(wildcard *.cc) ../bitset.cc ../memory/memory.cc
CC_OBJECTS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.o))
OBJECTS := $(AS_OBJECTS) $(CC_OBJECTS)
CC_DEPS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.d))

GTEST_DIR = ../third_party/gtest-1.6.0

CXXFLAGS = -Wall -O0 \
					 -iquote.. -iquote$(GTEST_DIR)/include \
					 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
				   -g
EXECUTABLE = test_all
GTEST_LIB = libgtest.a

.PHONY: all clean 

.PRECIOUS: $(OBJDIR)/%.d

all: $(EXECUTABLE)

clean:
	@-rm -f $(CC_DEPS) $(OBJECTS) $(OBJDIR)/gtest-all.o $(GTEST_LIB) $(EXECUTABLE)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean)))
	-include $(CC_DEPS)
endif
#include $(CC_DEPS)

$(GTEST_LIB): | $(OBJDIR)
	$(CC) -I$(GTEST_DIR)/include -I$(GTEST_DIR) -c $(GTEST_DIR)/src/gtest-all.cc -o $(OBJDIR)/gtest-all.o
	$(AR) -rv $(GTEST_LIB) $(OBJDIR)/gtest-all.o

$(EXECUTABLE): $(OBJECTS) $(GTEST_LIB)
	$(CC) $(CXXFLAGS) $(OBJECTS) $(GTEST_LIB) $(GTEST_DIR)/src/gtest_main.cc -o $(EXECUTABLE) -lpthread

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.d: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -MF$@ -MG -MM -MP -MT$(addprefix $(OBJDIR)/,$(<:.cc=.o)) $<

$(OBJDIR)/%.o: %.cc $(OBJDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@


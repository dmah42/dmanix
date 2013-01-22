OBJDIR = obj

CC = g++
AR = ar

CC_SOURCES := $(wildcard *.cc)
CC_OBJECTS := $(addprefix $(OBJDIR)/,$(CC_SOURCES:.cc=.o))
OBJECTS := $(AS_OBJECTS) $(CC_OBJECTS)

GTEST_DIR = ../third_party/gtest-1.6.0

CXXFLAGS = -Wall -O0 \
					 -iquote.. -iquote$(GTEST_DIR)/include \
					 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti \
				   -g
EXECUTABLE = test_all
GTEST_LIB = libgtest.a

all: $(EXECUTABLE)

clean:
	@-rm -f $(OBJECTS) $(OBJDIR)/gtest-all.o $(GTEST_LIB) $(EXECUTABLE)

$(GTEST_LIB): | $(OBJDIR)
	$(CC) -I$(GTEST_DIR)/include -I$(GTEST_DIR) -c $(GTEST_DIR)/src/gtest-all.cc -o $(OBJDIR)/gtest-all.o
	$(AR) -rv $(GTEST_LIB) $(OBJDIR)/gtest-all.o

$(EXECUTABLE): $(OBJECTS) $(GTEST_LIB)
	$(CC) $(CXXFLAGS) $(OBJECTS) $(GTEST_LIB) $(GTEST_DIR)/src/gtest_main.cc -o $(EXECUTABLE) -lpthread

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@


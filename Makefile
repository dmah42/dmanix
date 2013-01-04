SOURCES=base.o boot.o dt.o dt_flush.o heap.o interrupt.o io.o irq.o isr.o kalloc.o main.o paging.o screen.o timer.o

CXXFLAGS=-Wall -Werror -Wno-main -Os -nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32 -fno-exceptions -fno-rtti
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf

all: $(SOURCES) link

clean:
	-rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<

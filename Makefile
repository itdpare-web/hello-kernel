CC      := gcc
AS      := as
LD      := ld

CFLAGS  := -m32 -std=c11 -ffreestanding -fno-pic -fno-builtin \
           -fno-stack-protector -fno-asynchronous-unwind-tables \
           -nostdlib -nostdinc -Wall -Wextra -Werror -O2
ASFLAGS := --32
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib --nmagic

OBJS    := boot.o kmain.o console.o interrupts.o keyboard.o

.PHONY: all clean run

all: kernel.bin

boot.o: boot.s
	$(AS) $(ASFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

kernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin -serial stdio -display none

clean:
	rm -f $(OBJS) kernel.bin screen.ppm

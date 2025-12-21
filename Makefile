PREFIX ?= i686-elf-
CC := $(PREFIX)gcc
AS := $(PREFIX)as
LD := $(PREFIX)ld

CFLAGS := -ffreestanding -O2 -Wall -Wextra -std=c11 -m32 -fno-pic -fno-stack-protector -nostdlib -nostartfiles -nodefaultlibs
LDFLAGS := -T linker.ld -m elf_i386

OBJ := \
  kernel/multiboot.o \
  kernel/kernel.o \
  kernel/vga.o \
  kernel/kbd.o \
  kernel/shell.o \
  kernel/idt.o \
  kernel/timer.o \
  kernel/isr.o \
  kernel/gdt.o \
  kernel/paging.o \
  kernel/memory.o \
  kernel/pmm.o \
  kernel/heap.o \
  kernel/isr_stubs.o \
  kernel/gdt_flush.o \
  kernel/tty.o \
  kernel/log.o \
  kernel/panic.o \
  kernel/syscall.o \
  kernel/tss.o \
  kernel/tss_flush.o \
  kernel/string.o \
  kernel/enter_user_mode.o

all: os.iso

kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/%.o: kernel/%.asm
	nasm -f elf32 $< -o $@

kernel/multiboot.o: kernel/multiboot.S
	$(CC) -m32 -c $< -o $@

os.iso: kernel.elf grub.cfg
	mkdir -p isodir/boot/grub
	cp kernel.elf isodir/boot/kernel.elf
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o os.iso isodir >/dev/null 2>&1 || grub2-mkrescue -o os.iso isodir

run: os.iso
	qemu-system-i386 -cdrom os.iso

clean:
	rm -rf isodir *.o kernel.elf os.iso kernel/*.o

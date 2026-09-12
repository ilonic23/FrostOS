C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c power/*.c drivers/display/*.c multiboot/*.c drivers/storage/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h power/*.h drivers/display/*.h multiboot/*.h drivers/storage/*.h)
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o drivers/cpuid-detect.o}
CC = i386-elf-gcc
GDB = gdb
CFLAGS = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32

kernel.elf: boot/multiboot_entry.o ${OBJ}
	i386-elf-ld -o $@ -T linker.ld $^

# kernel.bin: boot/kernel_entry.o ${OBJ}
# 	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary
#
# os-image.bin: temp-image.bin
# 	dd if=/dev/zero of=$@ bs=1M count=4
# 	dd if=temp-image.bin of=$@ conv=notrunc
# 	dd if=/dev/zero of=hdd.img bs=1M count=4
#
# temp-image.bin: boot/bootsect.bin kernel.bin
# 	cat $^ > temp-image.bin

FrostOS.iso: kernel.elf
	mkdir -p ./iso/boot/grub/
	cp ./grub.cfg ./iso/boot/grub/grub.cfg
	cp ./kernel.elf ./iso/boot/kernel.bin
	grub-mkrescue -o FrostOS.iso iso/

iso: FrostOS.iso

disk:
	dd if=/dev/zero of=hdd.img bs=1M count=10

run: FrostOS.iso
	qemu-system-i386 -m 512M -boot once=d -cdrom FrostOS.iso -drive file=hdd.img,format=raw,index=0,media=disk,if=ide -audiodev pa,id=speaker -machine pcspk-audiodev=speaker -enable-kvm -cpu host -vga qxl

debug: CFLAGS += -DKERNEL_DEBUG -fno-omit-frame-pointer
debug: FrostOS.iso
	qemu-system-i386 -m 512M -boot once=d -cdrom FrostOS.iso -drive file=hdd.img,format=raw,index=0,media=disk,if=ide -audiodev pa,id=speaker -machine pcspk-audiodev=speaker -enable-kvm -cpu host -vga qxl -debugcon stdio

# debug: os-image.bin kernel.elf
# 	qemu-system-i386 -S -s -m 512M -fda os-image.bin -hda hdd.img -audiodev pa,id=speaker -machine pcspk-audiodev=speaker -d guest_errors,int &
# 	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

check: ${C_SOURCES}
	${CC} -ffreestanding -fsyntax-only -Wall -Wextra -m32 -c $^

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o libc/*.o
	rm -rf iso/

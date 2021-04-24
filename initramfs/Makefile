SRCS 		= $(wildcard ./code/*.c)
OBJS		= $(SRCS:%.c=%.o)

PROGRAM		= argv_test
ELF			= ./code/$(PROGRAM).elf
IMG			= $(PROGRAM).img

CPIO		= initramfs.cpio
CPIO_FILE	= model.py test.txt $(IMG)

CFLAGS 		= -Wall -O2 -ffreestanding -nostdlib
CC 			= aarch64-linux-gnu-gcc
LINKER 		= aarch64-linux-gnu-ld
OBJ_COPY 	= aarch64-linux-gnu-objcopy

.PHONY: 	clean

all:		$(CPIO)

$(CPIO):	$(IMG)
	find $(CPIO_FILE) | cpio -o -H newc > ./$(CPIO)
	mv ./$(CPIO) ../ 

$(IMG):		$(OBJS)
	$(LINKER) -nostdlib $(OBJS) -T ./code/link.ld -o $(ELF)
	$(OBJ_COPY) -O binary $(ELF) $@

code/%.o: 	code/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJS) $(ELF) $(IMG) >/dev/null 2>/dev/null || true
	
# NCTU Operating Capstone 2021

## LAB 0

## Author

| Student ID | GitHub Account | Name  | Email                       |
| -----------| -------------- | ----- | --------------------------- |
| 309551054  | Brothre23      | 于兆良 | daveyu824.cs09g@nctu.edu.tw |

## Files

| File          | Content      |  
| --------------| ------------ |  
| LAB-00.ld     | linker script|
| LAB-00.s      | source code  |

### From Source Code to Object Files

```bash
aarch64-linux-gnu-gcc -c LAB-00.s -o LAB-00.o
```

### From Object Files to ELF

```bash
aarch64-linux-gnu-ld -T LAB-00.ld -o kernel8.elf LAB-00.o
```

### From ELF to Kernel Image

```bash
aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img
```

## Check on QEMU

```bash
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm
```

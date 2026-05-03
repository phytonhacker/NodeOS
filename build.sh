#!/usr/bin/env bash
set -e

echo "==> Embedding JS files..."
python3 embed_js.py

echo "==> Assembling boot.asm..."
nasm -f elf32 boot.asm -o boot.o

echo "==> Compiling kernel.c..."
gcc -m32 -ffreestanding -fno-stack-protector \
    -fno-pie -fno-pic \
    -nostdlib -nostdinc \
    -c kernel.c -o kernel.o

echo "==> Compiling js/lexer.c..."
gcc -m32 -ffreestanding -fno-stack-protector \
    -fno-pie -fno-pic \
    -nostdlib -nostdinc \
    -c js/lexer.c -o lexer.o

echo "==> Linking..."
ld -m elf_i386 -T linker.ld -o iso/boot/myos.bin boot.o kernel.o lexer.o

echo "==> Building ISO..."
grub-mkrescue -o myos.iso iso/

echo "==> Done! Run with:"
echo "    qemu-system-x86_64 -cdrom myos.iso -nographic -serial stdio"
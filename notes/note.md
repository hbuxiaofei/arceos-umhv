
- seabios 编译
```bash
cat > .config << EOF
# for qemu machine types 2.0 + newer
CONFIG_QEMU=y
CONFIG_ROM_SIZE=256
CONFIG_ATA_DMA=n
EOF
echo "CONFIG_DEBUG_LEVEL=9" >> .config

make PYTHON=python3 oldnoconfig
make

```

- seabios 反汇编
```bash
objdump -D -b binary -m i8086 bios.bin
objdump -D -b binary -m i8086 romlayout.o
```

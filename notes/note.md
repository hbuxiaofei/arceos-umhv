- Pull crates to local workspace

[https://github.com/Starry-OS/Starry](https://github.com/Starry-OS/Starry)

```bash
# To download the tool
$ cargo install kbuild
$ mkdir crates
$ kbuild patch add axstarry
$ kbuild patch remove axstarry
$ kbuild patch list
```

- seabios 编译
```bash
cat > .config << EOF
# for qemu machine types 2.0 + newer
CONFIG_QEMU=y
CONFIG_ROM_SIZE=256
CONFIG_ATA_DMA=n

CONFIG_XEN=n

CONFIG_DEBUG_LEVEL=9
CONFIG_DEBUG_SERIAL=y
EOF
echo "CONFIG_DEBUG_LEVEL=9" >> .config

make PYTHON=python3 oldnoconfig
make

```

- seabios 反汇编
```bash
objdump -D -b binary -m i8086 bios.bin
objdump -D -b binary -m i8086 romlayout.o

-M intel : 指定intel格式

```

- TODO
```
1. 支持设置primary vcpu第一条指令地址 entry_point.
   说明: 可能还需要考虑设置代码段 CS 寄存器

2. 需要让seabios认为是在KVM环境下, 即 strcmp(signature, "KVMKVMKVM") == 0.
   目前情况: Found hypervisor signature "RVMRVMRVMRVM" at 40000000

3. 支持通过fw_cfg, 通过 QEMU_CFG_FILE_DIR 读取 e820 信息.

其他 ...


```

---

参考文档:
- [SeaBIOS实现简单分析](https://www.cnblogs.com/gnuemacs/p/14287120.html)
- [浅度剖析 SeaBIOS 之 QEMU 初始化](https://zhuanlan.zhihu.com/p/678576761])
- <<Qemu/kvm源码解析与应用>> - 李强


- Pull crates to local workspace

    参考: [https://github.com/Starry-OS/Starry](https://github.com/Starry-OS/Starry)

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

- Port IO

所谓端口Port IO, x86上使用in out指令进行访问, 和内存的地址空间完全隔离.(ARM上没有PIO)
Guest以Linux为例: cat /proc/ioports查看当前OS的所有的ioports :

```
  0000-0cf7 : PCI Bus 0000:00
  0000-001f : dma1
  0020-0021 : pic1
  0040-0043 : timer0
  0050-0053 : timer1
  0060-0060 : keyboard
  0064-0064 : keyboard
  0070-0077 : rtc0
  0080-008f : dma page reg
  00a0-00a1 : pic2
  00c0-00df : dma2
  00f0-00ff : fpu
  03c0-03df : vga+
  03f8-03ff : serial
  0510-051b : QEMU0002:00
    0510-051b : fw_cfg_io
  0600-067f : 0000:00:1f.0
    0600-0603 : ACPI PM1a_EVT_BLK
    0604-0605 : ACPI PM1a_CNT_BLK
    0608-060b : ACPI PM_TMR
    0620-062f : ACPI GPE0_BLK
    0630-0633 : iTCO_wdt.0.auto
      0630-0633 : iTCO_wdt
    0660-067f : iTCO_wdt.0.auto
      0660-067f : iTCO_wdt
  0700-073f : 0000:00:1f.3
    0700-073f : i801_smbus
0cf8-0cff : PCI conf1
0d00-ffff : PCI Bus 0000:00
  1000-1fff : PCI Bus 0000:01
  2000-2fff : PCI Bus 0000:02
  3000-3fff : PCI Bus 0000:03
  4000-4fff : PCI Bus 0000:04
  5000-5fff : PCI Bus 0000:05
  6000-6fff : PCI Bus 0000:06
  7000-7fff : PCI Bus 0000:07
  c040-c05f : 0000:00:1f.2
    c040-c05f : ahci
```

常见的port 40---timer, 60---keyboard等等. 这个是否可以变呢, 可以的, 只是基于业界使用习惯, 都会相对固定使用常用的ioport.



**Todo List**

- [x] 1. seabios第一条指令地址为: 0xf000:0xe05b, 支持设置primary vcpu第一条指令地址 entry_point.


    ```
    1. 目前实模式下还不支设置超过0xffff的地址
    2. 考虑设置代码段 CS 寄存器
    ```

- [x] 2. 设置虚拟化需要截获的io端口

    ```
    有些端口需要进行截获, 否则会透传到宿主机, 获取宿主机的信息, 例如pci信息, 内存大小信息等
    ```

- [ ] 3. dma 实现支持

    ```
    很多数据的传输需要通过 dma 传输
    ```


- [ ] 4. 实现fw_cfg设备模拟


    - [x] fw_cfg 实现 pio, 设备地址 [0x510, 0x511]
    ```
    告诉seabios, 虚拟化环境为 “QEMU”

    ```

    - [ ] fw_cfg 实现 dma, 设备地址 [0x514]
    ```
    用于传输数据, 例如内核data数据等
    ```

- [x] 5. 实现rtc设备模拟, 设备地址 [0x70, 0x71]


    ```
    在虚拟化环境中, seabios 通过 rtc 几个保留的寄存器获取内存大小信息
    ```

- [ ] 6. multiboot 实现

    ```
    seabios通过内核启动是通过multiboot协议启动的, 需要将内核文件进行重新封装
    ```


其他 ...


---

参考文档:
- [SeaBIOS实现简单分析](https://www.cnblogs.com/gnuemacs/p/14287120.html)
- [浅度剖析 SeaBIOS 之 QEMU 初始化](https://zhuanlan.zhihu.com/p/678576761])
- <<Qemu/kvm源码解析与应用>> - 李强


【handle_post => dopost => maininit => device_hardware_setup
    => usb_setup/ps2port_setup/lpt_setup/serial_setup/floppy_setup/
       ata_setup/ahci_setup/sdcard_setup/ramdisk_setup/virtio_blk_setup/
       virtio_scsi_setup/lsi_scsi_setup/esp_scsi_setup/megasas_setup/pvscsi_setup】

【handle_post => dopost => maininit => prepareboot => bcv_prepboot => map_hd_drive & add_bev】

【handle_post => dopost => maininit => startBoot
    => handle_19 => do_boot => boot_disk
    => handle_13 => handle_legacy_disk
        => disk_13 => [disk_1302 / disk_1303 / disk_1304 => basic_access] /
                       extended_access / disk_1300 / disk_1305 / disk_1310
        => send_disk_op => process_op => scsi_process_op => cdb_read / cdb_write => cdb_cmd_data => ...】

//Entry point for Power On Self Test (POST) - the BIOS initilization phase.
//This function makes the memory at 0xc0000-0xfffff read/writable and then calls dopost().
handle_post
	|--> make_bios_writable
	|--> dopost
			|--> qemu_preinit
					|--> qemu_detect
							|--> u16 v = pci_config_readw(0, PCI_VENDOR_ID)							【check northbridge(北桥) @ 00:00.0】
									|--> outl(0x80000000 | (bdf << 8) | (addr & 0xfc), PORT_PCI_CMD)【0x80000000】
									|--> return inw(PORT_PCI_DATA + (addr & 2))
							|--> u16 d = pci_config_readw(0, PCI_DEVICE_ID)							【i440fx or q35】
							|--> u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID)				【Red Hat, Inc】
							|--> u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID)						【Qemu virtual machine】
							|--> if (sv != 0x1af4 || sd != 0x1100) 									【Red Hat, Inc || Qemu virtual machine】
							|--> 	return
							|--> PlatformRunningOn |= PF_QEMU
						(*)	|--> kvm_detect
									|--> cpuid(KVM_CPUID_SIGNATURE, &eax, &ebx, &ecx, &edx)
									|--> memcpy(signature + 0, &ebx, 4)
									|--> memcpy(signature + 4, &ecx, 4)
									|--> memcpy(signature + 8, &edx, 4)
									|--> signature[12] = 0
									|--> if (strcmp(signature, "KVMKVMKVM") == 0)
									|--> 	dprintf(1, "Running on KVM\n")
									|--> 	PlatformRunningOn |= PF_KVM
					【get memory size from nvram】
					|--> u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_LOW) << 16) | (rtc_read(CMOS_MEM_EXTMEM2_HIGH) << 24))
					|--> rs = (((rtc_read(CMOS_MEM_EXTMEM_LOW) << 10) | (rtc_read(CMOS_MEM_EXTMEM_HIGH) << 18)) + 1 * 1024 * 1024)
				(*)	|--> add_e820(0, rs, E820_RAM)
					|--> add_e820(0xfffc0000, 256*1024, E820_RESERVED)								【reserve 256KB BIOS area at the end of 4 GB】
			|--> maininit
					|--> interface_init
							|--> qemu_cfg_init														【from qemu: fw_cfg_init1】【判断QEMU_CFG_SIGNATURE == "qemu"】
									|--> qemu_cfg_select(QEMU_CFG_SIGNATURE)
										|--> outw(f, PORT_QEMU_CFG_CTL)								【寄存器: 0x0510】
									|--> char *sig = "QEMU"
									|--> for (i = 0; i < 4; i++)
									|--> 	if (inb(PORT_QEMU_CFG_DATA) != sig[i])					【寄存器: 0x0511】
									|--> 		return ;
								(*)	|--> qemu_cfg_legacy											【qemu_cfg_read_entry: NUMA/ACPI/SMBIOS info】
											|-->
											|-->
											|-->
											|-->
											|-->
											|-->
									|--> qemu_cfg_read_entry										【QEMU_CFG_FILE_DIR】
								(*)	|--> qemu_cfg_read												【读取qemu中fw_cfg内容】
								(*)	|--> qemu_cfg_e820												【e820内存区域类型表】
											|--> add_e820
											|--> ...
							|--> coreboot_cbfs_init
						(*)	|--> ivt_init															【中断向量表】
						(*)	|--> bda_init															【bios数据区】
							|--> boot_init
									|--> loadBootOrder												【初始化Bootorder[i]数组, 启动顺序配置】
							|--> bios32_init
							|--> pmm_init
							|--> pnp_init
							|--> kbd_init															【键盘】
							|--> mouse_init															【鼠标】
				(*)	|--> platform_hardware_setup
							|--> setcr0
							|--> dma_setup															【CONFIG_ATA_DMA=y开启dma模式,ghost方式安装windows时,使用dma模式比较快,否则使用pio模式会很慢】
							|--> pic_setup															【i8259初始化】
							|--> mathcp_setup
							|--> timer_setup 														【时间定时服务, msleep/mdelay等可在此之后调用,方便调试】
									|--> cpuid(0, &eax, &ebx, &ecx, &edx)
									|--> cpuid(1, &eax, &ebx, &ecx, &cpuid_features)
									|--> tsctimer_setup												【rdtsc: TimerPort/TimerKHz】
							|--> clock_setup 														【时钟&中断服务】
									|--> pit_setup
									|--> rtc_setup
									|--> rtc_updating
									|--> SET_BDA(timer_counter, ticks % TICKS_PER_DAY)				【bios数据区:timer_counter】
									|--> rtc_read													【Century】
									|--> enable_hwirq(0,...)										【timer interrupt】
									|--> enable_hwirq(8,...)										【rtc中断】
							|--> qemu_platform_setup												【】
								【xen】
									|--> runningOnXen
									|--> pci_probe_devices
									|--> xen_hypercall_setup
									|--> xen_biostable_setup
								【kvm】
								(*)	|--> pci_setup													【《学习笔记--pci(seabios).txt》】
											|--> pci_probe_host										【PCI bus & bridge init】
											|--> pci_bios_init_bus
										(*)	|--> pci_probe_devices									【PCI device probing: 生成pci_device结构,加入PCIDevices链表】
											|--> pci_bios_init_platform								【i440fx & q35_mch】
										(*)	|--> pci_bios_check_devices								【】
										(*)	|--> pci_bios_map_devices								【Map regions on each device】
											|--> pci_bios_init_devices								【map the interrupt】
											|--> pci_enable_default_vga
									|--> smm_device_setup
									|--> smm_setup
									|--> mtrr_setup
								(*)	|--> smp_setup													【lapic & SIPI】
											|--> cpuid(1, &eax, &ebx, &ecx, &cpuid_features)
											|--> writel(APIC_SVR, val | APIC_ENABLED)				【enable local APIC】
											|--> writel(APIC_LINT0, 0x8700)							【Set LINT0 as Ext_INT, level triggered】
											|--> writel(APIC_LINT1, 0x8400)							【Set LINT1 as NMI, level triggered】
											|--> writel(APIC_ICR_LOW, 0x000C4600 | sipi_vector)		【broadcast SIPI】
									|--> pirtable_setup
									|--> mptable_setup
								(*)	|--> smbios_setup												【生成smbios信息,可以用“鲁大师”等软件查看,manufacturer/product/vendor等可在此修改】
										(*)	|--> smbios_romfile_setup								【使用qemu生成的smbios】【《学习笔记--dmidecode.txt》】
										(*)	|--> smbios_legacy_setup								【seabios生成smbios】【《学习笔记--fw_cfg.txt》】
									|--> find_acpi_rsdp
								(*)	|--> acpi_setup													【windows OEM激活会用到apic的RSDT表和SLIC表的oem_id和oem_table_id字段】
				(*)	|--> device_hardware_setup
						(*)	|--> usb_setup															【如果要从usb中系统启动,usb映射需要在此之前完成】
									|--> run_thread(__usb_setup, NULL);
											|--> xhci_setup
													|--> xhci_controller_setup
															|--> run_thread(configure_xhci, xhci)
											|--> ehci_setup
													|--> ehci_controller_setup
															|--> run_thread(configure_ehci, cntl)
											|--> uhci_setup
													|--> uhci_controller_setup
															|--> run_thread(configure_uhci, cntl)
											|--> ohci_setup
													|--> ohci_controller_setup
															|--> run_thread(configure_ohci, cntl)
							|--> ps2port_setup
									|--> enable_hwirq(1,...)
									|--> enable_hwirq(12,...)
									|--> run_thread(ps2_keyboard_setup, NULL);
							|--> lpt_setup
						(*)	|--> serial_setup														【串口, 打印服务】
							|--> floppy_setup														【软盘】
									|--> addFloppy
											|--> boot_add_floppy									【生成BootList链表】
									|--> enable_hwirq(6, FUNC16(entry_0e))
						(*)	|--> ata_setup															【IDE磁盘】
									|--> ata_scan
											|--> pci_init_device(pci_ata_tbl, pci, NULL)
													|--> found_compatibleahci
															|--> init_pciata
																	|--> init_controller
																			|--> run_thread(ata_detect, chan_gf)
																					|--> init_drive_atapi
																							|--> bootprio_find_ata_device
																									|--> find_prio	【Bootorder[i]数组决定启动顺序】
																							|--> boot_add_cd		【生成BootList链表】
																					|--> init_drive_ata
																							|--> bootprio_find_ata_device
																									|--> find_prio	【Bootorder[i]数组决定启动顺序】
																							|--> boot_add_hd		【生成BootList链表】
						(*)	|--> ahci_setup															【ahci控制器】
									|--> ahci_scan
											|--> ahci_controller_setup
													|--> ahci_port_alloc
													|--> run_thread(ahci_port_detect, port)
															|--> ahci_port_setup
																	|--> ......
																	|--> bootprio_find_ata_device
																			|--> find_prio			【Bootorder[i]数组决定启动顺序】
															|--> ahci_port_realloc
															|--> boot_add_hd/boot_add_cd			【生成BootList链表】
							|--> sdcard_setup														【sd卡】
							|--> cbfs_payload_setup
							|--> ramdisk_setup
						(*)	|--> virtio_blk_setup													【virtio_blk磁盘】
									|--> init_virtio_blk
											|--> vp_find_vq
											|--> boot_add_hd										【生成BootList链表】
						(*)	|--> virtio_scsi_setup													【virtio_scsi磁盘】
									|--> init_virtio_scsi
											|--> vp_find_vq
											|--> virtio_scsi_scan_target
													|--> virtio_scsi_add_lun
															|--> scsi_drive_setup
																	|--> boot_add_hd				【生成BootList链表】
							|--> lsi_scsi_setup														【scsi磁盘】
							|--> esp_scsi_setup
							|--> megasas_setup														【RAID磁盘】
							|--> pvscsi_setup
				(*)	|--> vgarom_setup																【在vgarom_setup之后,才可以printf调试信息到控制台上,之前只能通过串口输出调试信息】
							|--> ScreenAndDebug														【printf调试信息打到控制台开关】
							|--> vgahook_setup
							|--> init_pcirom
							|--> enable_vga_console
									|--> call16_int10(&br)											【Enable VGA text mode】
					|--> optionrom_setup
					|--> interactive_bootmenu														【启动交互,选择启动盘】
							|--> enable_bootsplash													【bootsplash.jpg/bootsplash.bmp】
					(*)		|--> get_keystroke														【Get key press, 同时有超时时间设置】
							|--> disable_bootsplash
						【控制台显示】
							|--> SeaBIOS version 1.8.2
							|--> Press F12 for boot menu.
							|--> 磁盘列表
							|--> Select boot device:
							|--> Booting from Hard Disk...
					|--> wait_threads
					|--> prepareboot
						(*)	|--> bcv_prepboot														【遍历BootList链表--由boot_add_hd/boot_add_cd/boot_add_floppy...生成BootList链表】
									|--> ......
									|--> add_bev													【初始化BEV数组--决定系统启动顺序】
					|--> make_bios_readonly
					|--> startBoot																	【Invoke int 19 to start boot process】
						(*)	|--> call16_int(0x19, &br)
									|
	---------------------------------
	|
handle_19
	|--> do_boot
			|--> boot_fail	【"No bootable device."】
			|--> boot_disk(0x00, CheckFloppySig)
		(*)	|--> boot_disk(0x80, 1)
				(*)	|--> call16_int(0x13, &br)
					|--> call_boot_entry
							|--> farcall16(&br)
		(*)	|--> boot_cdrom
					|--> cdrom_boot
							|--> scsi_process_op
									|--> cdb_read
									|--> cdb_write
									(*)	|--> cdb_cmd_data
												|--> atapi_cmd_data
														|--> send_cmd
														|--> ata_wait_data
														|--> ata_pio_transfer
												|--> usb_cmd_data
												|--> uas_cmd_data
											(*)	|--> virtio_scsi_cmd_data
														|--> virtio_scsi_cmd						【virtio协议】
																|--> vring_add_buf
																|--> vring_kick
																|--> vring_more_used
																|--> vring_get_buf
																|--> vp_get_isr
												|--> lsi_scsi_cmd_data
												|--> esp_scsi_cmd_data
												|--> megasas_cmd_data
												|--> pvscsi_cmd_data
												|--> ahci_cmd_data
														|--> ahci_command
					|--> call_boot_entry
							|--> farcall16(&br)
			|--> boot_cbfs
			|--> boot_rom
			|--> call16_int(0x18, &br)
					|
	-----------------
	|
handle_18
	|--> do_boot
			|--> ......

handle_13
	|--> disk_13
			|--> disk_1302/disk_1303/disk_1304 ...
					|--> basic_access
							|--> send_disk_op
									|--> __send_disk_op
											|--> process_op
													|--> process_floppy_op
												(*)	|--> process_ata_op
															|--> ata_readwrite
																	|--> ata_pio_cmd_data
																	|--> ata_dma_cmd_data
													|--> process_ramdisk_op
													|--> process_cdemu_op
												(*)	|--> process_virtio_blk_op
															|--> virtio_blk_op						【virtio协议】
																	|--> vring_add_buf
																	|--> vring_kick
																	|--> vring_more_used
																	|--> vring_get_buf
																	|--> vp_get_isr
													|--> process_atapi_op
												(*)	|--> scsi_process_op
															|--> cdb_read
															|--> cdb_write
															(*)	|--> cdb_cmd_data
																		|--> atapi_cmd_data
																				|--> send_cmd
																				|--> ata_wait_data
																				|--> ata_pio_transfer
																		|--> usb_cmd_data
																		|--> uas_cmd_data
																	(*)	|--> virtio_scsi_cmd_data
																				|--> virtio_scsi_cmd【virtio协议】
																						|--> vring_add_buf
																						|--> vring_kick
																						|--> vring_more_used
																						|--> vring_get_buf
																						|--> vp_get_isr
																		|--> lsi_scsi_cmd_data
																		|--> esp_scsi_cmd_data
																		|--> megasas_cmd_data
																		|--> pvscsi_cmd_data
																		|--> ahci_cmd_data
																				|--> ahci_command
													|--> ahci/sdcard ...

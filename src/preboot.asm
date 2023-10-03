; this ljmp instruction at 0xeffb is required by the ISP bootloader
; and will be placed at the 0x0000 during the write process
	.area	CSEG (ABS)
	.org    0xeffb
__startup_ljmp_for_isp:
    ljmp	__sdcc_gsinit_startup
	.db	0
	.db	0
; stop before 0xf000
; this is where the bootloader would start

platforms += {
    'sh68f90' : {
        'dir' : meson.current_source_dir(),
        'src' : files(
            'clock.c',
            'diag_info.c',
            'usbhw.c',
            'peripherals.c',
            'power.c',
            'extint.c',
            'systick.c',
            'nvm.c',
            'pwm.c',
        ),
        'flash_size' : 65536,
        'bootloader_size' : 4096,
        'sector_size' : 512,
        'xram_size' : 4096,
        'freq_sys' : 24000000,
        'settings_store' : 'flash',
    },
}

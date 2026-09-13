platforms += {
    'sh68f89' : {
        'dir' : meson.current_source_dir(),
        'src' : files(
            'clock.c',
            'diag_info.c',
            'usbhw.c',
            'peripherals.c',
            'power.c',
            'extint.c',
            'systick.c',
            'pca.c',
        ),
        'flash_size'      : 65536,
        'bootloader_size' : 4096,
        'sector_size'     : 1024,
        'xram_size'       : 2816,
        'freq_sys'        : 24000000,
        'settings_store'  : 'external',
    },
}

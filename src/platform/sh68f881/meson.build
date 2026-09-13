platforms += {
    'sh68f881' : {
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
        ),
        'flash_size' : 32768,
        'bootloader_size' : 4096,
        'sector_size' : 1024,
        'xram_size' : 2304,
        'freq_sys' : 12000000,
        'settings_store' : 'eeprom',
    },
}

src_sh68f89 = files(
    'clock.c',
    'diag_info.c',
    'usbhw.c',
    'peripherals.c',
    'power.c',
    'extint.c',
    'systick.c',
    'pca.c',
)

# boards here keep their settings off-chip, so nothing links nvm.c; that leaves diag as the
# SSP engine's only caller, and only in a debug build
if get_option('buildtype') == 'debug'
    src_sh68f89 += src_flash
endif

platforms += {
    'sh68f89' : {
        'dir' : meson.current_source_dir(),
        'src' : src_sh68f89,
        'flash_size' : 65536,
        'bootloader_size' : 4096,
        'sector_size' : 1024,
        'xram_size' : 2816,
        'freq_sys' : 24000000,
        'settings_store' : 'external',
    },
}

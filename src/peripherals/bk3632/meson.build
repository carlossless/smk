wireless_modules += {
    'bk3632' : {
        'src'  : src_bb_spi + files('rf_controller.c'),
        'inc'  : [meson.current_source_dir(), inc_peripherals],
        'args' : ['-DRF_ENABLED=1'],
    },
}

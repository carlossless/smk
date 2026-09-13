# bb_spi.c is the BK3632's transport, not a general bit-banged master: it drives the module's
# MOT and CS lines by name and waits on its ACK, so it belongs here rather than in the library.
wireless_modules += {
    'bk3632' : {
        'src' : files('bb_spi.c', 'rf_controller.c'),
        'inc' : [meson.current_source_dir(), inc_peripherals],
        'args' : ['-DRF_ENABLED=1'],
    },
}

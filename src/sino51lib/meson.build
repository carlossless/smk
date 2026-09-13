inc_base += meson.current_source_dir()

src_bb_spi = files('bb_spi.c')
src_bb_i2c = files('bb_i2c.c')

# on-chip bus blocks, on pins the part fixes; a board adds these to its own 'src'
src_spi = files('spi.c')
src_twi = files('twi.c')

src_libc += files(
    'startup.c',
    'delay.c',
    'uart.c',
    'isp.c',
    'ldo.c',
    'reset.c',
    'diag.c',
    'interrupts.c',
)

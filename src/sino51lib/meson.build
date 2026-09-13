inc_base += meson.current_source_dir()

src_bb_spi = files('bb_spi.c')
src_bb_i2c = files('bb_i2c.c')

# on-chip bus blocks, on pins the part fixes; a board adds these to its own 'src'
src_spi = files('spi.c')
src_twi = files('twi.c')

# the LED timers: the driver is here, the register list is the part's *_hw.h
src_pwm = files('pwm.c')
src_pca = files('pca.c')

# the on-chip flash controller; the part links it, for diag and for the store built on top of
# it in src/nvm/flash
src_flash = files('flash.c')

src_startup = files('startup.c')
src_delay = files('delay.c')
src_uart = files('uart.c')
src_ldo = files('ldo.c')
src_reset = files('reset.c')

src_libc_files = ['startup.c', 'delay.c', 'uart.c']

if get_option('isp').enabled()
    src_libc_files += 'isp.c'
endif

src_libc_files += ['ldo.c', 'reset.c', 'diag.c', 'interrupts.c']
src_libc += files(src_libc_files)

# the parts: the register map, and the half of each driver that is not shareable
subdir('sh68f90')
subdir('sh68f89')
subdir('sh68f881')

subdir('examples')

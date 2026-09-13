# Standalone firmware that uses the library and nothing else, one per peripheral. They are
# what says the library can be picked up on its own: each lists exactly the modules it needs,
# so anything that quietly drags in the rest of the tree shows up here as a link error.
#
# 'parts' names the parts an example is built for; 'src' is its closure, and 'core' is added
# from the part for the bring-up every example does.

sino51lib_examples = [
    {
        'name' : 'gpio_blink',
        'parts' : ['sh68f90', 'sh68f89', 'sh68f881'],
        'src' : files('gpio_blink/main.c') + src_startup + src_delay + src_ldo + src_reset,
        'args' : [],
    },
    {
        'name' : 'uart_hello',
        'parts' : ['sh68f90', 'sh68f89', 'sh68f881'],
        'src' : files('uart_hello/main.c') + src_startup + src_delay + src_ldo + src_uart,
        'args' : ['-DDEBUG_SINK_UART=1'],
    },
    {
        'name' : 'spi_xfer',
        'parts' : ['sh68f90', 'sh68f89', 'sh68f881'],
        'src' : files('spi_xfer/main.c') + src_startup + src_delay + src_ldo + src_spi,
        'args' : ['-I' + meson.current_source_dir() / 'spi_xfer'],
    },
    {
        'name' : 'twi_probe',
        'parts' : ['sh68f89'],
        'src' : files('twi_probe/main.c') + src_startup + src_delay + src_ldo + src_twi,
        'args' : ['-I' + meson.current_source_dir() / 'twi_probe'],
    },
    {
        'name' : 'pwm_duty',
        'parts' : ['sh68f90'],
        'src' : files('pwm_duty/main.c') + src_startup + src_delay + src_ldo,
        'args' : [],
    },
    {
        'name' : 'pca_duty',
        'parts' : ['sh68f89'],
        'src' : files('pca_duty/main.c') + src_startup + src_delay + src_ldo + src_pca,
        'args' : [],
    },
    {
        'name' : 'flash_info',
        'parts' : ['sh68f90', 'sh68f89', 'sh68f881'],
        'src' : files('flash_info/main.c') + src_startup + src_delay + src_ldo + src_flash,
        'args' : [],
    },
]

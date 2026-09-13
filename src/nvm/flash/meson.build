# The store the parts offer themselves: a reserved program-flash sector, or the EEPROM-like
# data block where there is one. Which of the two, and where, is the platform's nvm_hw.h.
# flash.c comes from the platform, which links it for diag as well.
nvm_modules += {
    'flash' : {
        'src' : files('nvm.c'),
        'inc' : [meson.current_source_dir()],
        'args' : [],
    },
}

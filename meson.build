project(
    'smk',
    'c',
    version: 'alpha',
    meson_version: '>=1.1.0'
)

fs = import('fs')

env = environment()
env.set('DEBUG', '1')

default_options = {
    'freq_sys': '24000000', # 24Mhz
    'watchdog_enable': '1',
    'vendor_id': '0x258a',
    'product_id': '0xdead',
}

parts = [
    # keyboard, platform, layouts, options
    ['example', 'sh68f90a', ['default'], {}],
    ['nuphy-air60', 'sh68f90a', ['default'], { 'vendor_id': '0x05ac', 'product_id': '0x024f', 'wireless': 'bk3632' }],
    ['eyooso-z11', 'sh68f90a', ['default'], { 'vendor_id': '0x258a', 'product_id': '0x002a' }],
    ['redragon-k630-norgb', 'sh68f90a', ['default'], { 'vendor_id': '0x258a', 'product_id': '0x002a' }],
]

# SMK

inc_base_dirs = [
    '.',
    'src',
    'src/smk',
    'src/plaform',
    'src/kb',
    'src/user',
    'src/keyboards',
]

src_smk = [
    'src/main.c',
    'src/smk/host.c',
    'src/smk/matrix.c',
    'src/smk/report.c',
    'src/smk/keyboard.c',
]

# USER DEFAULTS

src_user = [
    'src/user/user_init.c',
    'src/user/layout_process_record.c',
    'src/user/indicators_start.c',
    'src/user/indicators_pre_update.c',
    'src/user/indicators_update_step.c',
    'src/user/indicators_post_update.c',

    'src/kb/kb_init.c',
    'src/kb/kb_process_record.c',
    'src/kb/kb_update_switches.c',
    'src/kb/kb_update.c',
]

# PLATFORM

inc_platform_sh68f90a = [
    'src/platform/sh68f90a',
]

src_platform_sh68f90a = [
    'src/platform/sh68f90a/clock.c',
    'src/platform/sh68f90a/delay.c',
    'src/platform/sh68f90a/isp.c',
    'src/platform/sh68f90a/ldo.c',
    'src/platform/sh68f90a/pwm.c',
    'src/platform/sh68f90a/uart.c',
    'src/platform/sh68f90a/usb.c',
]

inc_platform_bk3632 = [
    'src/platform/bk3632',
    'src/platform', # for bb_spi
]

src_platform_bk3632 = [
    'src/platform/bb_spi.c',
    'src/platform/bk3632/rf_controller.c',
]

possible_src_keyboard = [
    'user_matrix.c',
    'user_init.c',
    # 'user_keyboard.c',
    'kb.c'
]

possible_src_layout = [
    'indicators.c',
    'layout.c',
]

cc_base_args = [
    '--Werror',
    '--std-c23',

    '-mmcs51',
    '--model-small',
    '--xram-size', '0x1000',
    '--xram-loc', '0x0000',
    '--code-size', '0xf000',
    '--opt-code-speed',
    '--out-fmt-ihx',

    '-DSMK_VERSION=@0@'.format(meson.project_version()),
]

if get_option('nkro').enabled()
    cc_base_args += '-DNKRO_ENABLE=1'
endif

if get_option('buildtype') == 'debug'
    cc_base_args += '-DDEBUG=1'
endif

sdar_args = ['-rc']

cc = find_program('sdcc', required : true)
sdar = find_program('sdar', required : true)
packihx = find_program('packihx', required : true)
skbt = find_program('sinowealth-kb-tool', required : true)

dir_base = meson.current_source_dir()

cc_incs = []
foreach dir : inc_base_dirs
    cc_incs += '-I' + join_paths(dir_base, dir)
endforeach

compiler = generator(cc,
    output : '@BASENAME@.rel',
    arguments : cc_base_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@']
)

rel_user = compiler.process(src_user)
lib_user = custom_target('user.lib',
    input : rel_user,
    output : 'user.lib',
    command : [sdar, sdar_args, '@OUTPUT@', '@INPUT@'],
)

foreach part : parts
    keyboard = part[0]
    platform = part[1]
    layouts = part[2]
    options = default_options + part[3]

    foreach layout : layouts
        src_main = src_smk
        inc_dirs = inc_base_dirs
        if platform == 'sh68f90a'
            src_main += src_platform_sh68f90a
            inc_dirs += inc_platform_sh68f90a
        else
            error('unsupported platform: @0@'.format(platform))
        endif

        cc_args = cc_base_args + [
            '-DFREQ_SYS=@0@'.format(options['freq_sys']),
            '-DWATCHDOG_ENABLE=@0@'.format(options['watchdog_enable']),
            '-DUSB_VID=@0@'.format(options['vendor_id']),
            '-DUSB_PID=@0@'.format(options['product_id']),
        ]

        wireless = options.get('wireless', '')
        if wireless == 'bk3632'
            src_main += src_platform_bk3632
            inc_dirs += inc_platform_bk3632
            cc_args += '-DRF_ENABLED=1'
        elif wireless != ''
            error('unsupported platform: @0@'.format(platform))
        endif

        kb_inc_dir = 'src/keyboards/@0@'.format(keyboard)
        if fs.is_dir(kb_inc_dir)
            inc_dirs += kb_inc_dir
        else
            error('keyboard dir does not exist: @0@'.format(kb_inc_dir))
        endif

        foreach src : possible_src_keyboard
            if fs.is_file('src/keyboards/@0@/@1@'.format(keyboard, src))
                src_main += 'src/keyboards/@0@/@1@'.format(keyboard, src)
            endif
        endforeach

        layout_inc_dir = 'src/keyboards/@0@/layouts/@1@'.format(keyboard, layout)
        if fs.is_dir(layout_inc_dir)
            inc_dirs += layout_inc_dir
        else
            error('layout dir does not exist: @0@'.format(layout_inc_dir))
        endif

        foreach src : possible_src_layout
            if fs.is_file('src/keyboards/@0@/layouts/@1@/@2@'.format(keyboard, layout, src))
                src_main += 'src/keyboards/@0@/layouts/@1@/@2@'.format(keyboard, layout, src)
            endif
        endforeach

        prefix = '@0@_@1@_'.format(keyboard, layout)

        cc_incs = []
        foreach dir : inc_dirs
            cc_incs += '-I' + join_paths(dir_base, dir)
        endforeach

        compiler = generator(cc,
            output : '@BASENAME@.rel',
            arguments : cc_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@'],
        )

        rel_main = compiler.process(src_main)

        ihx_smk_target_name = '@0@smk.ihx'.format(prefix)
        ihx_smk = custom_target(ihx_smk_target_name,
            input : rel_main,
            output : ihx_smk_target_name,
            depends: [lib_user],
            command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@', '-l' + lib_user.full_path()],
        )

        hex_smk_target_name = '@0@smk.hex'.format(prefix)
        hex_smk = custom_target(hex_smk_target_name,
            input : ihx_smk,
            output : hex_smk_target_name,
            capture: true,
            install : true,
            install_dir : 'firmware',
            command : [packihx, '@INPUT@'],
        )

        flash_target_name = '@0@flash'.format(prefix)
        run_target(flash_target_name,
            command : [skbt, 'write', '-p', keyboard, hex_smk.full_path()],
            depends : hex_smk,
        )

        message('target available: @0@, flash: @1@'.format(hex_smk_target_name, flash_target_name))
    endforeach
endforeach

project(
    'smk',
    'c',
    version: 'alpha'
)

env = environment()
env.set('DEBUG', '1')

default_options = {
    'freq_sys': '24000000', # 24Mhz
    'watchdog_enable': '1',
    'vendor_id': '0x258a',
    'product_id': '0xdead',
}

parts = [
    ['example', 'sh68f90a', ['default'], {}],
    ['nuphy-air60', 'sh68f90a', ['default'], { 'vendor_id': '0x05ac', 'product_id': '0x024f' }],
]

# SMK

inc_base_dirs = [
    '.',
    'src',
    'src/smk',
    'src/plaform',
    'src/user',
    'src/keyboards',
]

src_smk = [
    'src/main.c',
    'src/smk/host.c',
    'src/smk/matrix.c',
    'src/smk/report.c',
]

# USER DEFAULTS

src_user = [
    'src/user/layout_process_record.c',
    'src/user/indicators_pre_update.c',
    'src/user/indicators_update_step.c',
    'src/user/indicators_post_update.c',
]

# PLATFORM

inc_platform_sh68f90a = [
    'src/plaform/sh68f90a',
]

src_platform_sh68f90a = [
    'src/platform/sh68f90a/clock.c',
    'src/platform/sh68f90a/delay.c',
    'src/platform/sh68f90a/gpio.c',
    'src/platform/sh68f90a/isp.c',
    'src/platform/sh68f90a/ldo.c',
    'src/platform/sh68f90a/pwm.c',
    'src/platform/sh68f90a/uart.c',
    'src/platform/sh68f90a/usb.c',
    'src/platform/sh68f90a/keyboard.c',
]

# USER KEYBOARDS

inc_user_example = [
    'src/keyboards/example',
]

src_user_example = [
    'src/keyboards/example/smatrix.c',
]

inc_user_nuphy_air60 = [
    'src/keyboards/nuphy-air60',
]

src_user_nuphy_air60 = [
    'src/keyboards/nuphy-air60/smatrix.c',
]

# USER LAYOUTS

src_user_example_default = [
    'src/keyboards/example/layouts/default/layout.c',
]

src_user_nuphy_air60_default = [
    'src/keyboards/nuphy-air60/layouts/default/indicators.c',
    'src/keyboards/nuphy-air60/layouts/default/layout.c',
]

cc_base_args = [
    '--Werror',
    '--std-c2x',

    '-mmcs51',
    '--model-small',
	'--xram-size', '0x1000',
    '--xram-loc', '0x0000',
	'--code-size', '0xf000',
    '--opt-code-speed',
    '--out-fmt-ihx',

    '-DSMK_VERSION=@0@'.format(meson.project_version()),
]

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
        endif

        if keyboard == 'nuphy-air60'
            src_main += src_user_nuphy_air60
            inc_dirs += inc_user_nuphy_air60

            if layout == 'default'
                src_main += src_user_nuphy_air60_default
            endif
        elif keyboard == 'example'
            src_main += src_user_example
            inc_dirs += inc_user_example

            if layout == 'default'
                src_main += src_user_example_default
            endif
        else
            error('unsupported keyboard: ' + keyboard)
        endif

        prefix = keyboard + '_' + layout + '_'

        cc_args = cc_base_args + [
            '-DFREQ_SYS=@0@'.format(options['freq_sys']),
            '-DWATCHDOG_ENABLE=@0@'.format(options['watchdog_enable']),
            '-DUSB_VID=@0@'.format(options['vendor_id']),
            '-DUSB_PID=@0@'.format(options['product_id']),
        ]

        cc_incs = []
        foreach dir : inc_dirs
            cc_incs += '-I' + join_paths(dir_base, dir)
        endforeach

        compiler = generator(cc,
            output : '@BASENAME@.rel',
            arguments : cc_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@'],
        )

        rel_main = compiler.process(src_main)
        ihx_smk = custom_target(prefix + 'smk.ihx',
            input : rel_main,
            output : prefix + 'smk.ihx',
            depends: [lib_user],
            command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@', '-l' + lib_user.full_path()],
        )

        hex_smk = custom_target(prefix + 'smk.hex',
            input : ihx_smk,
            output : prefix + 'smk.hex',
            capture: true,
            install : true,
            install_dir : 'firmware',
            command : [packihx, '@INPUT@'],
        )

        flash = run_target(prefix + 'flash',
            command : [skbt, 'write', '-p', keyboard, hex_smk.full_path()],
            depends : hex_smk,
        )
    endforeach
endforeach

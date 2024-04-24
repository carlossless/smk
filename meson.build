project(
    'smk',
    'c',
    version: 'alpha'
)

parts = [
    ['nuphy-air60', 'sh68f90a', ['default']]
]

cc_args = [
    # '--Werror', # TODO: reeneable
    '--std-c2x',

    '-mmcs51',
    '--model-small',
	'--xram-size', '0x1000',
    '--xram-loc', '0x0000',
	'--code-size', '0xf000',
    '--opt-code-speed',
    '--out-fmt-ihx',

    '-DFREQ_SYS=24000000',
    '-DWATCHDOG_ENABLE=1',
    '-DUSB_VID=0x05ac',
    '-DUSB_PID=0x024f',
    '-DSMK_VERSION=@0@'.format(meson.project_version()),
]

if get_option('buildtype') == 'debug'
    cc_args += '-DDEBUG=1'
endif

sdar_args = ['-rc']

inc_dirs = [
    '.',
    'src',
    'src/smk',
    'src/plaform',
    'src/plaform/sh68f90a', # TODO: move to a conditional
    'src/user',
    'src/keyboards',
]

src_main = [
    'src/main.c',
    'src/smk/host.c',
    'src/smk/matrix.c',
    'src/smk/report.c',
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

src_user = [
    'src/user/indicators.c',
    'src/user/layout.c',
]

cc = find_program('sdcc', required : true)
sdar = find_program('sdar', required : true)
packihx = find_program('packihx', required : true)
skbt = find_program('sinowealth-kb-tool', required : true)

dir_base = meson.current_source_dir()
cc_incs = []

foreach dir : inc_dirs
    cc_incs += '-I' + join_paths(dir_base, dir)
endforeach

compiler = generator(cc,
    output : '@BASENAME@.rel',
    arguments : cc_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@']
)

rel_user = compiler.process(src_user)
lib_user = custom_target('user.lib',
    input : rel_user,
    output : 'user.lib',
    command : [sdar, sdar_args, '@OUTPUT@', '@INPUT@'],
)

rel_platform_sh68f90a = compiler.process(src_platform_sh68f90a)
lib_platform_sh68f90a = custom_target('platform_sh68f90a.lib',
    input : rel_platform_sh68f90a,
    output : 'platform_sh68f90a.lib',
    command : [sdar, sdar_args, '@OUTPUT@', '@INPUT@'],
)

foreach part : parts
    keyboard = part[0]
    platform = part[1]
    foreach layout : part[2]
        if platform == 'sh68f90a'
            lib_platform = lib_platform_sh68f90a
        else
            error('unsupported platform: ' + platform)
        endif

        if keyboard == 'nuphy-air60'
            src_main += [
                'src/keyboards/nuphy-air60/smatrix.c',
                'src/keyboards/nuphy-air60/layouts/default/indicators.c',
                'src/keyboards/nuphy-air60/layouts/default/layout.c',
            ]
            inc_dirs += [
                'src/keyboards/nuphy-air60',
            ]
        else
            error('unsupported keyboard: ' + keyboard)
        endif

        prefix = keyboard + '_' + layout + '_'

        rel_main = compiler.process(src_main)
        ihx_smk = custom_target(prefix + 'smk.ihx',
            input : rel_main,
            output : prefix + 'smk.ihx',
            depends: [lib_platform, lib_user],
            command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@', '-l' + lib_platform.full_path(), '-l' + lib_user.full_path()],
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
            command : [skbt, 'write', '-p', 'nuphy-air60', hex_smk.full_path()],
            depends : hex_smk,
        )

        message(prefix + 'flash')
    endforeach
endforeach

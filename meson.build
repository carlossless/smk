project(
    'smk',
    'c',
    version: 'alpha'
)

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

    '-DDEBUG=1',
    '-DFREQ_SYS=24000000',
    '-DWATCHDOG_ENABLE=1',
    '-DUSB_VID=0x05ac',
    '-DUSB_PID=0x024f',
    '-DSMK_VERSION=@0@'.format(meson.project_version()),
]

sdar_args = ['-rc']

inc_dirs = [
    '.',
    'src',
    'src/smk',
    'src/plaform',
    'src/plaform/sh68f90a', # TODO: move to a conditional
    'src/user',
    'src/keyboards',

    # TODO: should be based on keyboard + layout selection
    'src/keyboards/nuphy-air60',
]

src_main = [
    'src/main.c',
    'src/smk/host.c',
    'src/smk/matrix.c',
    'src/smk/report.c',

    # TODO: should be based on keyboard + layput seleciton
    'src/keyboards/nuphy-air60/smatrix.c', # TODO: rename properly
    'src/keyboards/nuphy-air60/layouts/default/indicators.c',
    'src/keyboards/nuphy-air60/layouts/default/layout.c',
]

src_platform = [ # TODO: should be based on platform selection
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

rel_platform = compiler.process(src_platform)
rel_user = compiler.process(src_user)
rel_main = compiler.process(src_main)

lib_platform = custom_target('platform.lib',
    input : rel_platform,
    output : 'platform.lib',
    command : [sdar, sdar_args, '@OUTPUT@', '@INPUT@'],
)

lib_user = custom_target('user.lib',
    input : rel_user,
    output : 'user.lib',
    command : [sdar, sdar_args, '@OUTPUT@', '@INPUT@'],
)

ihx_smk = custom_target('smk.ihx', # should be keyboard/layout specific
    input : rel_main,
    output : 'smk.ihx',
    depends: [lib_platform, lib_user],
    command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@', '-l' + lib_platform.full_path(), '-l' + lib_user.full_path()],
)

hex_smk = custom_target('smk.hex', # should be keyboard/layout specific
    input : ihx_smk,
    output : 'smk.hex',
    capture: true,
    install : true,
    install_dir : 'firmware',
    command : [packihx, '@INPUT@'],
)

flash = run_target('flash',
    command : [skbt, 'write', '-p', 'nuphy-air60', hex_smk.full_path()],
    depends : hex_smk,
)

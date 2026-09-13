project(
    'smk',
    'c',
    version: 'alpha',
    meson_version: '>=1.1.0'
)

env = environment()
env.set('DEBUG', '1')

default_options = {
    'watchdog_enable': '1',
    'vendor_id': '0x258a',
    'product_id': '0xdead',
}

dir_base = meson.current_source_dir()

inc_base = [dir_base]
src_smk = []
src_kb = []
src_user = []
platforms = {}
wireless_modules = {}
nvm_modules = {}
keyboards = {}

subdir('src')

cc_base_args = [
    '--Werror',
    '--std-c23',

    '-mmcs51',
    '--model-large',
    '--stack-auto',
    '--xram-loc', '0x0000',
    '--opt-code-speed',
    '--out-fmt-ihx',

    '-DSMK_VERSION=@0@'.format(meson.project_version()),
]

if get_option('nkro').enabled()
    cc_base_args += '-DNKRO_ENABLE=1'
endif

if get_option('sleep').enabled()
    cc_base_args += '-DSLEEP_ENABLE=1'
endif

if get_option('buildtype') == 'debug'
    cc_base_args += '-DDEBUG=1'
    cc_base_args += '--debug'   # emit SDCC .cdb for source-level tests (tests/sim.py)

    debug_sink = get_option('debug_sink')
    if debug_sink == 'console' or debug_sink == 'both'
        cc_base_args += '-DDEBUG_SINK_CONSOLE=1'
    endif
    if debug_sink == 'uart' or debug_sink == 'both'
        cc_base_args += '-DDEBUG_SINK_UART=1'
    endif
endif

sdar_args = ['-rc']

cc = find_program('sdcc', required : true)
sdar = find_program('sdar', required : true)
packihx = find_program('packihx', required : true)
sinowisp = find_program('sinowisp', required : true)

# SDCC's -M family is deps-only, so a wrapper writes the depfile ninja needs.
py = find_program('python3', required : true)
sdcc_compile = join_paths(meson.current_source_dir(), 'utils/sdcc_compile.py')

# Fails the build on an unclaimed interrupt vector, or on an ISR-reachable function
# sharing SDCC's static overlay with a main-loop one (project_sdcc_isr_overlay_collision).
check_interrupts = files('utils/check_interrupts.py')

# Host-side generator that bakes per-device LED animation geometry into ROM tables
# (see utils/led_geometry_gen.c). Compiled once with the host compiler at configure
# time so the generated headers exist on disk before any firmware source is built.
host_cc = find_program('cc', 'gcc', required : true)
gen_led_geometry = join_paths(meson.current_build_dir(), 'led_geometry_gen')
run_command(host_cc, '-O2', '-o', gen_led_geometry, join_paths(meson.current_source_dir(), 'utils/led_geometry_gen.c'), check : true)

cc_incs = []
foreach dir : inc_base
    cc_incs += '-I' + dir
endforeach

compiler = generator(py,
    output : '@BASENAME@.rel',
    # depfile + wrapper: rebuild a .rel whenever one of its #included headers changes
    # (plain SDCC gives no header tracking, so header edits are otherwise silent).
    depfile : '@BASENAME@.d',
    arguments : [sdcc_compile, '@DEPFILE@', cc.full_path()] + cc_base_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@']
)

rel_user = compiler.process(src_user + src_kb)
lib_user = custom_target('user.lib',
    input : rel_user,
    output : 'user.lib',
    command : [sdar, sdar_args, '@OUTPUT@', '@INPUT@'],
)

foreach keyboard, board : keyboards
    platform = board['platform']
    if not platforms.has_key(platform)
        error('unsupported platform: @0@ (keyboard @1@)'.format(platform, keyboard))
    endif

    spec            = platforms[platform]
    options         = default_options + board['options']
    bootloader_addr = spec['flash_size'] - spec['bootloader_size']
    marker_addr     = bootloader_addr - spec['sector_size']

    platform_cc_args = [
        '--xram-size', '@0@'.format(spec['xram_size']),
        '-DFREQ_SYS=@0@'.format(spec['freq_sys']),
    ]

    if spec['settings_store'] == 'flash'
        settings_addr = marker_addr - spec['sector_size']
        platform_cc_args += [
            '--code-size', '@0@'.format(settings_addr),
            '-DFLASH_CFG_ADDR=@0@u'.format(settings_addr),
            '-DFLASH_CFG_SIZE=@0@u'.format(spec['sector_size']),
            '-DFLASH_MARKER_ADDR=@0@u'.format(marker_addr),
        ]
    else
        # settings live outside the program flash, so code may run up to the marker sector
        platform_cc_args += ['--code-size', '@0@'.format(marker_addr)]
    endif

    foreach layout, variant : board['layouts']
        src_main = src_smk + spec['src']
        inc_dirs = inc_base + [spec['dir']]

        cc_args = cc_base_args + platform_cc_args + [
            '-DWATCHDOG_ENABLE=@0@'.format(options['watchdog_enable']),
            '-DUSB_VID=@0@'.format(options['vendor_id']),
            '-DUSB_PID=@0@'.format(options['product_id']),
            '-DKEYBOARD_NAME="@0@"'.format(keyboard),
            '-DLAYOUT_NAME="@0@"'.format(layout),
        ]

        wireless = options.get('wireless', '')
        if wireless != ''
            if not wireless_modules.has_key(wireless)
                error('unsupported wireless module: @0@'.format(wireless))
            endif
            mod = wireless_modules[wireless]
            src_main += mod['src']
            inc_dirs += mod['inc']
            cc_args += mod['args']
        endif

        nvm = options.get('nvm', '')
        if nvm != ''
            if not nvm_modules.has_key(nvm)
                error('unsupported nvm: @0@'.format(nvm))
            endif
            mod = nvm_modules[nvm]
            src_main += mod['src']
            inc_dirs += mod['inc']
            cc_args += mod['args']
        endif

        src_main += board['src'] + variant['src']
        inc_dirs += [board['dir'], variant['dir']]

        prefix = '@0@_@1@_'.format(keyboard, layout)

        # LED boards bake their animation geometry into ROM tables at configure time:
        # a header with the matrix sizes (for the board's _Static_assert) and a source
        # file with the tables + user_led_* accessors the shared effect engine calls.
        geom_src = []
        if options.has_key('led_cols')
            configure_file(
                output : prefix + 'led_geometry.h',
                command : [gen_led_geometry, 'header', options['led_rows'], options['led_cols'], options['led_cx2'], options['led_cy2']],
                capture : true,
            )
            geom_src += configure_file(
                output : prefix + 'led_geometry.c',
                command : [gen_led_geometry, 'source', options['led_rows'], options['led_cols'], options['led_cx2'], options['led_cy2']],
                capture : true,
            )
            cc_args += '-DLED_GEOMETRY_HEADER="@0@led_geometry.h"'.format(prefix)
            cc_args += '-I' + meson.current_build_dir()
        endif

        cc_incs = []
        foreach dir : inc_dirs
            cc_incs += '-I' + dir
        endforeach

        compiler = generator(py,
            output : '@BASENAME@.rel',
            # See the user-lib compiler above: wrapper + depfile so ninja rebuilds a
            # .rel whenever one of its #included headers changes.
            depfile : '@BASENAME@.d',
            arguments : [sdcc_compile, '@DEPFILE@', cc.full_path()] + cc_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@'],
        )

        rel_main = compiler.process(src_main, geom_src)

        ihx_smk_target_name = '@0@smk.ihx'.format(prefix)
        ihx_smk = custom_target(ihx_smk_target_name,
            input : rel_main,
            output : ihx_smk_target_name,
            depends: [lib_user],
            command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@', '-l' + lib_user.full_path()],
        )

        # The .hex depends on this, so a violation breaks the build.
        interrupt_check = custom_target('@0@interrupt-check'.format(prefix),
            input : ihx_smk,
            output : '@0@smk.interrupts-ok'.format(prefix),
            command : [py, check_interrupts,
                       '--src', spec['dir'],
                       '--asmdir', meson.current_build_dir() / '@0@smk.ihx.p'.format(prefix),
                       '--map', meson.current_build_dir() / '@0@smk.map'.format(prefix),
                       '--stamp', '@OUTPUT@'],
        )

        hex_smk_target_name = '@0@smk.hex'.format(prefix)
        hex_smk = custom_target(hex_smk_target_name,
            input : ihx_smk,
            output : hex_smk_target_name,
            capture: true,
            install : true,
            install_dir : 'firmware',
            depends : interrupt_check,
            command : [packihx, '@INPUT@'],
        )

        flash_target_name = '@0@flash'.format(prefix)
        run_target(flash_target_name,
            command : [sinowisp, 'write', '-d', keyboard, '--force', hex_smk.full_path()], # TODO: try to remove the need for --force
            depends : hex_smk,
        )

        message('target available: @0@, flash: @1@'.format(hex_smk_target_name, flash_target_name))
    endforeach
endforeach

# Simulator-based firmware tests, run in the patched uCsim simulator (flake
# package ucsim-sh68f90). Discovers every tests/test_*.py so new test files are
# picked up automatically. Only registered when both the simulator and python3
# are available; the suite itself skips if no firmware .hex has been built yet.
sim = find_program('ucsim-sh68f90', required : false)
py = find_program('python3', required : false)
if sim.found() and py.found()
    test('sim-tests',
        py,
        args : ['-m', 'unittest', 'discover', '-s', join_paths(dir_base, 'tests'),
                '-t', join_paths(dir_base, 'tests')],
        env : {'SMK_UCSIM': sim.full_path()},
        # full suite is ~30s locally and most tests boot through real init;
        # bump well above the meson 30s default so slower CI runners pass.
        timeout : 300,
    )
endif

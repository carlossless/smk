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
    ['example', 'sh68f90', ['default'], {}],
    ['nuphy-air60', 'sh68f90', ['default'], { 'vendor_id': '0x05ac', 'product_id': '0x024f', 'wireless': 'bk3632', 'led_rows': '5', 'led_cols': '16', 'led_cx2': '13', 'led_cy2': '4' }],
    ['eyooso-z11', 'sh68f90', ['default'], { 'vendor_id': '0x258a', 'product_id': '0x002a', 'led_rows': '5', 'led_cols': '14', 'led_cx2': '13', 'led_cy2': '4' }],
    ['hykker', 'sh68f881', ['default'], { 'vendor_id': '0x258a', 'product_id': '0x001f', 'led_rows': '6', 'led_cols': '24', 'led_cx2': '23', 'led_cy2': '5' }],
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
    'src/smk/console.c',
    'src/smk/settings.c',
    'src/smk/led_effect.c',
    'src/smk/sleep.c',
    'src/smk/tick.c',
]

# USER DEFAULTS

src_user = [
    'src/user/user_init.c',
    'src/user/layout_process_record.c',
    'src/user/indicators_start.c',
    'src/user/indicators_pre_update.c',
    'src/user/indicators_update_step.c',
    'src/user/indicators_post_update.c',
    'src/user/indicators_init.c',
    'src/user/indicators_render.c',
    'src/user/indicators_apply_defaults.c',
    'src/user/indicators_validate_settings.c',
    'src/user/indicators_pwm_enable.c',
    'src/user/indicators_pwm_disable.c',
    'src/user/user_led_radial.c',
    'src/user/user_led_axis_x.c',
    'src/user/user_led_axis_y.c',
    'src/user/user_matrix_sinks_off.c',
    'src/user/user_matrix_scan_pre.c',
    'src/user/user_matrix_scan_post.c',
    'src/user/settings_save_pre.c',
    'src/user/settings_save_post.c',

    'src/kb/kb_init.c',
    'src/kb/kb_process_record.c',
    'src/kb/kb_update_switches.c',
    'src/kb/kb_update.c',
]

# PLATFORM

inc_platform_sh68f90 = [
    'src/platform/sh68f90',
]

src_platform_sh68f90 = [
    'src/platform/sh68f90/startup.c',
    'src/platform/sh68f90/clock.c',
    'src/platform/sh68f90/delay.c',
    'src/platform/sh68f90/diag.c',
    'src/platform/sh68f90/flash.c',
    'src/platform/sh68f90/isp.c',
    'src/platform/sh68f90/ldo.c',
    'src/platform/sh68f90/pwm.c',
    'src/platform/sh68f90/systick.c',
    'src/platform/sh68f90/uart.c',
    'src/platform/sh68f90/usb.c',
    'src/platform/sh68f90/stack.c',
    'src/platform/sh68f90/power.c',
    'src/platform/sh68f90/extint.c',
]

inc_platform_sh68f881 = [
    'src/platform/sh68f881',
]

src_platform_sh68f881 = [
    'src/platform/sh68f881/clock.c',
    'src/platform/sh68f881/delay.c',
    'src/platform/sh68f881/diag.c',
    'src/platform/sh68f881/usb.c',
    'src/platform/sh68f881/isp.c',
    'src/platform/sh68f881/ldo.c',
    'src/platform/sh68f881/debug_sink.c',
    'src/platform/sh68f881/power.c',
    'src/platform/sh68f881/extint.c',
    'src/platform/sh68f881/systick.c',
    'src/platform/sh68f881/stack.c',
    'src/platform/sh68f881/flash.c',
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
    'user_sleep.c',
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
    '--model-large',
    '--stack-auto',
    '--xram-size', '0x1000',
    '--xram-loc', '0x0000',
    '--code-size', '0xec00', # cap code below the flash config sector (0xEC00) + bootloader
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

# Guards against the SDCC overlay/interrupt hazard (project_sdcc_isr_overlay_collision):
# fails the build if any interrupt-reachable function shares SDCC's static overlay
# with a main-loop function, which would let an ISR corrupt main-loop state.
check_isr_overlay = files('utils/check_isr_overlay.py')

# Host-side generator that bakes per-device LED animation geometry into ROM tables
# (see utils/led_geometry_gen.c). Compiled once with the host compiler at configure
# time so the generated headers exist on disk before any firmware source is built.
host_cc = find_program('cc', 'gcc', required : true)
gen_led_geometry = join_paths(meson.current_build_dir(), 'led_geometry_gen')
run_command(host_cc, '-O2', '-o', gen_led_geometry, join_paths(meson.current_source_dir(), 'utils/led_geometry_gen.c'), check : true)

# Host-side reader for the HID debug console; built with the host compiler.
executable(
    'smk-console',
    'tools/smk-console.c',
    native : true,
    install : false,
)

dir_base = meson.current_source_dir()

cc_incs = []
foreach dir : inc_base_dirs
    cc_incs += '-I' + join_paths(dir_base, dir)
endforeach

compiler = generator(py,
    output : '@BASENAME@.rel',
    # depfile + wrapper: rebuild a .rel whenever one of its #included headers changes
    # (plain SDCC gives no header tracking, so header edits are otherwise silent).
    depfile : '@BASENAME@.d',
    arguments : [sdcc_compile, '@DEPFILE@', cc.full_path()] + cc_base_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@']
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
        if platform == 'sh68f90'
            src_main += src_platform_sh68f90
            inc_dirs += inc_platform_sh68f90
            platform_cc_args = []
        elif platform == 'sh68f881'
            src_main += src_platform_sh68f881
            inc_dirs += inc_platform_sh68f881
            # 32 KiB part: keep code below the settings sector (0x6800) and the
            # bootloader, and xram below the endpoint buffers at 0x0A00.
            platform_cc_args = ['--code-size', '0x6800', '--xram-size', '0x0900']
        else
            error('unsupported platform: @0@'.format(platform))
        endif

        cc_args = cc_base_args + platform_cc_args + [
            '-DFREQ_SYS=@0@'.format(options['freq_sys']),
            '-DWATCHDOG_ENABLE=@0@'.format(options['watchdog_enable']),
            '-DUSB_VID=@0@'.format(options['vendor_id']),
            '-DUSB_PID=@0@'.format(options['product_id']),
            '-DKEYBOARD_NAME="@0@"'.format(keyboard),
            '-DLAYOUT_NAME="@0@"'.format(layout),
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
            cc_incs += '-I' + join_paths(dir_base, dir)
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

        # Overlay/interrupt-safety gate: reads the just-linked .map + per-module .asm
        # and fails if an ISR-reachable function overlays a main-reachable one. The
        # .hex depends on it, so a violation breaks the build.
        overlay_check = custom_target('@0@overlay-check'.format(prefix),
            input : ihx_smk,
            output : '@0@smk.overlay-ok'.format(prefix),
            command : [py, check_isr_overlay,
                       '--src', meson.current_source_dir() / 'src',
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
            depends : overlay_check,
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

# BRING-UP TARGETS
#
# Minimal images that enumerate and dump the information block to the debug console.
# No matrix, no LEDs, no user lib.

bringup_parts = [
    # keyboard, platform, options
    ['hykker', 'sh68f881', { 'vendor_id': '0x258a', 'product_id': '0x001f' }],
]

cc_bringup_args = [
    '--Werror',
    '--std-c23',

    '-mmcs51',
    '--model-large',
    '--stack-auto',
    # The endpoint buffers are absolute at 0x0A00-0x0AFF, so xram has to reach past them.
    '--xram-size', '0x0b00',
    '--xram-loc', '0x0000',
    '--code-size', '0x6f00', # cap code below the app-validity marker (0x6FFB) + bootloader
    '--opt-code-speed',
    '--out-fmt-ihx',

    '-DDEBUG=1',
    '-DDEBUG_SINK_CONSOLE=1',
    '-DSMK_VERSION=@0@'.format(meson.project_version()),
]

foreach part : bringup_parts
    keyboard = part[0]
    platform = part[1]
    options = part[2]

    # Only what a bare enumerate-and-dump image needs: no systick, flash or sleep, so
    # none of the shared smk layer gets dragged in.
    if platform == 'sh68f881'
        src_bringup = [
            'src/platform/sh68f881/clock.c',
            'src/platform/sh68f881/delay.c',
            'src/platform/sh68f881/diag.c',
            'src/platform/sh68f881/usb.c',
            'src/platform/sh68f881/isp.c',
            'src/platform/sh68f881/debug_sink.c',
        ]
        inc_bringup = inc_platform_sh68f881
    else
        error('unsupported bring-up platform: @0@'.format(platform))
    endif

    src_bringup += 'src/keyboards/@0@/bringup.c'.format(keyboard)
    src_bringup += 'src/smk/console.c'
    inc_bringup += 'src/keyboards/@0@'.format(keyboard)
    inc_bringup += 'src/smk'
    inc_bringup += 'src'

    cc_args = cc_bringup_args + [
        '-DWATCHDOG_ENABLE=1',
        '-DUSB_VID=@0@'.format(options['vendor_id']),
        '-DUSB_PID=@0@'.format(options['product_id']),
        '-DKEYBOARD_NAME="@0@"'.format(keyboard),
    ]

    cc_incs = []
    foreach dir : inc_bringup
        cc_incs += '-I' + join_paths(dir_base, dir)
    endforeach

    prefix = '@0@_bringup_'.format(keyboard)

    compiler = generator(py,
        output : '@BASENAME@.rel',
        depfile : '@BASENAME@.d',
        arguments : [sdcc_compile, '@DEPFILE@', cc.full_path()] + cc_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@'],
    )

    ihx_bringup_name = '@0@smk.ihx'.format(prefix)
    ihx_bringup = custom_target(ihx_bringup_name,
        input : compiler.process(src_bringup),
        output : ihx_bringup_name,
        command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@'],
    )

    hex_bringup_name = '@0@smk.hex'.format(prefix)
    custom_target(hex_bringup_name,
        input : ihx_bringup,
        output : hex_bringup_name,
        capture : true,
        install : true,
        install_dir : 'firmware',
        command : [packihx, '@INPUT@'],
    )

    message('bring-up target available: @0@'.format(hex_bringup_name))
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

kb_dir = meson.current_source_dir()

keyboards += {
    'example' : {
        'dir'      : kb_dir,
        'platform' : 'sh68f90',
        'src'      : files(
            'user_matrix.c',
            'user_sleep.c',
            'kb.c',
        ),
        'layouts'  : {
            'default' : {
                'dir' : kb_dir / 'layouts/default',
                'src' : files('layouts/default/layout.c'),
            },
        },
        'options'  : {},
    },
}

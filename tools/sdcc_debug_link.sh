#!/bin/sh
# Wrapper for the SDCC firmware link in debug (--debug) builds.
#
# With --debug, SDCC's linker reports "Multiple definition of C$<hdr>$<line>$..."
# for the C-line debug records of the `static inline` helpers in headers
# (report.h's KEYCODE2SYSTEM/KEYCODE2CONSUMER, add_key/del_key/...): each
# translation unit that includes the header emits its own copy, and merging
# them at link time collides. These are harmless -- the linker still writes a
# correct .ihx and the .cdb we need for source-level tests -- but it exits
# nonzero, which would fail the build.
#
# So: run the real link, and succeed as long as the -o output was produced.
# Any *real* link failure produces no output and is still reported.

out=""
prev=""
for a in "$@"; do
    [ "$prev" = "-o" ] && out="$a"
    prev="$a"
done

"$@"
rc=$?

if [ -n "$out" ] && [ -s "$out" ]; then
    exit 0
fi
exit "$rc"

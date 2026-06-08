// smk-console: print SMK's HID debug console output.
//
// SMK (DEBUG builds) ships debug text as HID input reports with report id
// REPORT_ID_CONSOLE (7) on its "extra" interface. This tool opens every
// /dev/hidraw* node matching the given VID:PID and prints the payload of any
// report 7 it sees. It keeps scanning, so matching devices that appear or
// disappear (unplug/replug, reboot, re-enumeration) are picked up and dropped
// automatically without restarting the tool. Reading hidraw nodes usually
// requires root.
//
// On connect it sends a SET_REPORT(Feature, 7) handshake: the firmware holds
// its console output buffered until a tool announces itself this way, then
// flushes everything queued so far (including the boot banner). This defeats
// the attach race where the kernel would otherwise drain a one-shot report
// before this tool has the node open.
//
//   cc -O2 -o smk-console tools/smk-console.c
//   sudo ./smk-console            # defaults to 05ac:024f (nuphy-air60)
//   sudo ./smk-console 258a:002a  # eyooso-z11
//
#include <dirent.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define REPORT_ID_CONSOLE 7
#define MAX_DEVS          32
#define RESCAN_MS         500

static struct pollfd fds[MAX_DEVS];
static char          paths[MAX_DEVS][300];
static int           nfds;

static int already_open(const char *path)
{
    for (int i = 0; i < nfds; i++) {
        if (strcmp(paths[i], path) == 0) return 1;
    }
    return 0;
}

// Open any matching /dev/hidraw* nodes we are not already listening on.
static void scan(unsigned vid, unsigned pid)
{
    DIR *d = opendir("/dev");
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) && nfds < MAX_DEVS) {
        if (strncmp(e->d_name, "hidraw", 6) != 0) continue;
        char path[300];
        snprintf(path, sizeof(path), "/dev/%s", e->d_name);
        if (already_open(path)) continue;
        // O_RDWR so we can send the attach handshake; fall back to read-only.
        int fd = open(path, O_RDWR);
        if (fd < 0) fd = open(path, O_RDONLY);
        if (fd < 0) continue;
        struct hidraw_devinfo info;
        if (ioctl(fd, HIDIOCGRAWINFO, &info) == 0 && (uint16_t)info.vendor == vid && (uint16_t)info.product == pid) {
            fprintf(stderr, "[smk-console] connected %s\n", path);
            // Announce ourselves so the firmware flushes its buffered console
            // (incl. the boot banner) to us. Without this it holds output back to
            // avoid the hidraw attach race. The feature report id selects the
            // console; the payload is don't-care (just makes an OUT data stage).
            // Harmless on the wrong interface / a read-only fd / non-DEBUG fw.
            unsigned char hello[5] = {REPORT_ID_CONSOLE, 'S', 'M', 'K', 0};
            ioctl(fd, HIDIOCSFEATURE(sizeof(hello)), hello);
            fds[nfds].fd      = fd;
            fds[nfds].events  = POLLIN;
            fds[nfds].revents = 0;
            snprintf(paths[nfds], sizeof(paths[nfds]), "%s", path);
            nfds++;
        } else {
            close(fd);
        }
    }
    closedir(d);
}

// Drop a dead node (device gone) so a later scan can pick up its replacement.
static void drop(int i)
{
    fprintf(stderr, "[smk-console] disconnected %s\n", paths[i]);
    close(fds[i].fd);
    int last = nfds - 1;
    fds[i] = fds[last];
    memcpy(paths[i], paths[last], sizeof(paths[i]));
    nfds--;
}

int main(int argc, char **argv)
{
    unsigned vid = 0x05ac, pid = 0x024f;
    if (argc > 1 && sscanf(argv[1], "%x:%x", &vid, &pid) != 2) {
        fprintf(stderr, "usage: %s [VID:PID]\n", argv[0]);
        return 2;
    }

    fprintf(stderr, "[smk-console] watching for %04x:%04x (Ctrl-C to quit)\n", vid, pid);

    uint8_t buf[64];
    for (;;) {
        // Pick up newly-arrived matching nodes every cycle.
        scan(vid, pid);

        // poll() doubles as the rescan timer: with no devices it just sleeps
        // RESCAN_MS and loops back to scan() again.
        int r = poll(fds, nfds, RESCAN_MS);
        if (r < 0) {
            perror("poll");
            return 1;
        }
        if (r == 0) continue;

        // Walk high-to-low so drop()'s swap-with-last never skips an entry.
        for (int i = nfds - 1; i >= 0; i--) {
            short re = fds[i].revents;
            if (re & (POLLHUP | POLLERR | POLLNVAL)) {
                drop(i);
                continue;
            }
            if (!(re & POLLIN)) continue;
            int n = read(fds[i].fd, buf, sizeof(buf));
            if (n <= 0) {
                drop(i);
                continue;
            }
            if (buf[0] != REPORT_ID_CONSOLE) continue;
            for (int j = 1; j < n && buf[j] != 0; j++) {
                putchar(buf[j]);
            }
            fflush(stdout);
        }
    }
}

// smk-console: print SMK's HID debug console output.
//
// SMK (DEBUG builds) ships debug text as HID input reports with report id
// REPORT_ID_CONSOLE (7) on its "extra" interface. This tool opens every
// /dev/hidraw* node matching the given VID:PID, and prints the payload of any
// report 7 it sees. Reading hidraw nodes usually requires root.
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

int main(int argc, char **argv)
{
    unsigned vid = 0x05ac, pid = 0x024f;
    if (argc > 1 && sscanf(argv[1], "%x:%x", &vid, &pid) != 2) {
        fprintf(stderr, "usage: %s [VID:PID]\n", argv[0]);
        return 2;
    }

    struct pollfd fds[MAX_DEVS];
    int           nfds = 0;

    // Wait up to ~15s for matching hidraw nodes so we survive a reboot/re-enum.
    for (int tries = 0; tries < 75 && nfds == 0; tries++) {
        DIR *d = opendir("/dev");
        if (!d) {
            perror("opendir /dev");
            return 1;
        }
        struct dirent *e;
        while ((e = readdir(d)) && nfds < MAX_DEVS) {
            if (strncmp(e->d_name, "hidraw", 6) != 0) continue;
            char path[300];
            snprintf(path, sizeof(path), "/dev/%s", e->d_name);
            int fd = open(path, O_RDONLY);
            if (fd < 0) continue;
            struct hidraw_devinfo info;
            if (ioctl(fd, HIDIOCGRAWINFO, &info) == 0 && (uint16_t)info.vendor == vid && (uint16_t)info.product == pid) {
                fprintf(stderr, "[smk-console] listening on %s\n", path);
                fds[nfds].fd     = fd;
                fds[nfds].events = POLLIN;
                nfds++;
            } else {
                close(fd);
            }
        }
        closedir(d);
        if (nfds == 0) usleep(200000);
    }

    if (nfds == 0) {
        fprintf(stderr,
                "[smk-console] no hidraw node for %04x:%04x "
                "(device present? permission? DEBUG build flashed?)\n",
                vid, pid);
        return 1;
    }

    uint8_t buf[64];
    for (;;) {
        if (poll(fds, nfds, -1) < 0) {
            perror("poll");
            return 1;
        }
        for (int i = 0; i < nfds; i++) {
            if (!(fds[i].revents & POLLIN)) continue;
            int n = read(fds[i].fd, buf, sizeof(buf));
            if (n <= 0) continue;
            if (buf[0] != REPORT_ID_CONSOLE) continue;
            for (int j = 1; j < n && buf[j] != 0; j++) {
                putchar(buf[j]);
            }
            fflush(stdout);
        }
    }
}

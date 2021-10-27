/*
 * "unraw" keyboard mode, e.g. after pressing alt-sysrq-r
 *
 * In X, the mode K_OFF is used by default. The existing "kbd_mode" program does not support this mode.
 *
 * This needs to run as root.
 *
 * License: CC0
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

int main(void)
{
    int fd = open("/dev/tty0", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    if (ioctl(fd, KDSKBMODE, (long)K_OFF)) {
        perror("ioctl mode");
        return 2;
    }
    return 0;
}

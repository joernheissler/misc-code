#define _GNU_SOURCE

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>

void typespeed(void)
{
    struct timespec first, last;
    int state = 0;
    unsigned char buf[4096];
    unsigned backspace = 0;
    unsigned chars = 0;

    for (;;) {
        ssize_t l = read(0, buf, sizeof buf);
        if (l < 0) abort();
        if (l == 0) continue;
        if (buf[0] == 3) break;
        if (buf[0] == 127) {
            backspace++;
            write(1, "\x1b[D \x1b[D", 7);
        } else {
            write(1, buf, l);
            chars++;
        }

        if (state == 0) {
            clock_gettime(CLOCK_MONOTONIC, &first);
            last = first;
            state = 1;
        } else if (state == 1) {
            clock_gettime(CLOCK_MONOTONIC, &last);
        }
    }
    if (! state) return;

    chars -= backspace;
    unsigned long long tdiff = (last.tv_sec * 1000 + last.tv_nsec / 1000000)
                             - (first.tv_sec * 1000 + first.tv_nsec / 1000000);
    printf("\nChars: %u, Backspace: %u, Time: %llums, Chars/Minute: %f\n",
            chars,     backspace,     tdiff,        chars * 60000.0 / tdiff);
}

int main(void)
{
    struct termios attr, orig;
    tcgetattr(0, &attr);
    tcgetattr(0, &orig);
    attr.c_lflag &= ~ICANON & ~ISIG & ~ECHO;
    tcsetattr(0, TCSANOW, &attr);

    typespeed();

    tcsetattr(0, TCSANOW, &orig);
}

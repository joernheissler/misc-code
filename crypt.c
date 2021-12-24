#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key> <salt>\n", argv[0] ? argv[0] : "crypt");
        return 1;
    }

    const char *res = crypt(argv[1], argv[2]);
    puts(res ? res : "(null)");

    return 0;
}

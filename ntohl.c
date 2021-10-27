#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

static inline uint32_t wulf_ntohl(unsigned char *p)
{
    uint32_t tmp;
    memcpy(&tmp, p, sizeof tmp);
    return (tmp << 24) | ((tmp << 8) & 0xff0000) | ((tmp >> 8) & 0xff00) | (tmp >> 24);
}

int main(void)
{
    unsigned char buf[4];
    if (fread(buf, sizeof buf, 1, stdin) != 1) abort();
    printf("%" PRIu32 "\n", wulf_ntohl(buf));
    return 0;
}

/* check raid5 with 3 devices for xor errors. */

#include <stdlib.h>
#include <stdio.h>

#define BLOCKSIZE (512*1024)
#define N (BLOCKSIZE / sizeof (unsigned long))

static inline int check(unsigned long buf[3][N])
{
    for(size_t i = 0; i < N; ++i) {
        if((buf[0][i] ^ buf[1][i]) != buf[2][i]) return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    FILE *fd[3];
    int i;
    unsigned long buf[3][N];
    unsigned block = 0;

    if(argc != 4) abort();

    for(i = 0; i < 3; ++i) {
        fd[i] = fopen(argv[i + 1], "rb");
        if(!fd[i]) abort();
    }

    while(i == 3) {
        for(i = 0; i < 3; ++i) {
            if(fread(buf[i], sizeof (unsigned long), N, fd[i]) != N) {
                printf("Read error at disk %d block %u\n", i, block);
                break;
            }
        }
        if(check(buf)) printf("xor error in block %u\n", block);
        ++block;
    }
    for(i = 0; i < 3; ++i) {
        fclose(fd[i]);
    }
    return 0;
}

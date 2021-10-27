/* compare two sets of binary files to each other. If a byte is the same in the first set and the same in the second set but differs
 * across both sets, print the values and position.
 * It is great for reverse engineering binary formats, for example savegames.
 * Save savegame a few times, throw away an item from your inventory (or whatever...), save another few times. Run mcmp. And now
 * you know which byte saves your inventory slot. Also works with other things like ec registers.
 */
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int i;
    FILE **fds,**afds,**bfds;;
    int n;
    int a,b;
    
    if(argc < 4) {
        fprintf(stderr, "Usage: %s <a0> <a1> <aM> '' <b0> <b1> <bN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fds = malloc((argc - 1) * sizeof *fds);
    if(!fds) abort();

    for(a = -1, i = 1, n = 0; i < argc; ++i) {
        if(!argv[i][0]) {
            if(a != -1) abort();
            a = n;
            continue;
        }

        fds[n] = fopen(argv[i], "rb");
        if(!fds[n]) {
            perror(argv[i]);
            abort();
        }
        ++n;
    }
    b = n - a;
    if(a <= 0 || b <= 0) abort();

    afds = fds;
    bfds = fds + a;
    unsigned long pos;

    for(pos = 0;; ++pos) {
        int z = 0;
        int u,v,x;
        u = fgetc(afds[0]);
        v = fgetc(bfds[0]);

        if(u == EOF || v == EOF) break;
        if(u == v) z = 1;
        
        for(i = 1; i < a; ++i) {
            x = fgetc(afds[i]);
            if(x == EOF) goto out;
            if(x != u) z = 1;
        }
        for(i = 1; i < b; ++i) {
            x = fgetc(bfds[i]);
            if(x == EOF) goto out;
            if(x != v) z = 1;
        }
        if(!z) printf("%08lX: %02X  %02X\n", pos, (unsigned)u, (unsigned)v);
    }

out:

    return 0;
}

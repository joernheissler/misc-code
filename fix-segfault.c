/* Fix for the Linux "Segmentation fault" bug */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/mman.h>

static void segv(int sig, siginfo_t *info, void *v)
{
    (void) v;
    if (sig != SIGSEGV) return;
    uintptr_t p = (uintptr_t)info->si_addr;
    p &= ~((uintptr_t)8191);
    if (! mmap((void *)p, 8192, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0)) abort();
}

int main(void)
{
    sigaction(SIGSEGV, &(struct sigaction){.sa_sigaction = segv, .sa_flags = SA_RESTART | SA_SIGINFO}, 0);

    int *foo = (int *)(31337 * sizeof (*foo));
    foo[12] = 42;
    printf("%d\n", foo[12]);
}

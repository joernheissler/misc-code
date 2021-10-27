#include <stddef.h>
#include <stdio.h>

int main(void)
{
    int c;
    int s = 0;
    unsigned char a = 0;

    while(c = getchar(), c != EOF) {
        if(c >= '0' && c <= '9') {
            a = a << 4 | (c - '0');
        } else if(c >= 'a' && c <= 'f') {
            a = a << 4 | (c - 'a' + 10);
        } else if(c >= 'A' && c <= 'F') {
            a = a << 4 | (c - 'A' + 10);
        } else continue;
        ++s;
        if(s == 2) {
            putchar(a);
            s = 0;
        }
    }
    return 0;
}

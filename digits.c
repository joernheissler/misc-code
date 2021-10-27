#include <stdio.h>
#include <limits.h>

/* number of decimal digits an unsigned integer type can have at most,
 * not including the terminating 0 byte of strings.
 * One needs log_10(2) digits per bit, 28/93 is slightly above this value.
 * If bits * 28 is not divisible by 93, the result is truncated and one byte
 * short, hence add 1.
 *
 * E.g. one needs 32 * l(2)/l(10) ~ 9.6323 ~ 10 bytes for 32 bit unsigned int.
 * floor(32 * 28 / 93) + 1 = floor(9.6344) + 1 = 10
 */

#define DIGITS(v) (sizeof(v) * CHAR_BIT * 28 / 93 + 1)

int main(void)
{
    unsigned long x = 12345678910111213141lu;
    char buf[DIGITS(x) + 1];
    sprintf(buf, "%lu", x);
    printf("%lu %s\n", sizeof buf, buf);
    return 0;
}

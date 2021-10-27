#include <stdio.h>

int main(void)
{
    for (int i = 1; i <= 100; ++i) {
        printf("%.*s%.*s%.0d\n",
            -! (i % 3), "Fizz",
            -! (i % 5), "Buzz",
            i * (i * i * i * i % 15 == 1)
        );
    }
}

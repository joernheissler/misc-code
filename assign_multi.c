#include <stdlib.h>
#include <stdio.h>

#define ASSIGN_MULTI(dst, ...)                                                 \
do {                                                                           \
    typedef typeof (*(dst)) assign_multi_dst_type;                             \
    struct assign_multi_struct_type {                                          \
        assign_multi_dst_type x[  sizeof (assign_multi_dst_type[]){__VA_ARGS__}\
                                / sizeof (assign_multi_dst_type) ];            \
    };                                                                         \
    *(struct assign_multi_struct_type *)(dst) =                                \
        (struct assign_multi_struct_type){{__VA_ARGS__}};                      \
} while (0)

int main(void)
{
    int *a = malloc(10 * sizeof *a);

    if (! a) abort();

    ASSIGN_MULTI(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    for (size_t i = 0; i < 10; ++i) {
        printf("%d\n", a[i]);
    }

    free(a);
}

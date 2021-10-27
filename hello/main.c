#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hello.h"

static int foo(hello *greeter)
{
    char name[20];
    size_t name_len;
    
    hello_greet(greeter);

    if (! hello_set_name(greeter, "Slartibartfast")) {
        return 0;
    }
    
    hello_greet(greeter);
    
    printf("Enter another name: ");
    if (! fgets(name, sizeof name, stdin)) {
        fputs("Cannot read name!\n", stderr);
        return 0;
    }
    name_len = strlen(name);
    if (name[name_len - 1] == '\n') {
        name[name_len - 1] = 0;
    }

    if (! hello_set_name(greeter, name)) {
        return 0;
    }

    hello_greet(greeter);
    
    if (! hello_set_name(greeter, NULL)) {
        return 0;
    }
    
    hello_greet(greeter);
    return 1;
}

int main(void)
{
    int result;

    hello *greeter = hello_new();
    if (! greeter) {
        return EXIT_FAILURE;
    }

    result = foo(greeter);
    hello_del(greeter);

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

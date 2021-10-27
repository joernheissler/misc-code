#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hello.h"

struct hello {
    char *name;
};

hello *hello_new(void)
{
    hello *self = malloc(sizeof *self);
    if (! self) {
        perror("malloc");
        return NULL;
    }
    self->name = NULL;
    if (! hello_set_name(self, "World")) {
        free(self);
        return NULL;
    }
    return self;
}

void hello_del(hello *self)
{
    if (self->name) {
        free(self->name);
    }
    free(self);
}

int hello_set_name(hello *self, const char *name)
{
    char *new_name;

    if (name) {
        size_t size = strlen(name) + 1;
        if (new_name = malloc(size), ! new_name) {
            perror("malloc");
            return 0;
        }
        memcpy(new_name, name, size);
    } else {
        new_name = NULL;
    }

    if (self->name) {
        free(self->name);
    }

    self->name = new_name;

    return 1;
}

void hello_greet(hello *self)
{
    printf("Hello, %s!\n", self->name ? self->name : "anonymous");
}

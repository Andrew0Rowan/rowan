#ifndef AASSERT_H
#define AASSERT_H

#include <stdlib.h>
#include <stdio.h>

#define AV_STRINGIFY(s)         AV_TOSTRING(s)
#define AV_TOSTRING(s)          #s

#define assert0(cond) do {                                              \
    if (!(cond)) {                                                      \
        printf("Assertion %s failed at %s:%d\n",                        \
               AV_STRINGIFY(cond), __FILE__, __LINE__);                 \
        abort();                                                        \
    }                                                                   \
} while (0)

#endif
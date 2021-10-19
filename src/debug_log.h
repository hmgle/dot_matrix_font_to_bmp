#ifndef _DEBUG_LOG_H
#define _DEBUG_LOG_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define debug_print(fmt, ...) \
    do { \
        if (DEBUG) \
            fprintf(stderr, "debug_print: %s: %d: %s():" \
                    fmt "\n", __FILE__, __LINE__, __func__, \
                    ##__VA_ARGS__); \
    } while (0)

#endif

#pragma once

#ifndef SEGM_SIZE
// 5kb max segment size
#define SEGMENT_SIZE 1024*5
#endif

#include <stdlib.h>
#include "debug.h"



typedef struct {
    size_t size;
    size_t capacity;
    char* segments[];
} Bigint;



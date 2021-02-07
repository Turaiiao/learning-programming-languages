//
// Copyright (c) 2021 bingxio. All rights reserved.
//

#ifndef LANG_FRAME_H
#define LANG_FRAME_H

#include <stdint.h>

#include "arr.h"
#include "obj.h"

typedef struct {
    arr code;
    arr data;
    arr cons;
    arr vars;
    arr name;
} frame;

void init_frame(
        frame *f,
//        bytecode list.
        uint8_t *code,
//        eval stack.
        arr *data,
//        constant list.
        arr *cons,
//        variable list.
        arr *vars,
//        used builtin function, method and more.
        arr *name
);

#endif // LANG_FRAME_H
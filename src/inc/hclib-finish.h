#ifndef HCLIB_FINISH_H
#define HCLIB_FINISH_H

#include "hclib-promise.h"
#include "hclib-rt.h"

typedef struct finish_t {
    struct finish_t* parent;
    volatile int counter;
    hclib_future_t *finish_dep;
    tree_node *node_in_dpst;
    int belong_to_task_id;
} finish_t;

#endif

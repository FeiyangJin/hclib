#ifndef HCLIB_FINISH_H
#define HCLIB_FINISH_H

#include "hclib-promise.h"

#ifdef DRDP_ENABLED
#include "hclib-rt.h"
#endif

typedef struct finish_t {
    struct finish_t* parent;
    volatile int counter;
    hclib_future_t *finish_dep;
#ifdef DRDP_ENABLED
    tree_node *node_in_dpst;
    int belong_to_task_id;
#endif
} finish_t;

#endif

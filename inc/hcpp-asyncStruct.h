/*
 * hcpp-asyncStruct.h
 *  
 *      Author: Vivek Kumar (vivekk@rice.edu)
 */

#ifndef HCPP_ASYNCSTRUCT_H_
#define HCPP_ASYNCSTRUCT_H_

#include <string.h>

#include "hcpp-place.h"
#include "hcpp-task.h"

#ifdef __cplusplus
extern "C" {
#endif

inline void init_task_t(task_t *t,
        generic_framePtr fp, size_t arg_sz, void *async_args) {
    HASSERT(t);
    HASSERT(arg_sz <= MAX_HCPP_ASYNC_ARG_SIZE);
    t->_fp = fp;
    t->is_asyncAnyType = 0;
    t->ddf_list = NULL;
    memcpy(&t->_args, async_args, arg_sz);
}

inline hclib_ddf_t ** get_ddf_list(task_t *t) {
    return t->ddf_list;
}

inline void mark_as_asyncAnyTask(task_t *t) {
    t->is_asyncAnyType = 1;
}

inline int is_asyncAnyTask(task_t *t) {
    return t->is_asyncAnyType;
}

void spawn(task_t * task);
void spawn_at_hpt(place_t* pl, task_t * task);
void spawn_await(task_t * task, hclib_ddf_t** ddf_list);
void spawn_commTask(task_t * task);

#ifdef __cplusplus
}
#endif

#endif /* HCPP_ASYNCSTRUCT_H_ */

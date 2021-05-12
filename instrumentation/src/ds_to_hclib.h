#ifndef DS_TO_HCLIB_H 
#define DS_TO_HCLIB_H 
#include "ds_dpst.h"

#ifdef __cplusplus
extern "C" {
#endif

extern DisjointSet *ds;
extern bool hclib_ready;

typedef int (*hclib_function)(void); 
extern hclib_function hclib_current_task_id; 

typedef void* (*hclib_function_dpst)(void);
extern hclib_function_dpst hclib_current_step_node;
extern hclib_function_dpst hclib_print_dpst;

hclib_task test_get_task_info(int task_id);
__attribute__((weak)) int ds_get_current_finish();
__attribute__((weak)) bool ds_current_is_future();
__attribute__((weak)) void* hclib_get_current_task_info(int* task_id, int* current_finish_id, bool* is_step, bool* is_future);

#ifdef __cplusplus
}
#endif


#endif
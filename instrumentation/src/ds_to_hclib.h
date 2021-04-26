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

#ifdef __cplusplus
}
#endif


#endif
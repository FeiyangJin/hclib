#ifndef AAA_C_CONNECTOR_H 
#define AAA_C_CONNECTOR_H 

#ifdef __cplusplus
extern "C" {
#endif


void ds_addSet(int set_index);
int ds_findSet(int element);
void ds_unionSet(int a, int b);
void ds_printElementSets(int *element_array);
void ds_merge(int a, int b);
void ds_addnt(int task, int nt_task_id);
int ds_ntcounts(int task_id);
int ds_getlsa(int task_id);
void ds_setlsa(int task_id, int lsa);
void ds_printAll();
void ds_printdsbyset();

void ds_addtask(int task_id, int parent_id, void *node_in_dpst, void *task_address, int state);
int ds_parentid(int task_id);
int ds_taskState(int task_id);
void* ds_get_dpst_node(int task_id);

void ds_addFinish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address);
void ds_add_task_to_finish(int finish_id, int task_id);
void ds_end_finish_merge(int finish_id);

#ifdef __cplusplus
}
#endif


#endif
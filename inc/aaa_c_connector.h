#ifndef AAA_C_CONNECTOR_H 
#define AAA_C_CONNECTOR_H 

#ifdef __cplusplus
extern "C" {
#endif


void ds_addSet(int set_index);
int ds_findSet(int element);
void* ds_find_set_info(int task_id);
void ds_merge(int a, int b, void* query_node);
void ds_addnt(int task, int nt_task_id, void* last_step_before_nt);
int ds_ntcounts(int task_id);
int ds_ntcounts_task(int task_id);
void ds_print_nt(int set_id);
int ds_getlsa(int task_id);
int ds_getlsa_task(int task_id);
void ds_printAll();
void ds_printdsbyset();
void ds_print_table();

void ds_addtask(int task_id, int parent_id, void *node_in_dpst, void *task_address, int state, void *last_node_reachable_in_parent);
int ds_parentid(int task_id);
int ds_taskState(int task_id);
void ds_print_all_tasks();
void ds_update_task_dpst_node(int task_id, void *new_node);
void* ds_get_dpst_node(int task_id);
void ds_update_task_state(int task_id, int new_state);

void ds_addFinish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address);
void ds_add_task_to_finish(int finish_id, int task_id);
void ds_end_finish_merge(int finish_id, void* query_node);


// reachability
int ds_find_task_node_index(int task_id);
void* ds_find_lca_left_child(void* node1, void* node2);
bool ds_precede(void* step_a, void* step_b, int task_a, int task_b);

#ifdef __cplusplus
}
#endif


#endif
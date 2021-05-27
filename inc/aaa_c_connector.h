#ifndef AAA_C_CONNECTOR_H 
#define AAA_C_CONNECTOR_H 

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) void ds_print_check_write_count();
__attribute__((weak)) void ds_print_check_read_count();
__attribute__((weak)) void ds_promise_task(bool b);
__attribute__((weak)) void ds_free(void* ptr);
__attribute__((weak)) int ds_get_tree_join_count();
__attribute__((weak)) int ds_get_cache_size();
__attribute__((weak)) void ds_print();
__attribute__((weak)) void ds_addSet(int set_index);
__attribute__((weak)) int ds_findSet(int element);
__attribute__((weak)) void ds_merge(int a, int b, void* query_node, bool update_inline_finish);
__attribute__((weak)) void ds_addnt(int task, int nt_task_id, void* last_step_before_nt);
__attribute__((weak)) int ds_ntcounts(int task_id);
__attribute__((weak)) int ds_ntcounts_task(int task_id);
__attribute__((weak)) void ds_print_nt(int set_id);
__attribute__((weak)) int ds_getlsa(int task_id);
__attribute__((weak)) int ds_getlsa_task(int task_id);
__attribute__((weak)) void ds_printAll();
__attribute__((weak)) void ds_printdsbyset();
__attribute__((weak)) void ds_print_table();

__attribute__((weak)) void ds_addtask(int task_id, int parent_id, void *node_in_dpst, void *task_address, int state, int belong_to_finish_id, void *last_node_reachable_in_parent);
__attribute__((weak)) int ds_parentid(int task_id);
__attribute__((weak)) int ds_taskState(int task_id);
__attribute__((weak)) void ds_print_all_tasks();
__attribute__((weak)) void ds_update_task_dpst_node(int task_id, void *new_node);
__attribute__((weak)) void* ds_get_dpst_node(int task_id);
__attribute__((weak)) void ds_update_task_state(int task_id, int new_state);

__attribute__((weak)) void ds_addFinish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address);
__attribute__((weak)) void ds_add_task_to_finish(int finish_id, int task_id);
__attribute__((weak)) void ds_end_finish_merge(int finish_id, void* query_node);


// reachability
__attribute__((weak)) int ds_find_task_node_index(int task_id);
__attribute__((weak)) void* ds_find_lca_left_child(void* node1, void* node2);
__attribute__((weak)) bool ds_precede(void* step_a, void* step_b, int task_a, int task_b);


__attribute__((weak)) void ds_hclib_ready(bool state);
__attribute__((weak)) void ds_set_task_id_pointer(void* function_p);
__attribute__((weak)) void ds_set_step_node_pointer(void* function_p);
__attribute__((weak)) void ds_set_print_dpst_pointer(void* function_p);
__attribute__((weak)) bool ds_dpst_precede(void* node1, void* node);

#ifdef __cplusplus
}
#endif


#endif
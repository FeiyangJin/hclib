#include <cstdlib>

#include "ds_to_hclib.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

DisjointSet *ds = new DisjointSet();
bool hclib_ready = false;

hclib_function hclib_current_task_id = NULL;
hclib_function_dpst hclib_current_step_node = NULL;
hclib_function_dpst hclib_print_dpst = NULL;

hclib_task test_get_task_info(int task_id){
    return ds->get_task_info(task_id);
}

int ds_get_tree_join_count(){
    return ds->get_tree_join_count();
}

bool ds_dpst_precede(void* node1, void* node2){
    return ds->precede_dpst((tree_node_cpp*)node1,(tree_node_cpp*)node2);
}

void ds_hclib_ready(bool state){
    hclib_ready = state;
}

void ds_set_print_dpst_pointer(void* function_p){
    hclib_print_dpst = (hclib_function_dpst) function_p;
}

void ds_set_step_node_pointer(void* function_p){
    hclib_current_step_node = (hclib_function_dpst) function_p;
}

void ds_set_task_id_pointer(void* function_p){
    hclib_current_task_id = (hclib_function) function_p;
}

void ds_end_finish_merge(int finish_id, void* query_node){
    tree_node_cpp* query_node_cpp = (tree_node_cpp*) query_node;
    ds->end_finish_merge(finish_id, query_node_cpp);
}

void ds_add_task_to_finish(int finish_id, int task_id){
    ds->add_task_to_finish(finish_id,task_id);
}

void ds_print_all_tasks(){
    ds->print_all_tasks();
}

void ds_update_task_dpst_node(int task_id, void *new_node){
    ds->update_task_dpst_node(task_id,new_node);
}

void ds_addFinish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address){
    hclib_finish *new_finish = new hclib_finish(finish_id, belong_to_task_id,node_in_dpst,finish_address);
    ds->addFinish(finish_id, new_finish);
}

void ds_addtask(int task_id, int parent_id, void *node_in_dpst, void *task_address, int belong_to_finish_id, int state, void *last_node_reachable_in_parent){
    task_state val = static_cast<task_state>(state);
    hclib_task new_task(task_id, parent_id, node_in_dpst, task_address, belong_to_finish_id, val);
    tree_node_cpp* node = (tree_node_cpp*) last_node_reachable_in_parent;
    ds->addTask(task_id,new_task,node);
}

int ds_parentid(int task_id){
    return ds->get_task_info(task_id).parent_id;
}

int ds_taskState(int task_id){
    int val = static_cast<int>(ds->get_task_info(task_id).this_task_state);
    return val;
}

void* ds_get_dpst_node(int task_id){
    return ds->get_task_info(task_id).node_in_dpst;
}

void ds_addSet(int set_index){
    ds->addSet(set_index);
}

int ds_findSet(int element){
    return ds->Find(element);
}

void ds_merge(int a, int b, void* query_node, bool update_inline_finish){
    tree_node_cpp* query_node_cpp = (tree_node_cpp*) query_node;
    ds->mergeBtoA(a,b,query_node_cpp, update_inline_finish);
}

void ds_addnt(int task, int nt_task_id, void* last_step_before_nt){
    tree_node_cpp* node = (tree_node_cpp*) last_step_before_nt;
    ds->addnt(task,nt_task_id,node);
}

int ds_ntcounts(int task_id){
    return ds->ntcounts(task_id);
}

int ds_ntcounts_task(int task_id){
    return ds->ntcounts_task(task_id);
}

void ds_print_nt(int set_id){
    ds->print_nt(set_id);
}

int ds_getlsa(int task_id){
    return ds->getlsa(task_id);
}

int ds_getlsa_task(int task_id){
    return ds->getlsa_task(task_id);
}

void ds_printAll(){
    ds->printds();
}

void ds_printdsbyset(){
    ds->printdsbyset();
}

void ds_print_table(){
    printf("print table from instrumentation \n");
    ds->print_table();
}

void ds_update_task_state(int task_id, int new_state){
    task_state state = static_cast<task_state>(new_state);
    ds->update_task_state(task_id,state);
}

int ds_find_task_node_index(int task_id){
    return ds->find_task_node_index(task_id);
}

void* ds_find_lca_left_child(void* node1, void* node2){
    tree_node_cpp* node1_tree = (tree_node_cpp*) node1;
    tree_node_cpp* node2_tree = (tree_node_cpp*) node2;
    return (void*) ds->find_lca_left_child_cpp(node1_tree,node2_tree); 
}

bool ds_precede(void* step_a, void* step_b, int task_a, int task_b){
    tree_node_cpp* step_a_tree = (tree_node_cpp*) step_a;
    tree_node_cpp* step_b_tree = (tree_node_cpp*) step_b;
    return ds->precede(step_a_tree,step_b_tree,task_a,task_b);
}

void ds_print(){
    printf("hello from hclib \n");
}

int ds_get_cache_size(){
    return ds->get_cache_size();
}

#ifdef __cplusplus
}
#endif
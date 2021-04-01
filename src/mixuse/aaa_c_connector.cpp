#include <cstdlib>

#include "aaa_c_connector.h"
#include "aaa.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

static DisjointSet *ds = new DisjointSet();

void ds_end_finish_merge(int finish_id){
    ds->end_finish_merge(finish_id);
}

void ds_add_task_to_finish(int finish_id, int task_id){
    ds->add_task_to_finish(finish_id,task_id);
}

void ds_break_previous_steps(int task_id, int task_id_for_previous_steps){
    ds->break_previous_steps(task_id,task_id_for_previous_steps);
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

void ds_addtask(int task_id, int parent_id, void *node_in_dpst, void *task_address, int state){
    task_state val = static_cast<task_state>(state);
    hclib_task *new_task = new hclib_task(task_id, parent_id, node_in_dpst, task_address, val);
    ds->addTask(task_id,new_task);
}

int ds_parentid(int task_id){
    return ds->get_task_info(task_id)->parent_id;
}

int ds_taskState(int task_id){
    int val = static_cast<int>(ds->get_task_info(task_id)->this_task_state);
    return val;
}

void ds_update_task_parent(int task_id, int new_parent_id){
    ds->update_task_parent(task_id, new_parent_id);
}

void* ds_get_dpst_node(int task_id){
    return ds->get_task_info(task_id)->node_in_dpst;
}

void ds_addSet(int set_index){
    ds->addSet(set_index);
}

int ds_findSet(int element){
    return ds->Find(element);
}

void ds_merge(int a, int b){
    ds->mergeBtoA(a,b);
}

void ds_addnt(int task, int nt_task_id){
    ds->addnt(task,nt_task_id);
}

int ds_ntcounts(int task_id){
    return ds->ntcounts(task_id);
}

int ds_getlsa(int task_id){
    return ds->getlsa(task_id);
}

void ds_setlsa(int task_id, int lsa){
    ds->setlsa(task_id,lsa);
}

void ds_printAll(){
    ds->printds();
}

void ds_printdsbyset(){
    ds->printdsbyset();
}

void ds_print_table(){
    ds->print_table();
}

void ds_update_task_state(int task_id, int new_state){
    task_state state = static_cast<task_state>(new_state);
    ds->update_task_state(task_id,state);
}

#ifdef __cplusplus
}
#endif
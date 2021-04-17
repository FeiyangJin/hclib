#include "hclib_cpp.h"
#include <unistd.h>

typedef struct nt_info{
    int task_id;
    tree_node* last_node_before_this_nt;
} nt_info;

typedef struct set_info{
    int set_id;
    tree_node* query_node_in_current_set;
} set_info;

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {
    // test DPST initilization
    hclib_worker_state *main_ws = current_ws();
    hclib_task_t *main_task = (hclib_task_t *) main_ws->curr_task;
    tree_node *main_task_node = (tree_node*) ds_get_dpst_node(main_task->task_id);
    assert(main_task_node->depth == 0);
    assert(main_task_node->index == 0);
    assert(main_task_node->parent == NULL);
    assert(main_task_node->this_node_type == ROOT);
    assert(main_task_node->children_list_head->this_node_type == FINISH); 

    // test nt joins on dpst
    hclib::future_t<void> *fa = hclib::async_future([](){
        return;
    });

    hclib::future_t<void> *fb = hclib::async_future([&](){
        fa->wait();
    });
    
    tree_node* fb_node = (tree_node*) ds_get_dpst_node(fb->corresponding_task_id);
    assert(fb_node->number_of_child == 2);
    assert(fb_node->children_list_head->is_parent_nth_child == 0);
    assert(fb_node->children_list_tail->is_parent_nth_child == 1);
    assert(fb_node->children_list_head->this_node_type == fb_node->children_list_tail->this_node_type);
    assert(fb_node->children_list_head->this_node_type == STEP);

    // test merge
    fb->wait();
    assert(ds_findSet(0) == ds_findSet(fb->corresponding_task_id));
    assert(ds_findSet(0) != ds_findSet(fa->corresponding_task_id));
    
    //printDPST();
    set_info* b_set_info = (set_info*) ds_find_set_info(fb->corresponding_task_id);
    tree_node* b_query_node = b_set_info->query_node_in_current_set;
    assert(b_query_node->this_node_type == STEP);
    assert(b_query_node->parent->index == 1);
    assert(b_query_node->parent->this_node_type == FINISH);

    // test merge part 2
    int inner_task_id;
    set_info* fc_set_info;
    set_info* inner_task_set_info;
    hclib::future_t<void> *fc = hclib::async_future([&](){
        hclib_worker_state *ws = current_ws();
        hclib_task_t *fc_task = (hclib_task_t *) ws->curr_task;
        fc_set_info = (set_info*) ds_find_set_info(fc_task->task_id);
        assert(fc_set_info->set_id == fc_task->task_id);
        assert(fc_set_info->query_node_in_current_set == NULL);

        hclib::finish([&](){

          hclib::async([&](){
            ws = current_ws();
            hclib_task_t *inner_task = (hclib_task_t *) ws->curr_task;
            inner_task_id = inner_task->task_id;
          });
        });

        inner_task_set_info = (set_info*) ds_find_set_info(inner_task_id);
        assert(inner_task_set_info->set_id == ds_findSet(fc_task->task_id));
        assert(inner_task_set_info->query_node_in_current_set != NULL);
        assert(inner_task_set_info->query_node_in_current_set->parent->index == fc_task->node_in_dpst->index);
        return;
    });

    fc->wait();
    
    fc_set_info = (set_info*) ds_find_set_info(fc->corresponding_task_id);
    inner_task_set_info = (set_info*) ds_find_set_info(inner_task_id);
    assert(fc_set_info->set_id == inner_task_set_info->set_id);
    assert(fc_set_info->set_id == ds_findSet(0));
    assert(fc_set_info->query_node_in_current_set != NULL);
    assert(inner_task_set_info->query_node_in_current_set != NULL);
    assert(fc_set_info->query_node_in_current_set->index == inner_task_set_info->query_node_in_current_set->index);
    assert(fc_set_info->query_node_in_current_set->this_node_type == STEP);
    assert(fc_set_info->query_node_in_current_set->parent->index == 1);

    printf("all tests passsed in updated test\n");
    // end of hclib
  });
  
  return 0;
}

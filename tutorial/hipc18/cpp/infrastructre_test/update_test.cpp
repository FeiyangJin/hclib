#include "hclib_cpp.h"
#include <unistd.h>

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
    assert(ds_findSet(0) != ds_findSet(fb->corresponding_task_id));
    assert(ds_findSet(0) != ds_findSet(fa->corresponding_task_id));
    
    assert(ds_findSet(fb->corresponding_task_id) == 2);

    // test merge part 2
    int inner_task_id;
    void* inner_step_node;
    void* inner_step_node2;
    hclib::future_t<void> *fc = hclib::async_future([&](){
        hclib_worker_state *ws = current_ws();
        hclib_task_t *fc_task = (hclib_task_t *) ws->curr_task;

        hclib::finish([&](){
          hclib::async([&](){
            ws = current_ws();
            hclib_task_t *inner_task = (hclib_task_t *) ws->curr_task;
            inner_task_id = inner_task->task_id;
            inner_step_node = (void*) get_current_step_node();
          });
          inner_step_node2 = (void*) get_current_step_node();
        });

        void* cstep = (void*) get_current_step_node();
        assert(ds_precede(inner_step_node,cstep,inner_task_id,fc_task->task_id));
        assert(ds_precede(inner_step_node2,cstep,inner_task_id,fc_task->task_id));

        return;
    });

    fc->wait();

    printf("all tests passsed in updated test\n");
    // end of hclib
  });
  
  return 0;
}

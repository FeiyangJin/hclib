#include "hclib_cpp.h"
#include <unistd.h>

typedef struct nt_info{
    int task_id;
    tree_node *last_node_before_this_nt;
} nt_info;

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

    //ds_print_all_tasks();

    // test nt joins on dpst
    hclib::future_t<void> *fa = hclib::async_future([](){
        return;
    });

    

    hclib::future_t<void> *fb = hclib::async_future([&](){
        fa->wait();
        
        hclib::finish([](){

          hclib::async([](){

          });
          // printDPST();
          // printf("\n");
        });
        ds_print_all_tasks();
        printDPST();
        printf("\n");
    });

    
    //ds_print_nt(ds_findSet(fb->corresponding_task_id));
    
    // tree_node* fb_node = (tree_node*) ds_get_dpst_node(fb->corresponding_task_id);
    // assert(fb_node->number_of_child == 2);
    // assert(fb_node->children_list_head->is_parent_nth_child == 0);
    // assert(fb_node->children_list_tail->is_parent_nth_child == 1);
    // assert(fb_node->children_list_head->this_node_type == fb_node->children_list_tail->this_node_type);

    // test merge
    fb->wait();
    
    //ds_print_table();
    ds_print_all_tasks();
    printDPST();
    
    printf("all tests passsed int updated test\n");
    // end of hclib
  });
  
  return 0;
}

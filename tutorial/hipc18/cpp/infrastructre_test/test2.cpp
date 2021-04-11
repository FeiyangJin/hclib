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

    // test DPST 
    hclib::future_t<void> *fa = hclib::async_future([](){
        hclib::finish([](){

            hclib::async([](){
                hclib_worker_state *ws = current_ws();
                hclib_task_t *task = (hclib_task_t *) ws->curr_task;
                finish_t *finish = get_current_finish(task);

                assert(finish->node_in_dpst->parent->this_node_type == FUTURE);
                assert(finish->node_in_dpst->children_list_head->next_sibling->this_node_type == ASYNC);

                assert(task->node_in_dpst->parent->this_node_type == FINISH);

                hclib::finish([&](){
                    ws = current_ws();
                    task = (hclib_task_t *) ws->curr_task;
                    finish = get_current_finish(task);

                    assert(finish->node_in_dpst->parent->this_node_type == FUTURE);
                    assert(finish->node_in_dpst->parent->corresponding_task_id != task->task_id);
                });
            });
        });
    });

    // notice: ws->current_finish marks the finish inside a task,
    // if no finish inside a task, it saves the finish the task is registered on.
    // task->current_finish always save the finish this task is registered on.
    // so when we execute a task, comparing the two to determine the STEP node 
    // we are at in DPST
    hclib::promise_t<int> *barrier = new hclib::promise_t<int>();
    hclib::future_t<void> *fb = hclib::async_future([&](){
        hclib_worker_state *ws = current_ws();
        hclib_task_t *task = (hclib_task_t *) ws->curr_task;
        finish_t *finish = task->current_finish;
        finish_t *ws_finish = ws->current_finish;
        assert(finish->belong_to_task_id == ws_finish->belong_to_task_id);

        hclib::finish([&](){
            finish = task->current_finish;
            ws_finish = ws->current_finish;
            assert(finish->node_in_dpst->parent->this_node_type == ROOT);
            assert(ws_finish->node_in_dpst->parent->this_node_type == FUTURE);
            
            hclib::finish([&](){
                barrier->get_future()->wait();
                finish = task->current_finish;
                ws_finish = ws->current_finish;
                assert(finish->node_in_dpst->parent->this_node_type == ROOT);
                assert(ws_finish->node_in_dpst->parent->this_node_type == FINISH);
            });
        });

    });
    barrier->put(10);

    tree_node *fa_node = (tree_node*) ds_get_dpst_node(fa->corresponding_task_id);
    tree_node *fb_node = (tree_node*) ds_get_dpst_node(fb->corresponding_task_id);
    assert(fa_node->children_list_head->this_node_type == STEP);
    assert(fb_node->children_list_head->this_node_type == STEP);
    assert(fa_node->next_sibling->next_sibling->index == fb_node->index);
    assert(fa_node->parent->index == fb_node->parent->index);



    // test execution order
    std::vector<int> execution_order;
    hclib::promise_t<int> *p = new hclib::promise_t<int>();
    hclib::promise_t<int> *q = new hclib::promise_t<int>();

    execution_order.push_back(0);

    hclib::finish([&](){
        execution_order.push_back(1);
        hclib::async([&](){
            execution_order.push_back(2);
            p->get_future()->wait();
            execution_order.push_back(6);

            hclib::async([&](){
                execution_order.push_back(7);
                q->get_future()->wait();
                execution_order.push_back(11);
            });
            execution_order.push_back(8);
            execution_order.push_back(9);
        });
        
        execution_order.push_back(3);

        hclib::async([&](){
            execution_order.push_back(4);
            p->get_future()->wait();
            execution_order.push_back(10);

            q->put(200);
            execution_order.push_back(12);
        });

        execution_order.push_back(5);
        p->put(100);
        execution_order.push_back(13);
    });

    execution_order.push_back(14);

    for(int i=0; i<execution_order.size(); i++){
        assert(i == execution_order.at(i));
    }
    
    
    printf("all tests passed int test 2\n");
    // end of hclib
  });
  
  return 0;
}

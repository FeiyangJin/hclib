#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 
    std::string const nodeTypes[5] = {"Root","Finish","Async","Future","Step"};

    hclib::launch(deps, 1, [&]() {

        // test finding lca left child
        int main_task_node_index = ds_find_task_node_index(0);
        tree_node* step1;
        tree_node* step2;
        int inner_task_id1;
        int inner_task_id2;
        hclib_worker_state *ws;
        hclib_task_t *task;

        hclib::async([&](){

            hclib::async([&](){
                step1 = get_current_step_node();
                ws = current_ws();
                task = (hclib_task_t *) ws->curr_task;
                inner_task_id1 = task->task_id;
            });
        });

        hclib::async([&](){
            hclib::async([&](){
                hclib::future_t<void> *f_inner = hclib::async_future([&](){
                    step2 = get_current_step_node();
                    ws = current_ws();
                    task = (hclib_task_t *) ws->curr_task;
                    inner_task_id2 = task->task_id;
                    assert(ds_precede((void*)step1, (void*)step2, inner_task_id1, inner_task_id2) == false);
                });

                f_inner->wait();
            });
           
        });

        tree_node *lca_lc = (tree_node*) ds_find_lca_left_child((void*)step1,(void*)step2);
        assert(lca_lc->this_node_type == ASYNC);
        assert(lca_lc->corresponding_task_id == 1);
        assert(main_task_node_index == 0);


        // test reachability 
        void* step3;
        int task3_id;
        
        hclib::future_t<void> *fa = hclib::async_future([&](){
            hclib::finish([&](){
                hclib::async([&](){
                    step3 = (void*) get_current_step_node();
                    ws = current_ws();
                    task = (hclib_task_t *) ws->curr_task;
                    task3_id = task->task_id;
                });
            });

            return;
        });

        int inner_task_id = 0;
        hclib::future_t<void> *fb = hclib::async_future([&](){
            fa->wait();
            ws = current_ws();
            task = (hclib_task_t *) ws->curr_task;
            void* current_step = (void*) get_current_step_node();
            assert(ds_precede(step3, current_step, task3_id, task->task_id) == true);

            hclib::async([&](){
                ws = current_ws();
                task = (hclib_task_t*) ws->curr_task;
                void* current_step_2 = (void*) get_current_step_node();
                assert(ds_precede(step3, current_step_2, task3_id, task->task_id) == true);
            });
            
        });
        fb->wait();

        // test reachability 2
        hclib::promise_t<int> *p = new hclib::promise_t<int>();
        void* step4;
        int task4_id;
        hclib::future_t<void> *fc = hclib::async_future([&](){

            hclib::async([&](){
                p->get_future()->wait();
                ws = current_ws();
                task = (hclib_task_t*) ws->curr_task;
                void* current_step = (void*) get_current_step_node();
                //printDPST();
                assert(ds_precede(step4, current_step, task4_id, task->task_id) == false);
            });

            step4 = (void*) get_current_step_node();
            ws = current_ws();
            task = (hclib_task_t*) ws->curr_task;
            task4_id = task->task_id;

        });
        p->put(5);

        fc->wait();
        printf("all tests passed in test 4 \n");

        // end of hclib
    });

    return 0;
}

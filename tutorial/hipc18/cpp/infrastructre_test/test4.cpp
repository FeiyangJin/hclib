#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 
    std::string const nodeTypes[5] = {"Root","Finish","Async","Future","Step"};

    hclib::launch(deps, 1, [&]() {

        // test reachability
        int main_task_node_index = ds_find_task_node_index(0);

        hclib::async([](){

        });

        hclib::async([](){

        });

        tree_node *lca_lc = (tree_node*) ds_find_lca_left_child(1,2);

        assert(lca_lc ->this_node_type == ASYNC);
        assert(ds_precede(1,2) == false);
        assert(ds_find_task_node_index(1) == 3);
        assert(main_task_node_index == 0);

        hclib::future_t<void> *fa = hclib::async_future([&](){
            return;
        });

        int inner_task_id = 0;
        hclib::future_t<void> *fb = hclib::async_future([&](){
            fa->wait();
            hclib::async([&](){
                hclib_worker_state *ws = current_ws();
                hclib_task_t *task = (hclib_task_t*) ws->curr_task;
                inner_task_id = task->task_id;
            });
        });
        assert(ds_precede(fa->corresponding_task_id,fb->corresponding_task_id) == true);
        assert(ds_precede(fa->corresponding_task_id,inner_task_id) == true);

        printf("all tests passsed int test 4 \n");

        // end of hclib
    });

    return 0;
}

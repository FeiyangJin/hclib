#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 

    hclib::launch(deps, 1, [&]() {
        std::vector<hclib::promise_t<void>*> pv;
        int index = 0;
        void* step1;
        void* step2;
        int task1;
        int task2;
        for(int i=0; i<3; i++){
            pv.push_back(new hclib::promise_t<void>());
            hclib::async([&](){
                if(index == 0){
                    task1 = get_current_task_id();
                    step1 = (void*) get_current_step_node();
                }
                else if(index == 1){
                    task2 = get_current_task_id();
                    step2 = (void*) get_current_step_node();
                }
                pv.at(index)->end_put();
            });
            index++;
        }
        for(auto i = pv.begin(); i != pv.end(); i++){
            (*i)->get_future()->wait();
        }

        void* c_step = (void*) get_current_step_node();
        int c = get_current_task_id();
        assert(ds_precede(step1,c_step,task1,c) == true);
        assert(ds_precede(step2,c_step,task2,c) == true);

        hclib::async([=](){
            hclib_worker_state *ws = current_ws();
            hclib_task_t *task = (hclib_task_t*) ws->curr_task;
            printf("current finish is %d \n",task->current_finish->node_in_dpst->index);
        });
        // ds_print_table();
        // printDPST();
        printf("all tests passed in test 7 \n");
        ds_hclib_ready(false);
        // end of hclib
    });

    return 0;
}

#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 
    std::string const nodeTypes[5] = {"Root","Finish","Async","Future","Step"};

    hclib::launch(deps, 1, [&]() {
        // tests reachability with promise
        hclib::promise_t<int> *p = new hclib::promise_t<int>();
        void* b_step;
        int b_id;
        void* pre_b_step;
        void* useless_step;
        int useless_id;
        
        hclib::future_t<void> *fa = hclib::async_future([&](){
            p->get_future()->wait();
            hclib::async([&](){
                void* current_step = (void*)get_current_step_node();
                int cid = get_current_task_id();
                assert(ds_precede(b_step,current_step,b_id,cid));
                assert(ds_precede(pre_b_step,current_step,0,cid));
                assert(ds_precede(useless_step,current_step,useless_id,cid) == false);
            });
            return;
        });

        hclib::async([&](){
            useless_step = (void*)get_current_step_node();
            useless_id = get_current_task_id();
        });

        pre_b_step = (void*)get_current_step_node();
        hclib::future_t<void> *fb = hclib::async_future([&](){
            b_id = get_current_task_id();
            b_step = (void*) get_current_step_node();
            p->put(5);
            return;
        });

        fa->wait();
        fb->wait();


        // more tests
        hclib::promise_t<int> *q = new hclib::promise_t<int>();
        void* current_step;
        void* step1;
        int id_1;
        hclib::async([&](){
            step1 = (void*) get_current_step_node();
            id_1 = get_current_task_id();
            q->get_future()->wait();
            current_step = (void*) get_current_step_node();
            assert(ds_precede(useless_step,current_step,useless_id,id_1) == false);
        });

        current_step = (void*)get_current_step_node();
        assert(ds_precede(step1,current_step,id_1,0) == false);

        hclib::finish([&](){
            hclib::async([&](){
                useless_step = (void*)get_current_step_node();
                useless_id = get_current_task_id();
            });

            hclib::async([&](){
                q->put(100);
            });
        });
        

        printf("all tests passed in test 5 \n");
        // end of hclib
    });

    return 0;
}

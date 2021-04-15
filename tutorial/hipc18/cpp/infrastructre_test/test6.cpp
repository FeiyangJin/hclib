#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 
    std::string const nodeTypes[5] = {"Root","Finish","Async","Future","Step"};

    hclib::launch(deps, 1, [&]() {
        //ds_print_table();

        // test reachability
        // lsa's nt changes during blocking
        hclib::promise_t<int> *p = new hclib::promise_t<int>();

        void* a_step;
        void* b_step;

        hclib::future_t<void> *fa = hclib::async_future([&](){
            a_step = (void*) get_current_step_node();
            return;
        });

        hclib::future_t<void> *fb = hclib::async_future([&](){
            b_step = (void*) get_current_step_node();
            return;
        });


        std::vector<void*> useless_steps;
        std::vector<int> useless_tasks;
        std::vector<void*> useful_steps;
        std::vector<int> useful_tasks;
        hclib::future_t<void> *fc = hclib::async_future([&](){
            fa->wait();
            hclib::future_t<void> *fd = hclib::async_future([&](){
                int cid = get_current_task_id();
                void* d_step = (void*) get_current_step_node();
                assert(ds_precede(a_step,d_step,fa->corresponding_task_id,cid));

                p->get_future()->wait();
                d_step = (void*) get_current_step_node();
                assert(ds_precede(a_step,d_step,fa->corresponding_task_id,cid));
                assert(ds_precede(b_step,d_step,fb->corresponding_task_id,cid) == false);
                for(int i=0; i<useless_steps.size(); i++){
                    assert(ds_precede(useful_steps.at(i),d_step,useful_tasks.at(i),cid));
                    assert(ds_precede(useless_steps.at(i),d_step,useless_tasks.at(i),cid) == false);
                }
                return;
            });

            hclib::finish([&](){
                for(int i=0; i<100; i++){
                    hclib::async([&](){
                        useless_steps.push_back((void*)get_current_step_node());
                        useless_tasks.push_back(get_current_task_id());
                    });
                }
            });
            fb->wait();
            fd->wait();
        });

        hclib::finish([&](){
            for(int i=0; i<100; i++){
                hclib::async([&](){
                    useful_steps.push_back((void*)get_current_step_node());
                    useful_tasks.push_back(get_current_task_id());
                });
            }
        });

        p->put(5);
        fc->wait();
        

        printf("all tests passed in test 6 \n");
        // end of hclib
    });

    return 0;
}

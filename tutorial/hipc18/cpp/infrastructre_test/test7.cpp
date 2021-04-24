#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

typedef struct access_info_test{
    int task_id;
    tree_node* node_in_dpst;
} access_info_test;

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 

    hclib::launch(deps, 1, [&]() {

        // this test will pass 
        // volatile int b = 3;
        // hclib::async([&](){
        //     b = 4;
        // });
        // b = 5;


        // this test will pass
        // hclib::async([&](){
        //     printf("inside async \n");
        //     volatile int a = 1;
        //     hclib::async([&](){
        //         printf("second async \n");
        //         a = 10;
        //     });
        //     a = 20;
        //     //printf("continuation \n");
            
        // });


        // no race
        volatile int c = 1;
        hclib::future_t<void> *fa = hclib::async_future([&](){
            c = 2;
        });
        fa->wait();
        c = 3;

        // no race 2
        hclib::promise_t<int> *p = new hclib::promise_t<int>();
        int d = 2;
        hclib::async([&](){
            d = 10;
            hclib::async([&](){
                p->get_future()->wait();
                d = 30;
            });
            d = 20;
            p->put(7);
        });

        // no race 3
        int e = 10;
        hclib::finish([&](){
            hclib::async([&](){
                e = 20;
            });
        });
        e = 30;

        printf("all tests passed in test 7 \n");
        //printDPST();
        ds_hclib_ready(false);
        // end of hclib
    });

    return 0;
}

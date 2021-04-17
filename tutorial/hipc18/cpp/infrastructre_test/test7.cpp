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
    std::string const nodeTypes[5] = {"Root","Finish","Async","Future","Step"};

    hclib::launch(deps, 1, [&]() {

        hclib::async([](){
            printf("inside async \n");
            int a = 1;
            int b = a;
            int c = b;
        });

        printf("all tests passed in test 7 \n");
        ds_hclib_ready(false);
        // end of hclib
    });

    return 0;
}

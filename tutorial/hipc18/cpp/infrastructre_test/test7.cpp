#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 

    hclib::launch(deps, 1, [&]() {
        hclib::async([&](){
            // step 5
        });

        hclib::async([&](){
            hclib::async([&](){
                // step 11
            });
        });

        hclib::async([&](){
            // step 14
        });

        printf("all tests passed in test 7 \n");
        ds_hclib_ready(false);
        // end of hclib
    });

    return 0;
}

#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 

    hclib::launch(deps, 1, [&]() {
        test_check_read();
        
        ds_hclib_ready(true);
        int a = 10;

        ds_hclib_ready(false);
        hclib::async([&](){
            ds_hclib_ready(true);
            a = 30;
            ds_hclib_ready(false);
        });
        ds_hclib_ready(true);

        a = 20;
        printf("all tests passed in test 8 \n");

        ds_print_check_read_count();
        ds_print_check_write_count();
        ds_hclib_ready(false);
        // end of hclib
    });

    return 0;
}

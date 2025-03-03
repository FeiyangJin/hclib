// Adapted from DRB165-taskdep4-orig-omp50-yes.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB165-taskdep4-orig-omp50-yes.c

#include "hclib_cpp.h"
#include <stdio.h>

int main(int argc, char **argv) {
    int x = 0, y = 2;

    char const *deps[] = { "system" };

    hclib::launch(deps, 1, [&]() {
        ds_hclib_ready(true);

        // Dynamically allocated promises inside launch
        hclib::promise_t<void>* px = new hclib::promise_t<void>();

        hclib::finish([&]() {

            // First child task: x++
            hclib::async([&]() {
                x++;
                px->put();  // Mark x as updated
            });

            // Second child task: y--
            hclib::async([&]() {
                y--;
            });

            // First taskwait: Ensures x is updated before printing
            px->get_future()->wait();

            printf("x=%d\n", x);
            printf("y=%d\n", y);

            // Second taskwait: Ensures all tasks are done before returning
            hclib::finish([&]() {});

        }); // hclib::finish ensures all async tasks complete

        // Clean up dynamic memory
        delete px;
    });

    return 0;
}

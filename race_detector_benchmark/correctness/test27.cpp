// Adapted from DRB167-taskdep4-orig-omp50-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB167-taskdep4-orig-omp50-no.c

#include "hclib_cpp.h"
#include <stdio.h>

int main(int argc, char **argv) {
    int x = 0, y = 2;

    char const *deps[] = { "system" };

    hclib::launch(deps, 1, [&]() {
        ds_hclib_ready(true);

        // Dynamically allocated promises inside launch
        hclib::promise_t<void>* px = new hclib::promise_t<void>();
        hclib::promise_t<void>* py = new hclib::promise_t<void>();

        hclib::finish([&]() {

            // First child task: x++
            hclib::async([&]() {
                x++;
                px->put();  // Mark x as updated
            });

            // Second child task: y = y - x (depends on x)
            hclib::async([&]() {
                px->get_future()->wait();  // Wait for x to be updated
                y = y - x;
                py->put();  // Mark y as updated
            });

            // First taskwait: Ensures x is updated before printing
            px->get_future()->wait();

            printf("x=%d\n", x);

            // Second taskwait: Ensures y is updated before printing
            py->get_future()->wait();

            printf("y=%d\n", y);

        }); // hclib::finish ensures all async tasks complete

        // Clean up dynamic memory
        delete px;
        delete py;
    });

    return 0;
}

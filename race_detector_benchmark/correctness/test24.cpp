// Adapted from DRB136-taskdep-mutexinoutset-orig-yes.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB136-taskdep-mutexinoutset-orig-yes.c

#include "hclib_cpp.h"
#include <stdio.h>

int main(int argc, char **argv) {
    int a, b, c, d;

    char const *deps[] = { "system" };

    hclib::launch(deps, 1, [&]() {
        ds_hclib_ready(true);

        // Dynamically allocated promises inside launch
        hclib::promise_t<void>* p_a = new hclib::promise_t<void>();
        hclib::promise_t<void>* p_b = new hclib::promise_t<void>();
        hclib::promise_t<void>* p_c = new hclib::promise_t<void>();

        hclib::finish([&]() {

            // Task to set c = 1
            hclib::async([&]() {
                c = 1;
                p_c->put();
            });

            // Task to set a = 2
            hclib::async([&]() {
                a = 2;
                p_a->put();
            });

            // Task to set b = 3
            hclib::async([&]() {
                b = 3;
                p_b->put();
            });

            // Task to update c += a
            hclib::async([&]() {
                p_a->get_future()->wait();
                c += a;
            });

            // Task to update c += b
            hclib::async([&]() {
                p_b->get_future()->wait();
                c += b;
            });

            // Task to set d = c
            hclib::async([&]() {
                p_c->get_future()->wait();
                d = c;
            });
        }); // hclib::finish ensures all async tasks complete

        // Clean up dynamic memory
        delete p_a;
        delete p_b;
        delete p_c;
    });

    return 0;
}

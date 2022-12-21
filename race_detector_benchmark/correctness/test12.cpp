// Adapted from DRB079-taskdep3-orig-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB079-taskdep3-orig-no.c

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int i=0, j, k;
    hclib::promise_t<void> *p = new hclib::promise_t<void>();

    hclib::finish([&](){

        hclib::async([&](){
            // sleep(3);
            i = 1;
            p->put();
        });

        hclib::async([&](){
            p->get_future()->wait();
            j = i;
        });

        hclib::async([&](){
            p->get_future()->wait();
            k = i;
        });

    });

    printf("j is %d, k is %d \n",j,k);
    printf("all tests passsed in test12 \n");
    // end of hclib
  });
  
  return 0;
}
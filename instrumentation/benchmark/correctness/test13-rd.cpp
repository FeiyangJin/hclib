// Adapted from DRB079-taskdep3-orig-no.c, but this one **dose** have a race
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB079-taskdep3-orig-no.c
// Author: Feiyang Jin
// Email: fjin35@gatech.edu

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int i=0, j, k;
    hclib::promise_t<void> *p = new hclib::promise_t<void>();

    hclib::async([&](){
        p->put();
        i = 1;
    });

    hclib::async([&](){
        p->get_future()->wait();
        j = i;
    });

    printf("all tests passsed in test13 \n");
    // end of hclib
  });
  
  return 0;
}
// Adapted from DRB174-non-sibling-taskdep-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB174-non-sibling-taskdep-no.c
// Author: Feiyang Jin
// Email: fjin35@gatech.edu

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int a = 0;
    hclib::promise_t<void> *p = new hclib::promise_t<void>();

    hclib::promise_t<void> *q = new hclib::promise_t<void>();

    hclib::async([&](){
        hclib::async([&](){
            a++;
            q->put();
        });
        q->get_future()->wait();
        p->put();
    });

    hclib::async([&](){
        p->get_future()->wait();
        hclib::async([&](){
            a++;
        });
    });

    printf("all tests passsed in test20 \n");

  });
  
  return 0;
}
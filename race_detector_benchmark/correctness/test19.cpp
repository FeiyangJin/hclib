// Adapted from DRB173-non-sibling-taskdep-yes.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB173-non-sibling-taskdep-yes.c

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
        });
        p->put();
    });

    hclib::async([&](){
        p->get_future()->wait();
        hclib::async([&](){
            a++;
        });
    });

    printf("all tests passsed in test19 \n");

  });
  
  return 0;
}
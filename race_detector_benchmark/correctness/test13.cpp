// Adapted from DRB078-taskdep2-orig-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB078-taskdep2-orig-no.c
// Author: Feiyang Jin
// Email: fjin35@gatech.edu

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int i=0;
    hclib::promise_t<void> *p = new hclib::promise_t<void>();

    hclib::finish([&](){

      hclib::async([&](){
          sleep(3);
          i = 1;
          p->put();
      });

      hclib::async([&](){
          p->get_future()->wait();
          i = 2;
      });
    });

    assert(i == 2);
    printf("all tests passsed in test13 \n");
    // end of hclib
  });
  
  return 0;
}
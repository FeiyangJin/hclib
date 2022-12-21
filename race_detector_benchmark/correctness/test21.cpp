// Adapted from DRB175-non-sibling-taskdep2-yes.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB175-non-sibling-taskdep2-yes.c

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int a = 0;

    for(int i=0; i<3; i++){
      // ds_hclib_ready(false);
      hclib::promise_t<void> *p = new hclib::promise_t<void>();
      // ds_hclib_ready(true);

      hclib::async([&](){
        a++;
        p->put();
      });
    }

    printf("all tests passsed in test21 \n");

  });
  
  return 0;
}
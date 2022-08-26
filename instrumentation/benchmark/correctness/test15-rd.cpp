// Adapted from DRB132-taskdep4-orig-omp45-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB132-taskdep4-orig-omp45-no.c
// Author: Feiyang Jin
// Email: fjin35@gatech.edu

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int x = 0, y = 2;
    hclib::promise_t<void> *p = new hclib::promise_t<void>();

    hclib::finish([&](){

        hclib::async([&](){
            x ++ ;
            p->put();
        });

        hclib::async([&](){
            y --;
        });

        p->get_future()->wait();

        printf("x=%d\n",x);

    });

    printf("y=%d\n",y);
    
    printf("all tests passsed in test15 \n");
    // end of hclib
  });
  
  return 0;
}
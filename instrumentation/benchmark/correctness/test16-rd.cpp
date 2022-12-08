// Adapted from DRB133-taskdep5-orig-omp45-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB133-taskdep5-orig-omp45-no.c
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
    hclib::promise_t<void> *q = new hclib::promise_t<void>();

    hclib::async([&](){
        x ++ ;
        p->put();
    });

    hclib::async([&](){
        p->get_future()->wait();
        y = y - x;
        q->put();
    });

    p->get_future()->wait();

    printf("x=%d\n",x);

    q->get_future()->wait();

    printf("y=%d\n",y);
    
    printf("all tests passsed in test16 \n");

  });
  
  return 0;
}
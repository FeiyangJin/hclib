// Adapted from DRB176-fib-taskdep-no.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB176-fib-taskdep-no.c
// Author: Feiyang Jin
// Email: fjin35@gatech.edu
// TODO: this one needs more testing because HCLIB lambda may cause problems

#include "hclib_cpp.h"
#include <unistd.h>

int fib(int n) {

    ds_set_write_new_section(true);
    int i = 0;
    int j = 0;
    int s = 0;
    if (n < 2){
        return n;
    }
    
    hclib::promise_t<void> *p = new hclib::promise_t<void>();
    hclib::promise_t<void> *q = new hclib::promise_t<void>();
    hclib::promise_t<void> *t = new hclib::promise_t<void>();

    ds_set_write_new_section(false);


    hclib::async([&i,&p,n](){
        i = fib(n-1);
        p->put();
    });


    hclib::async([&j,&q,n](){
        j = fib(n-2);
        q->put();
    });


    hclib::async([&](){
        p->get_future()->wait();
        q->get_future()->wait();

        s = i + j;

        t->put();
    });

    t->get_future()->wait();
    return i + j;

// #pragma omp task shared(i) depend(out : i)
//   i = fib(n - 1);
// #pragma omp task shared(j) depend(out : j)
//   j = fib(n - 2);
// #pragma omp task shared(i, j) depend(in : i, j)
//   s = i + j;
// #pragma omp taskwait
//   return i + j;
}

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int n = 5;
    if (argc > 1){
        n = atoi(argv[1]);
    }

    fib(n);
    printf("all tests passsed in test22 \n");

  });
  
  return 0;
}
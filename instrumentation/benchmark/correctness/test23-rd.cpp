// Adapted from DRB177-fib-taskdep-yes.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB177-fib-taskdep-yes.c
// Author: Feiyang Jin
// Email: fjin35@gatech.edu
// TODO: this one needs more testing because HCLIB lambda may cause problems

#include "hclib_cpp.h"
#include <unistd.h>

int fib(int n) {
    int i = 0;
    int j = 0;
    int s = 0;
    if (n < 2){
        return n;
    }
    
    hclib::promise_t<void> *p = new hclib::promise_t<void>();
    hclib::promise_t<void> *q = new hclib::promise_t<void>();
    hclib::promise_t<void> *t = new hclib::promise_t<void>();

    void* step1;
    void* step2;
    int task1;
    int task2;

    hclib::async([&](){
        i = fib(n-1);
        p->put();
    });

    hclib::async([&](){
        j = fib(n-2);
        q->put();
    });

    hclib::async([&](){
        p->get_future()->wait();

        s = i + j;

        t->put();
    });

    q->get_future()->wait();
    t->get_future()->wait();

    return i + j;
}

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int n = 3;
    if (argc > 1){
        n = atoi(argv[1]);
    }

    fib(n);
    printf("all tests passsed in test23 \n");

  });
  
  return 0;
}
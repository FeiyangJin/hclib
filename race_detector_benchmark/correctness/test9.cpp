// Adapted from DRB001-antidep1-orig-yes.c
// https://github.com/LLNL/dataracebench/blob/master/micro-benchmarks/DRB001-antidep1-orig-yes.c

#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    ds_hclib_ready(true);

    int len = 5;
    int a[len];

    for (int i=0; i<len; i++){
        a[i] = i;
    }

    for(int i = 0; i < len; i ++){
        hclib::async([&a,i](){
            a[i]=a[i+1]+1;
        });
    }
    printf("all tests passsed in test9\n");
    // end of hclib
  });
  
  return 0;
}
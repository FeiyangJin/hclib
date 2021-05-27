#include "hclib_cpp.h"
#include <inttypes.h>
#include <unordered_map>
#define THRESHOLD 10

using namespace std;
// unordered_map<int,int> memorization;

uint64_t fib_serial(uint64_t n) {
    if (n < 2) return n;
    return fib_serial(n-1) + fib_serial(n-2);
}

uint64_t fib_async_finish(uint64_t n) {
  ds_hclib_ready(true);
  ds_promise_task(true);

  // bool in_memo = memorization.find(n) != memorization.end();
  // if(in_memo){
  //   return memorization.at(n);
  // }
  
  if (n < THRESHOLD) {
    int result = fib_serial(n);
    return result;
  }

  //uint64_t x, y;
  ds_hclib_ready(false);
  hclib::promise_t<uint64_t> *x = new hclib::promise_t<uint64_t>();
  hclib::promise_t<uint64_t> *y = new hclib::promise_t<uint64_t>();

    ds_hclib_ready(false);
    hclib::async([&]() {
      uint64_t value1 = fib_async_finish(n-1);
      ds_hclib_ready(true); 
      x->put(value1);
      ds_hclib_ready(false);
    });
    
      
    ds_hclib_ready(false);
    uint64_t value2 = fib_async_finish(n-2);
    ds_hclib_ready(true);
    y->put(value2);


  int value = x->get_future()->wait() + y->get_future()->wait();
  // memorization.insert(std::pair<int,int>(n,value));
  return value;
}



int main(int argc, char** argv) {
  uint64_t n = argc>1?atoi(argv[1]) : 40;

  char const *deps[] = { "system" }; 
  hclib::launch(deps, 1, [&]() {
    long start = hclib_current_time_ms();
    ds_hclib_ready(true);
    
    uint64_t result = fib_async_finish(n);

    ds_hclib_ready(false);
    
    long end = hclib_current_time_ms();
    long dur = ((double)(end-start))/1000;
    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Time = %f \n",dur);
  });

  return 0;
}
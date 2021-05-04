#include "hclib_cpp.h"
#include <inttypes.h>

#define THRESHOLD 10

uint64_t fib_serial(uint64_t n) {
    if (n < 2) return n;
    return fib_serial(n-1) + fib_serial(n-2);
}

uint64_t fib_async_finish(uint64_t n) {
  if (n < THRESHOLD) { 
    return fib_serial(n);
  }

  //uint64_t x, y;
  hclib::promise_t<uint64_t> *x = new hclib::promise_t<uint64_t>();
  hclib::promise_t<uint64_t> *y = new hclib::promise_t<uint64_t>();

    //printf("before async \n");
    hclib::async([&]() {
      //printf("  first line of async \n"); 
      uint64_t value1 = fib_async_finish(n-1);
      x->put(value1);
    });
    
    uint64_t value2 = fib_async_finish(n-2);
    y->put(value2);

  //return (value1 + value2);
  return (x->get_future()->wait() + y->get_future()->wait());
}



int main(int argc, char** argv) {
  uint64_t n = argc>1?atoi(argv[1]) : 40;

  char const *deps[] = { "system" }; 
  hclib::launch(deps, 1, [&]() {
    // sequential execution
    long start = hclib_current_time_ms();

    //uint64_t result = fib_serial(n);

    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    //printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    //printf("Sequential Time = %f \n \n",dur);


    // async finish execution
    start = hclib_current_time_ms();

    uint64_t result = fib_async_finish(n);

    end = hclib_current_time_ms();
    dur = ((double)(end-start))/1000;
    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Async finish Time = %f \n \n",dur);
  });
  return 0;
}
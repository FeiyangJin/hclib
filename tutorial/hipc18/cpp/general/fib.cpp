#include "hclib_cpp.h"
#include <inttypes.h>

#define THRESHOLD 10

uint64_t fib_serial(uint64_t n) {
    if (n < 2) return n;
    return fib_serial(n-1) + fib_serial(n-2);
}


uint64_t fib(uint64_t n) {
  if (n < THRESHOLD) {
    return fib_serial(n);
  } 

  hclib::future_t<uint64_t>* f1 = hclib::async_future([=]() { 
    uint64_t x = fib(n - 1);
    return x;
  });

  uint64_t y = fib(n - 2);

  return y + f1->wait();
}


uint64_t fib_async_finish(uint64_t n) {
  if (n < THRESHOLD) { 
    return fib_serial(n);
  }

  uint64_t x, y;

  hclib::finish([&]() {
    hclib::async([&x, n]() {
      x = fib_async_finish(n-1);
    });
    y = fib_async_finish(n-2);
  }); 
  return (x + y);
}



int main(int argc, char** argv) {
  uint64_t n = argc>1?atoi(argv[1]) : 40;

  char const *deps[] = { "system" }; 
  hclib::launch(deps, 1, [&]() {
    long start = hclib_current_time_ms();

    uint64_t result = fib(n);

    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;

    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Futures Time = %f \n \n",dur);


    // sequential execution
    start = hclib_current_time_ms();

    result = fib_serial(n);

    end = hclib_current_time_ms();
    dur = ((double)(end-start))/1000;
    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Sequential Time = %f \n \n",dur);


    // async finish execution
    start = hclib_current_time_ms();

    result = fib_async_finish(n);

    end = hclib_current_time_ms();
    dur = ((double)(end-start))/1000;
    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Async finish Time = %f \n \n",dur);
  });
  return 0;
}
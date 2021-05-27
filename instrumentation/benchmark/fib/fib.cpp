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
#ifdef RACE_DETECTION
  ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
  ds_promise_task(true);
#endif

  // bool in_memo = memorization.find(n) != memorization.end();
  // if(in_memo){
  //   return memorization.at(n);
  // }
  
  if (n < THRESHOLD) {
    int result = fib_serial(n);
    return result;
  }

  //uint64_t x, y;
#ifdef RACE_DETECTION
  ds_hclib_ready(false);
#endif
  hclib::promise_t<uint64_t> *x = new hclib::promise_t<uint64_t>();
  hclib::promise_t<uint64_t> *y = new hclib::promise_t<uint64_t>();

#ifdef RACE_DETECTION
    ds_hclib_ready(false);
#endif
    hclib::async([&]() {
      uint64_t value1 = fib_async_finish(n-1);
#ifdef RACE_DETECTION
      ds_hclib_ready(true); 
#endif
      x->put(value1);
#ifdef RACE_DETECTION
      ds_hclib_ready(false);
#endif
    });
    
      
#ifdef RACE_DETECTION
    ds_hclib_ready(false);
#endif
    uint64_t value2 = fib_async_finish(n-2);
#ifdef RACE_DETECTION
    ds_hclib_ready(true);
#endif
    y->put(value2);


  int value = x->get_future()->wait() + y->get_future()->wait();
  // memorization.insert(std::pair<int,int>(n,value));
  return value;
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

#ifdef RACE_DETECTION
    ds_hclib_ready(false);
#endif
    
    end = hclib_current_time_ms();
    dur = ((double)(end-start))/1000;
    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Async finish Time = %f \n",dur);
#ifdef RACE_DETECTION
    printf("DPST height is: %d \n", get_dpst_height());
    printf("cache size is %d \n",ds_get_cache_size());
    printf("number of task is %d \n",get_task_id_unique());
    printf("number of nt join %d \n", get_nt_count());
    printf("number of tree joins %d \n", ds_get_tree_join_count());
#endif
  });

  return 0;
}

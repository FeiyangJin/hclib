#include "hclib_cpp.h"
#include <inttypes.h>
#include <unordered_map>
#define THRESHOLD 10

// llvm-symbolizer --obj=./test.exe 0x401e4d

using namespace std;

uint64_t cache[200001];

uint64_t fib_serial(uint64_t n) {
    if (n < 2) return n;
    return fib_serial(n-1) + fib_serial(n-2);
}

uint64_t fib_async_finish(uint64_t n) {
  if(cache[n] != -1){
    return cache[n];
  }

  if (n < THRESHOLD) {
    uint64_t result = fib_serial(n);
    return result;
  }

  //uint64_t x, y;
  #ifdef RACE_DETECTION
    ds_hclib_ready(false);
  #endif
  hclib::promise_t<uint64_t> *x = new hclib::promise_t<uint64_t>();
  hclib::promise_t<uint64_t> *y = new hclib::promise_t<uint64_t>();

    hclib::async([n,&x]() {
      #ifdef RACE_DETECTION
        ds_hclib_ready(false);
      #endif

      uint64_t value1 = fib_async_finish(n-1);

      #ifdef RACE_DETECTION
        ds_hclib_ready(true);
      #endif
      x->put(value1);
    });

    #ifdef RACE_DETECTION
      ds_hclib_ready(false);
    #endif
    uint64_t value2 = fib_async_finish(n-2);

    #ifdef RACE_DETECTION
      ds_hclib_ready(true);
    #endif     
    y->put(value2);

    uint64_t result = x->get_future()->wait() + y->get_future()->wait();
    cache[n] = result;

    return result;
}



int main(int argc, char** argv) {
  uint64_t n = argc>1?atoi(argv[1]) : 40;

  for(int i = 0; i < n + 1; i++){
    cache[i] = -1;
  }
  // printf("%d \n",cache[n]);

  char const *deps[] = { "system" }; 
  hclib::launch(deps, 1, [&]() {
    long start = hclib_current_time_ms();

    #ifdef RACE_DETECTION
      ds_hclib_ready(true);
    #endif

    uint64_t result = fib_async_finish(n);

    #ifdef RACE_DETECTION
      ds_hclib_ready(false);
    #endif
    
    long end = hclib_current_time_ms();
    long dur = ((double)(end-start))/1000;
    printf("Fibonacci of %" PRIu64 " is %" PRIu64 ".\n", n, result);
    printf("Async finish Time = %ld \n",dur);
    #ifdef RACE_DETECTION
        printf("DPST height is: %d \n", get_dpst_height());
        printf("cache size is %d \n",ds_get_cache_size());
        printf("number of task is %d \n",get_task_id_unique());
        printf("number of nt join %d \n", get_nt_count());
        printf("number of tree joins %d \n", ds_get_tree_join_count());
        ds_print_check_write_count();
        ds_print_check_read_count();
    #endif
  });

  return 0;
}

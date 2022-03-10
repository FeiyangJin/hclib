#include "hclib_cpp.h"
#include <inttypes.h>

using namespace std;

int main(int argc, char** argv) {

  char const *deps[] = { "system" }; 
  hclib::launch(deps, 1, [&]() {

    #ifdef RACE_DETECTION
      ds_hclib_ready(true);
    #endif

    int x = 10;

    hclib::async([&x](){
      // #ifdef RACE_DETECTION
      //   int *p = &x;

      //   ds_hclib_ready(true);
      //   asap_check_write(p,4);
      //   ds_hclib_ready(false);
      // #endif
      x = 70;
    });

    // #ifdef RACE_DETECTION
    //   int *p = &x;

    //   ds_hclib_ready(true);
    //   asap_check_read(p,4);
    //   ds_hclib_ready(false);
    // #endif
    printf("\n x is %d \n \n",x);
    printf("should detect race\n");
    

    // #ifdef RACE_DETECTION
    //     printf("DPST height is: %d \n", get_dpst_height());
    //     printf("cache size is %d \n",ds_get_cache_size());
    //     printf("number of task is %d \n",get_task_id_unique());
    //     printf("number of nt join %d \n", get_nt_count());
    //     printf("number of tree joins %d \n", ds_get_tree_join_count());
    //     ds_print_check_write_count();
    //     ds_print_check_read_count();
    // #endif
  });

  return 0;
}

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

    hclib::finish([&](){
        hclib::async([&x](){
            x = 70;
        });

        int y = x;
        
        hclib::async([&x](){
            x = 80;
        });
    });

    #ifdef RACE_DETECTION
        printf("DPST height is: %d \n", get_dpst_height());
        printf("cache size is %d \n",ds_get_cache_size());
        printf("number of task is %d \n",get_task_id_unique());
        printf("number of nt join %d \n", get_nt_count());
        printf("number of tree joins %d \n", ds_get_tree_join_count());
        ds_print_check_write_count();
        ds_print_check_read_count();
        printDPST();
    #endif

    printf("test2 ends \n");
    
  });

  return 0;
}

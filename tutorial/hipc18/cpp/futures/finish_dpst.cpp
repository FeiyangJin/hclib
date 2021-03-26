#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    hclib_print_current_task_info();

    hclib::future_t<void> *a = hclib::async_future([=]() {
      sleep(1);
      printf("A ");
      hclib_print_current_task_info();
      return;
    });

    hclib::finish([=](){
        hclib::finish([=](){
            printf("finish finish ");
            hclib_print_current_task_info();
        });
        int x = 0;
    });

    printf("Terminating\n");

    //printDSbyset();
    //printf("%s \n", DPST.root == NULL ? "true" : "false");
    printDPST();
  });
  
  return 0;
}

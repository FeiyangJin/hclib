#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    //hclib_print_current_task_info();

    hclib::future_t<void> *a = hclib::async_future([]() {
      sleep(1);
      printf("A ");
      hclib_print_current_task_info();
      hclib::finish([](){

        hclib::finish([](){

            hclib::finish([](){

              hclib::async([](){
                printf("third finish ");
                hclib_print_current_task_info();
              });
            });
        });
      });
      ds_printdsbyset();
      return;
    });
    

    hclib::future_t<void> *b = hclib::async_future([=]() {
      a->wait();
      printf("B ");
      hclib_print_current_task_info();
      ds_printdsbyset();
    });

    a->wait();
    b->wait();
    printf("Terminating\n");
    ds_printdsbyset();
    printDPST();
  });
  
  return 0;
}

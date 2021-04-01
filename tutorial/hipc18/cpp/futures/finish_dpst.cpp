#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {

    int inner_task_id = -1;
    hclib::future_t<void> *a = hclib::async_future([&]() {
      printf("A ");
      hclib_print_current_task_info();
      
      hclib::finish([&](){

        hclib::finish([&](){

            hclib::finish([&](){

              hclib::async([&](){
                printf("third finish ");
                hclib_worker_state *ws = current_ws();
                hclib_task_t *task = (hclib_task_t *)ws->curr_task;
                inner_task_id = task->task_id;
                hclib_print_current_task_info();
              });
            });
        });
      });
      return;
    });
    assert(ds_findSet(a->corresponding_task_id) == ds_findSet(inner_task_id));
    

    hclib::future_t<void> *b = hclib::async_future([=]() {
      a->wait();
      printf("B ");
      hclib_print_current_task_info();
    });

    a->wait();
    b->wait();
    ds_print_all_tasks();
    ds_print_table();
    //ds_printdsbyset();
    printDPST();
  });
  
  return 0;
}

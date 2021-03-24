#include "hclib_cpp.h"
#include <unistd.h>

/*
 * This program is computing the following task graph using async_future
 *
 *       (A)
 *       /\
 *      /  \
 *    (B)  (C)
 *     \    /\
 *      \  /  \
 *       (D)  (E)
 *         \   /
 *          \ /
 *          (F)
 */

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

    hclib::future_t<void> *b = hclib::async_future([=]() {
      a->wait();
      printf("B ");
      hclib_print_current_task_info();
      // hclib::async([=](){
      //   printf("B'  ");
      //   hclib_print_current_task_info();
      // });
      // return;
    });

    hclib::future_t<void> *c = hclib::async_future([=]() {
      a->wait();
      printf("C ");
      hclib_print_current_task_info();
      return;
    });

    hclib::future_t<void> *d = hclib::async_future([=]() {
      sleep(1);
      b->wait();
      c->wait();
      printf("D ");
      hclib_print_current_task_info();
      return;
    });

    hclib::future_t<void> *e = hclib::async_future([=]() {
      c->wait();
      printf("E ");
      hclib_print_current_task_info();

      hclib::async([=](){
        printf("  E' ");
        hclib_print_current_task_info();
      });

      return;
    });

    hclib::future_t<void> *f = hclib::async_future([=]() {
      d->wait();
      e->wait();
      printf("F ");
      hclib_print_current_task_info();
      printDS();
      return;
    });

    f->wait();
    

    printf("Terminating\n");
    //printf("%s \n", DPST.root == NULL ? "true" : "false");
    printDPST();
    printDS();
    //printf("future f corresponding task is: %d \n", f->corresponding_task->task_id);
  });
  
  return 0;
}

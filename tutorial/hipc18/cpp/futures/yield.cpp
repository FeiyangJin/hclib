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
      printf("A ");
      hclib_print_current_task_info();
      return;
    });

    hclib::future_t<void> *b = hclib::async_future([=]() {
      a->wait();
      printf("B ");
      hclib_print_current_task_info();
    });

    hclib::future_t<void> *c = hclib::async_future([=]() {
      a->wait();
      printf("C ");
      hclib_print_current_task_info();
      return;
    });

    hclib::future_t<void> *d = hclib::async_future([=]() {
      b->wait();
      c->wait();
      printf("D ");
      hclib_print_current_task_info();

      hclib::finish([=](){
        hclib::async([=](){
          printf("  d_finish ");
          hclib_print_current_task_info();
        });
      });

      return;
    });

    hclib::future_t<void> *e = hclib::async_future([=]() {
      c->wait();
      printf("E ");
      hclib_print_current_task_info();

      return;
    });

    hclib::future_t<void> *f = hclib::async_future([=]() {
      d->wait();
      e->wait();
      printf("F ");
      hclib_print_current_task_info();
      return;
    });

    f->wait();    

    printf("Terminating\n");

    ds_printdsbyset();
    //printf("%s \n", DPST.root == NULL ? "true" : "false");
    printDPST();
  });
  
  return 0;
}

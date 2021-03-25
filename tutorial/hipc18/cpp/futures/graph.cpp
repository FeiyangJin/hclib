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

      hclib_worker_state *out_ws = current_ws();
      hclib_task_t *out_task = (hclib_task_t *)out_ws->curr_task;
      int out_index = out_task->task_id;
      tree_node *step1 = out_task->node_in_dpst->children_list_head;

      hclib::async([=](){
        printf("  E' ");
        hclib_print_current_task_info();

        hclib_worker_state *in_ws = current_ws();
        hclib_task_t *in_task = (hclib_task_t *)in_ws->curr_task;

        tree_node* both_parent = (tree_node*) ds_get_dpst_node(out_index);
        HASSERT(find_lca(step1,in_task->node_in_dpst)->index == both_parent->index);
      });

      printf("after E' \n");
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

    printDSbyset();
    //printf("%s \n", DPST.root == NULL ? "true" : "false");
    printDPST();
  });
  
  return 0;
}

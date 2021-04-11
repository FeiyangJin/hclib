#include "hclib_cpp.h"
#include <unistd.h>

int main(int argc, char **argv) {
  char const *deps[] = { "system" }; 
  
  hclib::launch(deps, 1, [&]() {
    // test_initilization
    hclib_worker_state *main_ws = current_ws();
    hclib_task_t *main_task = (hclib_task_t *) main_ws->curr_task;
    int task_id = main_task->task_id;
    
    assert(task_id == 0);
    assert(ds_findSet(task_id) == 0);
    assert(ds_getlsa(task_id) == -1);
    assert(ds_ntcounts(task_id) == 0);
    assert(ds_parentid(task_id) == -1);
    assert(ds_taskState(task_id) == 0);

    // test finish join and execution order
    std::vector<int> v;
    hclib::finish([&](){
        for(int i=0; i<10; i++){
            hclib::async([&](){
                v.push_back(i);
            });
        }
    });

    for(int j=0;j<10;j++){
        assert(v.at(j) == j);
        assert(ds_findSet(j) == ds_findSet(0));
    }

    assert(ds_ntcounts(0) == 0);
    assert(ds_getlsa(0) == -1);
    
    // test finish join 2
    int inner_task = -1;

    hclib::future_t<void> *a = hclib::async_future([&]() {
        hclib::finish([&](){
            hclib::finish([&](){
                hclib::async([&](){
                    hclib_worker_state *in_ws = current_ws();
                    hclib_task_t *in_task = (hclib_task_t *)in_ws->curr_task;
                    inner_task = in_task->task_id;
                });
            });
        });

        return;
    });

    assert(ds_parentid(a->corresponding_task_id) == 0);
    assert(ds_parentid(inner_task) == a->corresponding_task_id);
    assert(ds_findSet(a->corresponding_task_id) == ds_findSet(inner_task));
    assert(ds_ntcounts(a->corresponding_task_id) == 0);
    assert(ds_taskState(inner_task) == 3);
    assert(ds_taskState(a->corresponding_task_id) == 2);

    // test non-tree joins and lsa
    int b_task = -1;
    int inner_task2 = -1;
    hclib::future_t<void> *b = hclib::async_future([&]() {
        a->wait();
        hclib_worker_state *ws = current_ws();
        hclib_task_t *task = (hclib_task_t *)ws->curr_task;
        b_task = task->task_id;

        hclib::finish([&](){
            hclib::async([&](){
                ws = current_ws();
                task = (hclib_task_t *)ws->curr_task;
                inner_task2 = task->task_id;
                assert(ds_ntcounts(inner_task2) == 0);
                assert(ds_getlsa(inner_task2) == ds_findSet(b_task));
                assert(ds_ntcounts(ds_findSet(b_task)) == 1);
            });
        });
        assert(ds_findSet(inner_task2) == ds_findSet(b_task));
        assert(ds_getlsa(ds_findSet(inner_task2)) == -1);
        assert(ds_ntcounts(ds_findSet(inner_task2)) == 1);
        return;
    });

    assert(ds_taskState(a->corresponding_task_id) == 3);
    assert(ds_findSet(a->corresponding_task_id) != ds_findSet(b->corresponding_task_id));
    

    // test future joins
    a->wait();
    b->wait();

    assert(task_id == 0);
    assert(ds_findSet(a->corresponding_task_id) == ds_findSet(b->corresponding_task_id));
    assert(ds_findSet(task_id) == ds_findSet(a->corresponding_task_id));
    assert(ds_ntcounts(task_id) == 1);
    assert(ds_getlsa(task_id) == -1);
    
    printf("all tests passed int test 1\n");
    // end of hclib
  });
  
  return 0;
}

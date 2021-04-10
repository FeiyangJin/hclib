#include "hclib_cpp.h"
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    char const *deps[] = { "system" }; 
    std::string const nodeTypes[5] = {"Root","Finish","Async","Future","Step"};

    hclib::launch(deps, 1, [&]() {
        // test finding lca and lca left child
        hclib_worker_state *main_ws = current_ws();
        hclib_task_t *main_task = (hclib_task_t *) main_ws->curr_task;
        tree_node* root = main_task->node_in_dpst;

        assert(root->number_of_child == 1);
        assert(root->children_list_head->is_parent_nth_child == 0);

        tree_node* step1;
        tree_node* step2;
        hclib::async([&](){
            hclib::finish([&](){
                step1 = get_current_step_node();
            }); 
        });

        hclib::async([&](){

            hclib::finish([&](){
                
                hclib::async([&](){
                    step2 = get_current_step_node();
                });
                
            }); 
        });

        tree_node* lca = find_lca(step1,step2);
        tree_node* lca_lc = find_lca_left_child(step1,step2);
        assert(lca->this_node_type == FINISH);
        // printf("lca index: %d, lca left child index: %d, lca left child type: ",lca->index,lca_lc->index);
        // std::cout << nodeTypes[static_cast<int>(lca_lc->this_node_type)] << std::endl;

        // printf("step1 node in dpst: %d \n",step1->index);
        // printf("step2 node in dpst: %d \n",step2->index);
        // printDPST();


        // test getting current step node
        tree_node* step3;
        tree_node* step4;
        tree_node* step5;
        tree_node* step6;
        tree_node* step7;
        hclib::promise_t<int> *p = new hclib::promise_t<int>();

        hclib::future_t<void> *fa = hclib::async_future([&](){
            step3 = get_current_step_node();
            hclib::finish([&](){
                step4 = get_current_step_node();
                hclib::async([&](){
                    step5 = get_current_step_node();
                    p->get_future()->wait();
                    step7 = get_current_step_node();
                });
                step6 = get_current_step_node();
                p->put(5);
            });
        });

        
        fa->wait();
        assert(step3->parent->this_node_type == FUTURE);
        assert(step4->is_parent_nth_child == 0);
        assert(step4->parent->this_node_type == FINISH);
        assert(step5->parent->this_node_type == ASYNC);
        assert(step6->is_parent_nth_child == 2);
        assert(step6->parent->this_node_type == FINISH);
        assert(step7->is_parent_nth_child == 1);
        assert(step7->parent->this_node_type == ASYNC);

        printf("all tests passed int test 3\n");
        // end of hclib
    });

    return 0;
}

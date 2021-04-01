#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();
		hclib::promise_t<int> *B = new hclib::promise_t<int>();

		hclib::future_t<void> *d = hclib::async_future([=](){
			A->get_future()->wait();
			printf("D ");
			hclib_print_current_task_info();
			return;
		});

		hclib::future_t<void> *a = hclib::async_future([=](){
			printf("A ");
			hclib_print_current_task_info();
			A->put(5);
			return;
		});

		hclib::future_t<void> *c = hclib::async_future([=](){
			B->get_future()->wait();
			printf("C ");
			hclib_print_current_task_info();
			return;
		});

		hclib::future_t<void> *b = hclib::async_future([=](){
			printf("B ");
			hclib_print_current_task_info();
			A->get_future()->wait();
			B->put(7);
			return;
		});

		d->wait();
		c->wait();

		printf("waiting is over \n");
		//printDPST();

		assert(A->setter_task_id == a->corresponding_task_id);
		assert(ds_findSet(a->corresponding_task_id) == a->corresponding_task_id);
		assert(ds_findSet(b->corresponding_task_id) == b->corresponding_task_id);
		
		tree_node *a_dpst_node = (tree_node*) ds_get_dpst_node(a->corresponding_task_id);
		tree_node *b_dpst_node = (tree_node*) ds_get_dpst_node(b->corresponding_task_id);
		tree_node *c_dpst_node = (tree_node*) ds_get_dpst_node(c->corresponding_task_id);
		
		assert(a_dpst_node->parent->this_node_type == FINISH);
		assert(c_dpst_node->parent->this_node_type == FINISH);
		assert(ds_ntcounts(b->corresponding_task_id) == 1);

		// printDPST();
		//ds_print_all_tasks();
		//ds_printdsbyset();
		ds_printAll();
	});
	return 0;
}

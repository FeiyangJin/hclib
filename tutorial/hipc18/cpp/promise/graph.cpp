#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();

		hclib::future_t<void> *a = hclib::async_future([=](){
			A->put(5);
			printf("A ");
			hclib_print_current_task_info();
			//ds_printdsbyset();
			//ds_print_all_tasks();
			return;
		});

		hclib::future_t<void> *c = hclib::async_future([=](){
			printf("C ");
			hclib_print_current_task_info();
			return;
		});

		hclib::future_t<void> *b = hclib::async_future([=](){
			c->wait();
			printf("B ");
			hclib_print_current_task_info();
			A->get_future()->wait();
			ds_printdsbyset();
			//ds_print_all_tasks();
			return;
		});

		a->wait();
		b->wait();

		printf("waiting is over \n");
		HASSERT(A->setter_task_id == a->corresponding_task_id);
		HASSERT(ds_findSet(a->corresponding_task_id) == 0);
		HASSERT(ds_findSet(b->corresponding_task_id) == 0);
		
		tree_node *a_dpst_node = (tree_node*) ds_get_dpst_node(a->corresponding_task_id);
		tree_node *b_dpst_node = (tree_node*) ds_get_dpst_node(b->corresponding_task_id);
		tree_node *c_dpst_node = (tree_node*) ds_get_dpst_node(c->corresponding_task_id);
		
		HASSERT(a_dpst_node->parent->corresponding_task_id == 0);
		HASSERT(c_dpst_node->parent->corresponding_task_id == 0);
		HASSERT(b_dpst_node->parent->index == a_dpst_node->index);
		HASSERT(ds_ntcounts(b->corresponding_task_id) > 0);

		//printDPST();
		//ds_print_all_tasks();
		//ds_printdsbyset();
		
		//ds_printAll();
		

	});
	return 0;
}

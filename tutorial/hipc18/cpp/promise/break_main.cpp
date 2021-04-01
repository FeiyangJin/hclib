#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();
        hclib::promise_t<int> *B = new hclib::promise_t<int>();

        hclib::async([=](){
            A->put(5);
        });

        A->get_future()->wait();

        //printDPST();
        ds_print_table();
        ds_print_all_tasks();

        B->put(7);
        B->get_future()->wait();

        ds_print_table();
        ds_print_all_tasks();

        printDPST();
	});
	return 0;
}

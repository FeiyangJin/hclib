#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();

		//hclib::async([=](){
			A->put(5);
		//});
		
		A->get_future()->wait();
		printf("wait is over \n");

		printf("promise A setter is: %d \n",A->setter_task_id);
		printDPST();
		ds_printdsbyset();
	});
	return 0;
}

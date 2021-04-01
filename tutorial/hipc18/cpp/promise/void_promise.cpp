#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<void> *A = new hclib::promise_t<void>();

        A->put(); 
        A->get_future()->wait();
        
        printf("should not blocked \n");
	});
	return 0;
}

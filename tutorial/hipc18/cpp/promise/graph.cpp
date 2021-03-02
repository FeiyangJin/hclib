#include "hclib_cpp.h"

int main(int argc, char **argv){
	hclib::promise_t<int> *A = new hclib::promise_t<int>();
	A->put(5);

	A->get_future()->wait();
	printf("wait is over \n");

	return 0;
}

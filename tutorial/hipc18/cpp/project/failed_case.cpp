#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "hclib_cpp.h"

int main(int argc, char **argv) {

	char const *deps[] = { "system" };
	hclib::launch(deps, 1, []() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();
		hclib::finish([&]() {
			
			int value = 33;

			hclib::async([=]() {
				printf("async \n");
				int result = A->get_future()->wait();
				//assert(result == value);
				printf("result is %d \n", result);
			});

			printf("right before put \n");
			A->put(value);
		});

	});
	return 0;
}
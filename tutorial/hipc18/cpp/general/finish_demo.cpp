#include "hclib_cpp.h"
#include <unistd.h>

// test if "finish" for async_await or just async
int main(int argc, char **argv) {
	char const *deps[] = { "system" };
	hclib::launch(deps, 1, [&]() {
		hclib::finish([&]() {
			/* spawn several async */
			hclib::async([&]() {
				sleep(2);
				printf("1st async \n");
			});

			hclib::async([&]() {
				sleep(2);
				printf("	2nd async \n");
			});

			hclib::async([&]() {
				sleep(2);
				printf("		3rd async \n");
			});

			printf("main thread in parallel \n");
			hclib::promise_t<void> *p = new hclib::promise_t<void>;
			hclib::async_await([=]() {
				sleep(5);
				printf(" async_await \n");
			}, p->get_future());

			p->put();
			
		}); /*end of finish, i.e. wait until all the asyncs inside this finish scope have terminated. */

		printf("\n \n \n main thread continuation \n");

	});
	return 0;
}
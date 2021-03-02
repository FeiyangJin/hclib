#include "hclib_cpp.h"
#include <unistd.h>

/*
 * This program is computing the following task graph using async_future
 *
 *       (A)
 *       /\
 *      /  \
 *    (B)  (C)
 *     \    /\
 *      \  /  \
 *       (D)  (E)
 *         \   /
 *          \ /
 *          (F)
 */

int main(int argc, char* argv[]){

	char const *deps[] = { "system" }; 
	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<void> *A = new hclib::promise_t<void>();
		hclib::promise_t<void> *B = new hclib::promise_t<void>();
		hclib::promise_t<void> *C = new hclib::promise_t<void>();
		hclib::promise_t<void> *D = new hclib::promise_t<void>();
		hclib::promise_t<void> *E = new hclib::promise_t<void>();
		hclib::promise_t<void> *F = new hclib::promise_t<void>();

		hclib::future_t<void> *af = A->get_future();
		hclib::future_t<void> *bf = B->get_future();
		hclib::future_t<void> *cf = C->get_future();
		hclib::future_t<void> *df = D->get_future();
		hclib::future_t<void> *ef = E->get_future();
		hclib::future_t<void> *ff = F->get_future();

		hclib::async([=]() {
			A->put(); printf("A \n");
		}); //Task A

		//async_await(lambda,future_1,future_2,.....,future_n)
		//Task is kept on hold until all the future objects in the parameter
		//list are ready with values inside them
		hclib::async_await([=] () {
			printf("	B \n"); B->put();
		}, af);

		hclib::async_await([=]() {
			printf("	C \n"); C->put();
		}, af);

		hclib::async_await([=]() {
			printf("		D \n"); D->put();
		}, bf, cf);

		hclib::async_await([=]() {
			printf("		E \n"); E->put();
		}, cf);

		hclib::async_await([=]() {
			printf("			F \n"); F->put();
		}, df, ef);

		printf("before the wait \n");
		af->wait();
		bf->wait();
		cf->wait();
		df->wait();
		ef->wait();
		ff->wait();

		free(A);
		free(B);
		free(C);
		free(D);
		free(E);
		free(F);

        // hclib_promise_t *p = hclib_promise_create();

        // //put data
        // int data = 10;
        // void *data_pointer = &data;
        // hclib_promise_put(p, data_pointer);

        // //get data from its future
        // hclib_future_t *future = hclib_get_future_for_promise(p);
        // void *get_pointer = hclib_future_get(future);
        // printf("the data is %d \n",*(int *)get_pointer);

        // hclib_promise_free(p);

		/*
		hclib::promise_t<int> *p = new hclib::promise_t<int>();

		//put data
		int data = 10;
		p->put(data);

		//get data from its future
		hclib::future_t<int> *future = p->get_future();
		int result = future->get();
		printf("the data is %d \n", result);

		future->wait();

		hclib_promise_free(p);
		*/
	});
	return 0;

}
#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();
        hclib::promise_t<int> *B = new hclib::promise_t<int>();

		hclib::future_t<void> *a = hclib::async_future([=](){
			printf("A ");
			hclib_print_current_task_info();

            hclib::async([=](){
                printf("B ");
                hclib_print_current_task_info();
                A->get_future()->wait();

                hclib::async([=](){
                    printf("C ");
                    hclib_print_current_task_info();
                    B->get_future()->wait();

                    // hclib::async([=](){
                    //     printf("E ");
                    //     hclib_print_current_task_info();
                    // });

                    printf("    C continuation");
                    hclib_print_current_task_info();
                });

                printf("    B continuation ");
                hclib_print_current_task_info();
            });

            printf("    A continuation ");
            hclib_print_current_task_info();
            A->put(5);

            hclib::async([=](){
                printf("D ");
                hclib_print_current_task_info();
            });

            B->put(3);

			return;
		});

        a->wait();
        //ds_printdsbyset();
	});
	return 0;
}

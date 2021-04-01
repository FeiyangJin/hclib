#include "hclib_cpp.h"

int main(int argc, char **argv){

	char const *deps[] = { "system" }; 
  
  	hclib::launch(deps, 1, [&]() {
		hclib::promise_t<int> *A = new hclib::promise_t<int>();
        hclib::promise_t<int> *B = new hclib::promise_t<int>();
        hclib::promise_t<int> *C = new hclib::promise_t<int>();
        
        printf("1 ");

        hclib::async([=](){
            printf("2 ");
            hclib::async([=](){
                printf("3 ");
                //ds_print_all_tasks();
                hclib::async([=](){
                    printf("4 " );
                    A->get_future()->wait();
                    printf("8 ");
                });
                printf("5 ");
                B->get_future()->wait();
                printf("11 ");
                
            });
            printf("6 ");
            A->get_future()->wait();
            printf("9 ");
        });

        printf("7 ");
        A->put(5);
        printf("10 ");
        B->put(7);

        printf("\n");
        ds_print_all_tasks();
        ds_print_table();
	});
	return 0;
}

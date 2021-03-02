#include "hclib_cpp.h"
#include <vector>

void work(int n){
    //n++;
    printf("inside work with index: %d \n",n);
}

void entrypoint(void *arg){
    std::vector<hclib::promise_t<void>*> allPromises;
    for(int i=0; i<50; ++i){
        allPromises.push_back(new hclib::promise_t<void>());
        int index = allPromises.size()-1;
        hclib::async([=,&allPromises]() {
            work(index);
            allPromises.at(index)->put();
        });
    }

    for(hclib::promise_t<void> *p : allPromises){
        p->get_future()->wait();
    }

    // for(int j = 0; j < allPromises.size(); j++){
    //     printf("    wait on %d promise \n",j);
    //     printf("    is it satisfied ? %d \n", allPromises.at(j)->satisfied);
    //     allPromises.at(j)->get_future()->wait();
    // }

    printf("ending all tasks \n");
}

int main(int argc, char **argv){
    char const *deps[] = { "system" };
    hclib_launch(entrypoint, NULL, deps, 1);

	return 0;
}
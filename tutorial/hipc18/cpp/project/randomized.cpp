#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <atomic>
#include "hclib_cpp.h"


int N = 1000;
int P = 2000; //number of promise
int T = 3; // branching factor
double W = 0.8; // probability to wait for a promise
//int future_id = 0;
std::atomic<int> future_id (0);

std::vector<hclib::promise_t<void>> allPromises;
double *data;

class PromiseCollection{
    public:
        std::vector<hclib::promise_t<void>> promiseList;
        PromiseCollection(std::vector<hclib::promise_t<void>> &list);
        PromiseCollection split();
        std::vector<hclib::promise_t<void>> getPromises();
};

PromiseCollection::PromiseCollection(std::vector<hclib::promise_t<void>> &list){
    this->promiseList = list;
}

std::vector<hclib::promise_t<void>> PromiseCollection::getPromises(){
    return this->promiseList;
}

PromiseCollection PromiseCollection::split(){
    std::vector<hclib::promise_t<void>> splitOff;
    int list_size = this->promiseList.size();

    for(int i = list_size/2; i < list_size; i++){
        splitOff.push_back(this->promiseList.at(i));
    }

    std::vector<hclib::promise_t<void>> newpromiseList;
    for(int i=0; i<list_size/2; i++){
        newpromiseList.push_back(this->promiseList.at(i));
    }
    this->promiseList = newpromiseList;
    //this->promiseList.erase(this->promiseList.begin()+list_size/2, this->promiseList.begin()+list_size);

    return PromiseCollection(splitOff);
}

void printArgument(){
    printf(" N: %d \n P: %d \n T: %d \n W: %f \n", N, P, T, W);
}

void fulfill(PromiseCollection promises){
    srand (time(NULL));
    // int id = future_id.load(std::memory_order_relaxed);
    // printf("task id is: %d \n",id);
    // id = id + 1;
    // future_id.store(id,std::memory_order_relaxed);
    // printf("promises size %ld \n",promises.getPromises().size());

    std::vector<hclib::future_t<void>*> tasks;
    for(int t = 0; t < T && promises.getPromises().size() > 1; t++){
        PromiseCollection pc = promises.split();
        hclib::future_t<void>* single_task = hclib::async_future([=]() {
            fulfill(pc);
            return;
        });
        
        tasks.push_back(single_task);
    }

    // wait for a promise with probability W
    if(((double) rand() / (RAND_MAX)) < W){
        int i = rand() % 100; // 0 <= i <= 99
        //printf("wait \n");
        allPromises.at(i).get_future()->wait();
    }

    // do some work by adding data
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            data[i] += data[j] * (((double) rand() / (RAND_MAX)) + 1);
        }
    }

    // complete promises and wait all tasks
    for(auto p : promises.getPromises()){
        if(!p.satisfied){
            p.put();
        }
    }

    for(auto cf : tasks){
        cf->get();
    }
}


int main(int argc, char ** argv) {
    if(argc < 5){
        printf("Using default argument \n");
    }
    else{
        N = atoi(argv[0]);
        P = atoi(argv[1]);
        T = atoi(argv[2]);
        W = std::stod(argv[3]);
    }
    printArgument();
    data = new double[N];

    for(int i=0; i<P; i++){
        allPromises.push_back(hclib::promise_t<void>());
    }

    std::vector<hclib::promise_t<void>> deepCopy = allPromises;
    PromiseCollection promises = PromiseCollection(deepCopy);

    char const *deps[] = { "system" };
    printf("start tasks \n");
    hclib::launch(deps, 1, [&]() {
        fulfill(promises);
    });
    printf("end tasks \n");

    return 0;
}
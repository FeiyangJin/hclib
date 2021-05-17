#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <random>
#include "hclib_cpp.h"


int N = 1000;
int P = 500; //number of promise
int T = 2; // branching factor
double W = 0.5; // probability to wait for a promise
unsigned long R = 1234567;

std::vector<hclib::promise_t<void>*> *allPromises = new std::vector<hclib::promise_t<void>*>();
double *data;

int task_count = 0;

void printArgument(){
    printf(" N: %d \n P: %d \n T: %d \n W: %f \n", N, P, T, W);
}


int next(int bits, unsigned long seed){
    srand((seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1));
    return (int)(seed >> (48 - bits));
}

long nextlong(unsigned long seed){
    return ((long)next(32,seed) << 32) + next(32,seed);
}

double nextdouble(unsigned long seed){
    return (((long)next(26,seed) << 27) + next(27,seed)) / (double)(1L << 53);
}


void fulfill(int start, int end, long seed){
    srand(seed);

    int initial_end = end;
    std::vector<hclib::future_t<void>*> tasks;


    ds_hclib_ready(false);
    for (int t = 0; t < T && (end - start > 1); t++) {
        int mid = (int)((end + start) / 2);
        long newSeed = nextlong(seed);
        hclib::future_t<void>* single_task = hclib::async_future([&]() {
            ds_hclib_ready(false);
            fulfill(mid,end,newSeed);
            return;
        });
        ds_hclib_ready(false);
        end = mid;
        
        tasks.push_back(single_task);
    }

    // wait for a promise with probability W
    if(((double) rand() / (RAND_MAX)) < W){
        int i = (int) (seed % P);
        if (i < 0) i += P;
        ds_hclib_ready(true);
        allPromises->at(i)->get_future()->wait();
        ds_hclib_ready(false);
    }

    // waiting
    ds_hclib_ready(true);
    data[0] = 1;
    data[1] = 2;
    // for(int i = 0; i < 50; i++){
    //     for(int j = 0; j < 50; j++){
    //         data[i] += data[j];
    //     }
    // }
    
    // complete promises and wait all tasks
    for(int i = start; i <= end; i++){
        if(allPromises->at(i)->satisfied == false){
            allPromises->at(i)->put();
        }
    }

    for(auto cf : tasks){
        cf->wait();
    }
    ds_hclib_ready(false);
}


int main(int argc, char ** argv) {
    if(argc < 5){
        printf("Using default argument \n");
    }
    // else{
    //     N = atoi(argv[0]);
    //     P = atoi(argv[1]);
    //     T = atoi(argv[2]);
    //     W = std::stod(argv[3]);
    // }
    printArgument();
    data = new double[N];

    for(int i=0; i<P; i++){
        allPromises->push_back(new hclib::promise_t<void>());
    }

    char const *deps[] = { "system" };
    printf("start tasks \n");
    long start = hclib_current_time_ms();

    hclib::launch(deps, 1, [&]() {
        fulfill(0,P-1,R);
        ds_hclib_ready(false);
    });

    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    printf("end tasks, duration is: %f \n",dur);

    for(auto p=allPromises->begin(); p!=allPromises->end(); p++){
        assert((*p)->satisfied);
    }
    printf("all promise satisfied \n");
    printf("cache size is %d \n",ds_get_cache_size());
    printf("number of task is %d \n",get_task_id_unique());
    printf("number of nt join %d \n", get_nt_count());

    return 0;
}
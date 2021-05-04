#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <random>
#include "hclib_cpp.h"


int N = 2000;
int P = 4000; //number of promise
int T = 3; // branching factor
double W = 0.7; // probability to wait for a promise
unsigned long R = 1234567;

std::vector<hclib::promise_t<void>*> *allPromises = new std::vector<hclib::promise_t<void>*>();
double *data;

int task_count = 0;

class PromiseCollection{
    public:
        std::vector<hclib::promise_t<void>*> *promiseList;
        PromiseCollection(std::vector<hclib::promise_t<void>*> *list);
        PromiseCollection* split();
        std::vector<hclib::promise_t<void>*>* getPromises();
};

PromiseCollection::PromiseCollection(std::vector<hclib::promise_t<void>*> *list){
    this->promiseList = list;
}

std::vector<hclib::promise_t<void>*>* PromiseCollection::getPromises(){
    return this->promiseList;
}

PromiseCollection* PromiseCollection::split(){
    std::vector<hclib::promise_t<void>*>* splitOff = new std::vector<hclib::promise_t<void>*>();
    int list_size = this->promiseList->size();

    for(int i = list_size/2; i < list_size; i++){
        splitOff->push_back(this->promiseList->at(i));
    }

    std::vector<hclib::promise_t<void>*>* newpromiseList = new std::vector<hclib::promise_t<void>*>();
    for(int i=0; i<list_size/2; i++){
        newpromiseList->push_back(this->promiseList->at(i));
    }
    this->promiseList = newpromiseList;
    //this->promiseList.erase(this->promiseList.begin()+list_size/2, this->promiseList.begin()+list_size);

    PromiseCollection *result = new PromiseCollection(splitOff);
    return result;
}

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


void fulfill(int start, int end,long seed){
    //printf("start %d, end %d \n",start,end);
    //srand(time(NULL));
    srand(seed);

    //printf("start is %d, end is %d \n",start,end);
    int initial_end = end;
    std::vector<hclib::future_t<void>*> tasks;

    //while(end - start > 1){
    for (int t = 0; t < T && (end - start > 1); t++) {
        int mid = (int)((end + start) / 2);
        long newSeed = nextlong(seed);
        hclib::future_t<void>* single_task = hclib::async_future([&]() {
            fulfill(mid,end,newSeed);
            return;
        });
        end = mid;
        
        tasks.push_back(single_task);
    }

    // wait for a promise with probability W
    if(((double) rand() / (RAND_MAX)) < W){
        int i = (int) (seed % P);
        if (i < 0) i += P;
        // can only wait for promise index behind us;
        //int i = end + ( std::rand() % (P - end) );
        
        // int satisfy_count = 0;
        // for(int j=0; j<allPromises->size(); j++){
        //     if(allPromises->at(j)->satisfied == true){
        //         satisfy_count++;
        //     }
        // }
        // printf("    wait on promise %d, total %d satisfied \n",i,satisfy_count);
        allPromises->at(i)->get_future()->wait();
    }

    // do some work by adding data
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            data[i] += data[j];
        }
    }

    // complete promises and wait all tasks
    for(int i = start; i <= end; i++){
        if(allPromises->at(i)->satisfied == false){
            allPromises->at(i)->put();
        }
    }

    for(auto cf : tasks){
        cf->wait();
    }
}


// void fulfill(PromiseCollection* promises,unsigned long seed){
//     //srand(time(NULL));
//     printf("promise size is: %lu, seed is %lu \n",promises->getPromises()->size(),seed);
//     //srand(seed);

//     std::vector<hclib::future_t<void>*> tasks;
//     for(int t = 0; t < T && promises->getPromises()->size() > 1; t++){
//         PromiseCollection* pc = promises->split();
//         long newSeed = seed;

//         hclib::future_t<void>* single_task = hclib::async_future([&]() {
//             fulfill(pc,newSeed);
//             return;
//         });
        
//         tasks.push_back(single_task);
//     }

//     double lower_bound = 0;
//     double upper_bound = 1;
//     std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
//     std::default_random_engine re;
//     double a_random_double = unif(re);

//     // wait for a promise with probability W
//     if(a_random_double < W){
//         int i = 0 + ( std::rand() % ( P - 0 + 1 ) );
//         // lower_bound = 0;
//         // upper_bound = P - 1;
//         // std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
//         // std::default_random_engine re;
//         // int i = (int)unif(re);
//         // int i = (int) (seed % P);
//         // if (i < 0) i += P;
        
//         int satisfy_count = 0;
//         for(int j=0; j<allPromises->size(); j++){
//             if(allPromises->at(j)->satisfied == true){
//                 satisfy_count++;
//             }
//         }
//         printf("    wait on promise %d, total %d satisfied \n",i,satisfy_count);
//         allPromises->at(i)->get_future()->wait();
//     }

//     // do some work by adding data
//     for(int i = 0; i < N; i++){
//         for(int j = 0; j < N; j++){
//             data[i] += data[j];
//         }
//     }

//     // complete promises and wait all tasks
//     for(auto p : *(promises->getPromises())){
//         if(!p->satisfied){
//             p->put();
//         }
//     }

//     for(auto cf : tasks){
//         cf->wait();
//     }
// }


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

    PromiseCollection* promises = new PromiseCollection(allPromises);

    char const *deps[] = { "system" };
    printf("start tasks \n");
    long start = hclib_current_time_ms();

    hclib::launch(deps, 1, [&]() {
        fulfill(0,P-1,R);
    });
    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    printf("end tasks, duration is: %f \n",dur);

    for(auto p=allPromises->begin(); p!=allPromises->end(); p++){
        assert((*p)->satisfied);
    }
    printf("all promise satisfied \n");

    return 0;
}
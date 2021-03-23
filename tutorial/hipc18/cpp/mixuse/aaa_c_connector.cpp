#include <cstdlib>

#include "aaa_c_connector.h"
#include "aaa.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

static DisjointSet *ds = new DisjointSet();
void addSet(int set_index){
    ds->addSet(set_index);
}

int findSet(int element){
    return ds->Find(element);
}

void unionSet(int a, int b){
    ds->Union(a,b);
}

void merge(int a, int b){
    ds->mergeBtoA(a,b);
}

void printAll(){
    ds->printds();
}

void printElementSets(int *element_array){
    
}

#ifdef __cplusplus
}
#endif
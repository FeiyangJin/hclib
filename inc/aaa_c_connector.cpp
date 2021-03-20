#include <cstdlib>

#include "aaa_c_connector.h"
#include "aaa.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

// Inside this "extern C" block, I can implement functions in C++, which will externally 
//   appear as C functions (which means that the function IDs will be their names, unlike
//   the regular C++ behavior, which allows defining multiple functions with the same name
//   (overloading) and hence uses function signature hashing to enforce unique IDs),

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

void printElementSets(int *element_array){
    
}

#ifdef __cplusplus
}
#endif
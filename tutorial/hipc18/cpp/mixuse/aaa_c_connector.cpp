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

void addnt(int task, int nt_task_id){
    ds->addnt(task,nt_task_id);
}

int ntcounts(int task_id){
    return ds->ntcounts(task_id);
}

int getlsa(int task_id){
    return ds->getlsa(task_id);
}

void setlsa(int task_id, int lsa){
    ds->setlsa(task_id,lsa);
}

void printAll(){
    ds->printds();
}

void printdsbyset(){
    ds->printdsbyset();
}

void printElementSets(int *element_array){
    
}

#ifdef __cplusplus
}
#endif
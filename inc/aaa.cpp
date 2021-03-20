#include "aaa.h"

DisjointSet::DisjointSet(){

}

void DisjointSet::addSet(int set_index){
    parent[set_index] = set_index;
    rank[set_index] = 0;
}

void DisjointSet::addSets(vector<int> const &universe){
    for (int i: universe)
    {
        parent[i] = i;
        rank[i] = 0;
    }
}

int DisjointSet::Find(int k){
    // if `k` is not the root
    if (parent[k] != k)
    {
        // path compression
        parent[k] = Find(parent[k]);
    }

    return parent[k];
}

void DisjointSet::Union(int a, int b){
    int x = Find(a);
    int y = Find(b);

    if (x == y) {
        return;
    }

    if (rank[x] > rank[y]) {
        parent[y] = x;
    }
    else if (rank[x] < rank[y]) {
        parent[x] = y;
    }
    else {
        parent[x] = y;
        rank[y]++;
    }
}

void printSets(vector<int> const &universe, DisjointSet &ds){
    for (int i: universe) {
        printf("element %d is in set %d \n",i,ds.Find(i));
    }
    printf("\n");
}
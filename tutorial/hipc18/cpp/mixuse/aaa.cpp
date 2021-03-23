#include "aaa.h"

DisjointSet::DisjointSet(){

}

void DisjointSet::addSet(int set_index){
    parent[set_index] = set_index;
    rank[set_index] = 0;
    vector<int> nontreejoins;
    nt[set_index] = nontreejoins;
    lsa[set_index] = -1;
}

void DisjointSet::addSets(vector<int> const &universe){
    for (int i: universe)
    {
        parent[i] = i;
        rank[i] = 0;
    }
}

int DisjointSet::Find(int k){
    if (parent[k] != k)
    {
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

void DisjointSet::mergeBtoA(int a, int b){
    int Sa = Find(a);
    int Sb = Find(b);

    if (Sa == Sb) {
        return;
    }

    // union nt
    vector<int> a_nt = nt.at(a);
    vector<int> b_nt = nt.at(b);
    for(auto i = b_nt.begin(); i != b_nt.end(); ++i){
        a_nt.push_back(*i);
    }
    nt[a] = a_nt;
    
    // union Sb into Sa
    parent[Sb] = Sa;

}

void DisjointSet::printds(){
    for (std::pair<int, int> element: parent) {
        printf("%d is in set: %d \n", element.first, Find(element.first));
    };
}


void printSets(vector<int> const &universe, DisjointSet &ds){
    for (int i: universe) {
        printf("element %d is in set %d \n",i,ds.Find(i));
    }
    printf("\n");
}
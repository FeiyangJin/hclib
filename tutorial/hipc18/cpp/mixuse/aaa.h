#ifndef AAA_H
#define AAA_H
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

// A class to represent a disjoint set
class DisjointSet
{
    unordered_map<int, int> parent;
 
    // stores the depth of trees
    unordered_map<int, int> rank;

public:
    DisjointSet();

    // add single set
    void addSet(int set_index);

    // add multiple sets
    void addSets(vector<int> const &universe);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);
 
    // Perform Union of two subsets
    void Union(int a, int b);
};
 
void printSets(vector<int> const &universe, DisjointSet &ds);

#endif
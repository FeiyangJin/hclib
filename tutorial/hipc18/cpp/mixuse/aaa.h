#ifndef AAA_H
#define AAA_H
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

// A class to represent a disjoint set
class DisjointSet
{
    // a map from the task_index to set
    unordered_map<int, int> parent;
 
    // stores the depth of trees
    unordered_map<int, int> rank;

    // stores the non-tree joins for each set
    unordered_map<int,vector<int>> nt;

    // stores the least-significant ancesotr for each set
    unordered_map<int,int> lsa;

public:
    DisjointSet();

    // add single set
    void addSet(int set_index);

    // add multiple sets
    void addSets(vector<int> const &universe);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);
 
    // Perform Union of two subsets based on rank
    void Union(int a, int b);

    // Peform Union of two subsets where Set A is the new parent
    void mergeBtoA(int a, int b);

    // print all sets
    void printds();
};


void printSets(vector<int> const &universe, DisjointSet &ds);
#endif
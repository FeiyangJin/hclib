#ifndef AAA_H
#define AAA_H
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

enum task_state{
    ACTIVE,
    BLOCKED,
    FINISHED_NOT_JOINED,
    JOINED
};

class hclib_task
{
    public:
        int task_id;
        int parent_id;
        void *node_in_dpst;
        void *task_address; // could be null if the task is freed
        task_state this_task_state;

        hclib_task(int task_id, int parent_id, void *node_in_dpst, void *task_address, task_state state);
};

// A class to represent a disjoint set
class DisjointSet
{
    // a map from task_id to task
    unordered_map<int, hclib_task*> all_tasks;

    // a map from task_index to set
    unordered_map<int, int> parent;
 
    // stores the depth of trees
    unordered_map<int, int> rank;

    // stores the non-tree joins for each set
    unordered_map<int,vector<int>> nt;

    // stores the least-significant ancesotr for each set
    unordered_map<int,int> lsa;

public:
    DisjointSet();
    // add task
    void addTask(int task_id, hclib_task *task);

    hclib_task* get_task_info(int task_id);

    // add single set
    void addSet(int set_index);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);
 
    // Perform Union of two subsets based on rank
    void Union(int a, int b);

    // Peform Union of two subsets where Set A is the new parent
    void mergeBtoA(int a, int b);

    // add a task to other's nt
    void addnt(int task, int nt_task_id);

    // return the number of nt for task's set
    int ntcounts(int task_id);

    // get lsa
    int getlsa(int task_id);

    // set the lsa
    void setlsa(int task_id, int lsa);

    // print each item
    void printds();

    // print by set
    void printdsbyset();
};


void printSets(vector<int> const &universe, DisjointSet &ds);
#endif
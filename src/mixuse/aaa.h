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
        void *task_address; // could be null if the task is freed or is empty
        task_state this_task_state;

        hclib_task(int task_id, int parent_id, void *node_in_dpst, void *task_address, task_state state);
};

class hclib_finish
{
    public:
        int finish_id;
        int belong_to_task_id;
        void *node_in_dpst;
        void *finish_address;
        vector<int> task_in_this_finish;

        hclib_finish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address);
};

// A class to represent a disjoint set
class DisjointSet
{
    // a map from finish dpst node id to finish
    unordered_map<int, hclib_finish*> all_finishes;

    // a map from task_id to task
    unordered_map<int, hclib_task*> all_tasks;

    // a map from task_index to set, aka which set the task is currently in
    unordered_map<int, int> parent_aka_setnowin;

    unordered_map<int, int> rank;

    // stores the non-tree joins for each set
    unordered_map<int,vector<int>> nt;

    // stores the least-significant ancesotr for each set
    unordered_map<int,int> lsa;

public:
    DisjointSet();

    void print_table();

    void end_finish_merge(int finish_id);

    void add_task_to_finish(int finish_id, int task_id);

    // add finish
    void addFinish(int finish_id, hclib_finish *finish);

    // add task
    void addTask(int task_id, hclib_task *task);

    hclib_task* get_task_info(int task_id);

    void update_task_parent(int task_id, int new_parent_id);

    void update_task_dpst_node(int task, void *new_node);

    void update_task_state(int task_id, task_state new_state);

    void break_previous_steps(int task_id, int task_id_for_previous_steps);

    void print_all_tasks();

    // add single set
    void addSet(int set_index);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);

    // Peform Union of two subsets where Set A is the new parent
    void mergeBtoA(int a, int b);

    void Union(int a, int b);

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
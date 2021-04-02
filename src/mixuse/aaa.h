#ifndef AAA_H
#define AAA_H
#include <iostream>
#include <vector>
#include <unordered_map>
#include "assert.h"
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

    // rank for each set
    unordered_map<int, int> rank;

    // stores the non-tree joins for each set
    unordered_map<int,vector<int>> nt;

    // stores the least-significant ancesotr for each set
    unordered_map<int,int> lsa;

public:
    DisjointSet();

    // finish functions
    void end_finish_merge(int finish_id);

    void add_task_to_finish(int finish_id, int task_id);

    void addFinish(int finish_id, hclib_finish *finish);

    // task functions
    void addTask(int task_id, hclib_task *task);

    hclib_task* get_task_info(int task_id);

    void update_task_parent(int task_id, int new_parent_id);

    void update_task_dpst_node(int task, void *new_node);

    void update_task_state(int task_id, task_state new_state);

    void break_previous_steps(int task_id, int task_id_for_previous_steps);

    void print_all_tasks();

    // disjoint set functions
    void addSet(int set_index);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);

    // Peform Union of two subsets where a called get(b)
    void mergeBtoA(int a, int b);

    void Union(int a, int b);

    // add a set to other's nt
    void addnt(int task, int nt_task_id);

    // return the number of nt for task's set
    int ntcounts(int task_id);

    // return the number of nt for the task
    int ntcounts_task(int task_id);

    // get lsa of the set task is now in
    int getlsa(int task_id);

    // get lsa of the task
    int getlsa_task(int task_id);

    // set lsa of the set task is now in
    void setlsa(int task_id, int lsa);

    // print each item
    void printds();

    // print by set
    void printdsbyset();

    // print in table format
    void print_table();
};

#endif
#ifndef AAA_H
#define AAA_H
#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include "assert.h"
using namespace std;

enum node_type_cpp{
    ROOT,
    FINISH,
    ASYNC,
    FUTURE,
    STEP
};

typedef struct tree_node_cpp{
    int index;
    int corresponding_task_id;
    enum node_type_cpp this_node_type;
    int depth;
    int number_of_child;
    int is_parent_nth_child;
    struct tree_node_cpp *parent;
    struct tree_node_cpp *children_list_head;
    struct tree_node_cpp *children_list_tail;
    struct tree_node_cpp *next_sibling;
} tree_node_cpp;

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

typedef struct nt_info{
    int task_id;
    tree_node_cpp *last_node_before_this_nt;
} nt_info;

typedef struct lsa_info{
    int task_id;
    tree_node_cpp *last_node_reachable_in_lsa;
} lsa_info;

typedef struct set_info{
    int set_id;
    tree_node_cpp *query_node_in_current_set;
} set_info;

// A class to represent a disjoint set
class DisjointSet
{
    // a map from finish dpst node id to finish
    unordered_map<int, hclib_finish*> all_finishes;

    // a map from task_id to task
    unordered_map<int, hclib_task*> all_tasks;

    // a map from task_index to set, aka which set the task is currently in
    unordered_map<int, set_info> parent_aka_setnowin;

    // rank for each set
    unordered_map<int, int> rank;

    unordered_map<int,vector<nt_info>> nt;

    unordered_map<int, lsa_info> lsa;


public:
    DisjointSet();

    // finish functions
    void end_finish_merge(int finish_id, tree_node_cpp* query_node);

    void add_task_to_finish(int finish_id, int task_id);

    void addFinish(int finish_id, hclib_finish *finish);

    // task functions
    void addTask(int task_id, hclib_task *task, tree_node_cpp *last_node_reachable_in_parent);

    hclib_task* get_task_info(int task_id);

    void update_task_dpst_node(int task, void *new_node);

    void update_task_state(int task_id, task_state new_state);

    void print_all_tasks();

    // disjoint set functions
    void addSet(int set_index);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);

    set_info find_helper(int k);

    // Peform Union of two subsets where A called get(B)
    void mergeBtoA(int a, int b, tree_node_cpp* query_node_in_A);

    // add a set to other's nt
    void addnt(int task, int nt_task_id,tree_node_cpp* last_node_before_nt);

    // return the number of nt for task's set
    int ntcounts(int task_id);

    // return the number of nt for the task
    int ntcounts_task(int task_id);

    void print_nt(int set_id);

    // get lsa of the set task is now in
    int getlsa(int task_id);

    // get lsa of the task
    int getlsa_task(int task_id);

    // set lsa of the set task is now in
    void setlsa(int task_id, lsa_info lsa);

    // print each item
    void printds();

    // print by set
    void printdsbyset();

    // print in table format
    void print_table();

    // reachability queries
    bool precede(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b);
    bool visit(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b, set<int> visited);

    tree_node_cpp* find_lca_left_child_cpp(tree_node_cpp* node1, tree_node_cpp* node2);

    int find_task_node_index(int task_id);
};

#endif
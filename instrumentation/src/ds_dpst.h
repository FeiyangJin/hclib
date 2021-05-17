#ifndef DS_DPST_H
#define DS_DPST_H
#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include "assert.h"
#include "struct_def.h"

using namespace std;

typedef struct cache_key{
  int task_a;
  int task_b;

  cache_key(int a, int b){
    this->task_a = a;
    this->task_b = b;
  }

  bool operator==(const cache_key& key) const
  {
      return task_a == key.task_a && task_b == key.task_b;
  }
} cache_key;

class CacheHashFunction {
  public:
    size_t operator()(const cache_key& key) const
    {
      //Cantor pairing function:
      //(a + b) * (a + b + 1) / 2 + a;
      int result = (key.task_a + key.task_b) * (key.task_a + key.task_b + 1) / 2 + key.task_a;
      return result;
    }
};

// A class to represent a disjoint set
class DisjointSet
{
    int tree_join_count = 0;

    // a map from finish dpst node id to finish
    unordered_map<int, hclib_finish*> all_finishes;

    unordered_map<cache_key,tree_node_cpp*,CacheHashFunction> cache;

    // a map from task_id to task
    unordered_map<int, hclib_task> all_tasks;

    // a map from task_index to the information of current set
    unordered_map<int, set_info*> parent_aka_setnowin;

public:
    DisjointSet();

    int get_tree_join_count();
    
    // finish functions
    void end_finish_merge(int finish_id, tree_node_cpp* query_node);

    void add_task_to_finish(int finish_id, int task_id);

    void addFinish(int finish_id, hclib_finish *finish);

    // task functions
    void addTask(int task_id, hclib_task task, tree_node_cpp *last_node_reachable_in_parent);

    hclib_task get_task_info(int task_id);

    void update_task_dpst_node(int task, void *new_node);

    void update_task_state(int task_id, task_state new_state);

    void print_all_tasks();

    // disjoint set functions
    void addSet(int set_index);
 
    // Find the root of the set in which element `k` belongs
    int Find(int k);

    set_info* find_helper(int k);

    // Peform Union of two subsets where A called get(B)
    void mergeBtoA(int a, int b, tree_node_cpp* query_node_in_A, bool update_inline_finish);

    // add a set to other's nt
    void addnt(int task, int nt_task_id,tree_node_cpp* last_node_before_nt);

    // return the number of nt for task's set
    int ntcounts(int task_id);

    // return the number of nt for the task
    int ntcounts_task(int task_id);

    void print_nt(int set_id);

    // get lsa of the set task is now in
    int getlsa(int task_id);

    lsa_info getlsa_info(int task_id);

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
    bool visit(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b, unordered_set<int> visited);

    tree_node_cpp* find_lca_left_child_cpp(tree_node_cpp* node1, tree_node_cpp* node2);

    int find_task_node_index(int task_id);

    bool precede_dpst(tree_node_cpp* node1, tree_node_cpp* node2);

    int get_cache_size();

    bool easy_precede(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b);
};

#endif
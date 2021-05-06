#ifndef STRUCT_DEF
#define STRUCT_DEF

#include <vector>

typedef struct access_info{
    int task_id;
    void* node_in_dpst;
} access_info;

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
        std::vector<int> task_in_this_finish;

        hclib_finish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address);
};

typedef struct nt_info{
    int task_id;
    tree_node_cpp *last_node_before_this_nt;
} nt_info;

inline bool operator<(const nt_info& lhs, const nt_info& rhs)
{
  return lhs.task_id < rhs.task_id;
}

typedef struct lsa_info{
    int task_id;
    tree_node_cpp *last_node_reachable_in_lsa;
} lsa_info;

typedef struct set_info{
    int set_id;
    tree_node_cpp *query_node_in_current_set;
} set_info;

#endif
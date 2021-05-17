#ifndef STRUCT_DEF
#define STRUCT_DEF

#include <vector>

typedef struct access_info{
    int task_id;
    void* node_in_dpst;

    inline access_info operator=(access_info t){
        this->task_id = t.task_id;
        this->node_in_dpst = t.node_in_dpst;
        return t;
    };
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
    int inline_finish_step;
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
        int belong_to_finish_id;
        task_state this_task_state;

        hclib_task(){

        };
        
        hclib_task(int task_id, int parent_id, void *node_in_dpst, void *task_address, int belong_to_finish_id, task_state state){
            this->task_id = task_id;
            this->parent_id = parent_id;
            this->node_in_dpst = node_in_dpst;
            this->task_address = task_address;
            this->belong_to_finish_id = belong_to_finish_id;
            this->this_task_state = state;
        }
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

    inline lsa_info operator=(lsa_info a){
        task_id = a.task_id;
        last_node_reachable_in_lsa = a.last_node_reachable_in_lsa;
        return a;
    };
} lsa_info;

typedef struct set_info{
    int set_id;
    int rank;
    lsa_info lsa;
    std::vector<nt_info>* nt;

    set_info(int id, int rank, lsa_info li, std::vector<nt_info>* nontree){
        this->set_id = id;
        this->rank = rank;
        this->lsa = li;
        this->nt = nontree;
    };

    inline set_info operator=(set_info a) {
        set_id = a.set_id;
        rank = a.rank;
        lsa = a.lsa;
        nt = a.nt;
        return a;
    };
} set_info;

#endif
#include "ds_dpst.h"

int DisjointSet::get_tree_join_count(){
    return this->tree_join_count;
}

void DisjointSet::add_task_to_finish(int finish_id, int task_id){
    this->all_finishes.at(finish_id)->task_in_this_finish.push_back(task_id);
}

void DisjointSet::addFinish(int finish_id, hclib_finish *finish){
    // this->all_finishes.insert({finish_id, finish});
    this->all_finishes.insert(robin_hood::pair<int, hclib_finish*>(finish_id, finish));
}

void DisjointSet::end_finish_merge(int finish_id, tree_node_cpp* query_node){
    hclib_finish *finish = this->all_finishes.at(finish_id);
    int finish_owner_task = finish->belong_to_task_id;
    for(vector<int>::iterator i = finish->task_in_this_finish.begin(); i != finish->task_in_this_finish.end(); i++){
        int sub_task_id = *i;
        this->update_task_state(sub_task_id,JOINED);
        this->mergeBtoA(finish_owner_task, sub_task_id, query_node, false);
    }
}

/**
 * @brief  Add a struct hclib_task to map and update lsa 
 * @note   
 * @param  task_id: task_id
 * @param  *task: the struct hclib_task
 * @retval None
 */
void DisjointSet::addTask(int task_id, hclib_task task, tree_node_cpp *last_node_reachable_in_parent){
    this->all_tasks.insert({task_id, task});
    // this->all_tasks.insert(robin_hood::pair<int, hclib_task>(task_id, task));

    if(task_id == 0){
        return;
    }

    set_info* task_set_info = find_helper(task_id);
    set_info* parent_set_info = find_helper(task.parent_id);
    
    // set least_significant_ancestor (LSA)
    // if parent has non-tree joins, lsa = parent
    // else lsa = parent.lsa
    if(parent_set_info->nt->size() > 0){
        assert(last_node_reachable_in_parent != NULL);

        task_set_info->lsa.last_node_reachable_in_lsa = last_node_reachable_in_parent;
        task_set_info->lsa.task_id = task.parent_id;

        std::vector<nt_info>* x = new vector<nt_info>();
        set_info* parent_set = find_helper(task.parent_id);
        for(auto nt_join = parent_set->nt->begin(); nt_join != parent_set->nt->end(); nt_join++){
            x->push_back(*nt_join);
        }
        task_set_info->lsa.lsa_nt = x;
    }
    else{
        task_set_info->lsa = parent_set_info->lsa;
    }
};

hclib_task DisjointSet::get_task_info(int task_id){
    return this->all_tasks.at(task_id);
}

void DisjointSet::update_task_dpst_node(int task_id, void *new_node){
    this->all_tasks.at(task_id).node_in_dpst = new_node;
}


DisjointSet::DisjointSet(){
    this->all_finishes.reserve(100);
    this->all_tasks.reserve(5000);
    this->parent_aka_setnowin.reserve(5000);
    this->cache.reserve(650000);
}

void DisjointSet::addSet(int task_index){
    lsa_info null_lsa = lsa_info();
    set_info* new_set = new set_info(task_index,0,null_lsa,new vector<nt_info>());
    this->parent_aka_setnowin.insert({task_index,new_set});
}

int DisjointSet::get_find_count(){
    printf("size of disjoint set: %d \n", parent_aka_setnowin.size());
    return this->find_count;
}

set_info* DisjointSet::find_helper(int k){
    #ifdef DEBUG
        this->find_count++;
    #endif

    assert(k != -1);
    set_info* node = parent_aka_setnowin.at(k);
    if (node->set_id != k) {
        // Recursively find root and apply path compression
        set_info* root = find_helper(node->set_id);
        node->set_id = root->set_id;  // Direct path to root
        node->rank = root->rank;
        node->lsa = root->lsa;
        node->nt = root->nt;
    }
    return node;
}

int DisjointSet::Find(int k){
    return find_helper(k)->set_id;
}

/**
 * @brief  task a called get(b), merge the two disjoint sets
 * @note   the new set could be a_set or b_set, depends on rank
 * @param  a: task a task_id
 * @param  b: task b task_id
 * @retval None
 */
void DisjointSet::mergeBtoA(int a, int b, tree_node_cpp* query_node, bool update_inline_finish){
    #ifdef DEBUG
        this->tree_join_count++;
    #endif
    
    if(update_inline_finish){
        tree_node_cpp* node = (tree_node_cpp*) this->all_tasks.at(b).node_in_dpst;
        node->inline_finish_step = query_node->is_parent_nth_child;
    }

    set_info* a_set_info = find_helper(a);
    set_info* b_set_info = find_helper(b);

    int a_set = a_set_info->set_id;
    int b_set = b_set_info->set_id;
    int a_rank = a_set_info->rank;
    int b_rank = b_set_info->rank;

    if (a_set == b_set) {
        return;
    }

    // Create new NT vector using reserve for efficiency
    std::vector<nt_info>* new_nt = new std::vector<nt_info>();
    new_nt->reserve(a_set_info->nt->size() + b_set_info->nt->size());
    
    // Use set for deduplication
    std::set<nt_info> unique_nt;
    unique_nt.insert(a_set_info->nt->begin(), a_set_info->nt->end());
    unique_nt.insert(b_set_info->nt->begin(), b_set_info->nt->end());
    
    // Move elements to new vector
    new_nt->assign(unique_nt.begin(), unique_nt.end());

    // set new lsa
    lsa_info new_lsa = a_set_info->lsa;

    // now just join B to A
    // int new_set = a_set;
    // this->parent_aka_setnowin[b_set] = a_set;
    // union two sets
    set_info* new_set;
    if(a_rank > b_rank){
        *b_set_info = *a_set_info;
        new_set = a_set_info;
    }
    else if(a_rank < b_rank){
        *a_set_info = *b_set_info;
        new_set = b_set_info;
    }
    else{
        *b_set_info = *a_set_info;
        a_set_info->rank++;
        new_set = a_set_info;
    }
 
    new_set->nt = new_nt;
    new_set->lsa = new_lsa;
}

void DisjointSet::addnt(int task, int nt_task_id, tree_node_cpp* last_node_before_nt){
    set_info* task_set = find_helper(task);
    task_set->nt->push_back(nt_info(nt_task_id, last_node_before_nt));

    this->all_tasks.at(nt_task_id).this_task_state = JOINED;
}

int DisjointSet::ntcounts(int task_id){
    return find_helper(task_id)->nt->size();
}

int DisjointSet::ntcounts_task(int task_id){
    return -1;
}

lsa_info DisjointSet::getlsa_info(int task_id){
    return find_helper(task_id)->lsa;
}

int DisjointSet::getlsa(int task_id){
    return find_helper(task_id)->lsa.task_id;
}

int DisjointSet::getlsa_task(int task_id){
    return -1;
}

void DisjointSet::setlsa(int task_id, lsa_info new_lsa){
    this->find_helper(task_id)->lsa = new_lsa;
}

string state_string[4] = {"Active", "Blocked", "Finished_not_Joined", "Joined"};

void DisjointSet::print_all_tasks(){
    for (robin_hood::pair<int, hclib_task> element: this->all_tasks) {
        hclib_task task = element.second;
        int task_id = element.first;
        printf("task %d, parent is %d, has %d nt joins, lsa is %d, now in set: %d, task state is: ", 
            task_id, task.parent_id, ntcounts(task_id), getlsa(task_id), Find(task_id));

        int state = static_cast<int>(this->all_tasks.at(task_id).this_task_state);
        std::cout << state_string[state];

        printf("\n");
        // if(this->lsa[task_id].task_id != -1){
        //     assert(this->lsa[task_id].last_node_reachable_in_lsa != NULL);
        //     printf("    lsa last reachable node index is %d \n",this->lsa[task_id].last_node_reachable_in_lsa->index);
        // }
        // else{
        //     printf("\n");
        // }
    }
}

void DisjointSet::print_nt(int set_id){
    vector<nt_info>* ntjoins = find_helper(set_id)->nt;
    printf("set %d has non-tree joins: \n",set_id);
    for(auto join = ntjoins->begin(); join != ntjoins->end(); join++){
        printf("task %d, DPST index of last node before this join %d \n",(*join).task_id,(*join).last_node_before_this_nt->index);
    }
}

void DisjointSet::printds(){
    for (robin_hood::pair<int, set_info*> element: parent_aka_setnowin) {
        printf("%d is in set: %d, lsa is %d \n", element.first, Find(element.first),getlsa(element.first));
        printf("%d has nt joins: ", element.first);
        for(auto item = element.second->nt->begin(); item != element.second->nt->end(); ++item){
            printf("%d ",(*item).task_id);
        }
        printf("\n");
    };

}

void DisjointSet::printdsbyset(){
    unordered_map<int, vector<int>> all_sets;

    for (robin_hood::pair<int, set_info*> element: parent_aka_setnowin) {
        int the_parent = Find(element.first);
        if(all_sets.count(the_parent) > 0){
            all_sets[the_parent].push_back(element.first);
        }
        else{
            vector<int> initial_set;
            initial_set.push_back(element.first);
            all_sets[the_parent] = initial_set;
        }
    };

    for (std::pair<int,vector<int>> the_set: all_sets){
        printf("In set %d, we have elements: ",the_set.first);
        for(auto member = the_set.second.begin(); member != the_set.second.end(); member++){
            printf("%d ",*member);
        }

        set_info* the_set_info = find_helper(the_set.first);

        printf("\n    nt_joins: ");
        for(auto nt_join = the_set_info->nt->begin(); nt_join != the_set_info->nt->end(); nt_join++){
            printf("%d ",(*nt_join).task_id);
        }

        printf("\n      lsa is: %d ", getlsa(the_set.first));

        printf("\n\n");
    }
}

void DisjointSet::print_table(){
    unordered_map<int, vector<int>> all_sets;

    for (robin_hood::pair<int, set_info*> element: parent_aka_setnowin) {
        int the_parent = Find(element.first);
        if(all_sets.count(the_parent) > 0){
            all_sets[the_parent].push_back(element.first);
        }
        else{
            vector<int> initial_set;
            initial_set.push_back(element.first);
            all_sets[the_parent] = initial_set;
        }
    };

    printf("Disjoint Set | Task | NT | LSA \n");

    for (std::pair<int,vector<int>> the_set: all_sets){
        printf("%d", the_set.first);
        
        printf(" | ");

        for(auto member = the_set.second.begin(); member != the_set.second.end(); member++){
            printf("%d,",*member);
        }

        printf(" | ");

        set_info* the_set_info = find_helper(the_set.first);
        for(auto nt_join = the_set_info->nt->begin(); nt_join != the_set_info->nt->end(); nt_join++){
            printf("%d,",(*nt_join).task_id);
        }

        printf(" | ");

        printf("%d", getlsa(the_set.first));

        printf("\n\n");
    }

}

void DisjointSet::update_task_state(int task_id, task_state new_state){
    this->all_tasks[task_id].this_task_state = new_state;
}

int DisjointSet::find_task_node_index(int task_id){
    tree_node_cpp* node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
    return node->index;
}

tree_node_cpp* DisjointSet::find_lca_left_child_cpp(tree_node_cpp* node1, tree_node_cpp* node2){
    tree_node_cpp* node1_last_node = node1;
    tree_node_cpp* node2_last_node = node2;

    while (node1->depth != node2->depth)
    {
        node1_last_node = node1;
        node2_last_node = node2;
        if (node1->depth > node2->depth)
        {
            node1 = node1->parent;
        }
        else{
            node2 = node2->parent;
        }
    }

    while(node1->index != node2->index){
        node1_last_node = node1;
        node2_last_node = node2;
        node1 = node1->parent;
        node2 = node2->parent;
    }; // end

    if(node1_last_node->is_parent_nth_child < node2_last_node->is_parent_nth_child){
        // node1 is to the left of node 2
        return node1_last_node;
    }

    return node2_last_node;
}



#define CACHE ;

/**
 * @brief  check if node1 precedes node2 in dpst
 * @note   
 * @param  node1: 
 * @param  node2: 
 * @retval true if node1 precdes node2 by tree edges 
 */
bool DisjointSet::precede_dpst(tree_node_cpp* node1, tree_node_cpp* node2){    
    if(node1->parent->index == node2->parent->index){
        if(node1->is_parent_nth_child <= node2->is_parent_nth_child){
            return true;
        }
        else{
            return false;
        }
    }
    
    // need to guarantee prev_node is to the left of current_node
    tree_node_cpp* node1_last_node;
    tree_node_cpp* node2_last_node;

    while (node1->depth != node2->depth)
    {
        node1_last_node = node1;
        node2_last_node = node2;
        if (node1->depth > node2->depth)
        {
            node1 = node1->parent;
        }
        else{
            node2 = node2->parent;
        }
    }

    while(node1->index != node2->index){
        node1_last_node = node1;
        node2_last_node = node2;
        node1 = node1->parent;
        node2 = node2->parent;
    }; // end

    if(node1_last_node->is_parent_nth_child < node2_last_node->is_parent_nth_child){
        // node1 is to the left of node 2
        if(node1_last_node->this_node_type == FUTURE || node1_last_node->this_node_type == ASYNC){
            if(node1_last_node->inline_finish_step > 0 && node1_last_node->inline_finish_step <= node2_last_node->is_parent_nth_child){
                return true;
            }
            return false;
        }
        else{
            return true;
        }
    }

    return false;
}

int cachehit = 0;
int cachemiss = 0;
int samestepcount = 0;
int totalprecede = 0;
int visitmax = 0;
int visittotalsize = 0;
int visitcount = 0;
int visitmin = 1000;
robin_hood::unordered_set<int> visited;

bool DisjointSet::precede(tree_node_cpp* step_a, tree_node_cpp* step_b, unsigned int task_a, unsigned int task_b){
    // if(step_a->index == step_b->index){
    //     samestepcount ++;
    //     return true;
    // }
    #ifdef DEBUG
        totalprecede ++;
    #endif

    #ifdef CACHE
        unsigned long int key = (((unsigned long int)task_a) << 23) | task_b; 
        unsigned int& in_cache = cache[key]; // if not in cache, it will be inserted
        if(in_cache >= (unsigned) step_a->index){
            #ifdef DEBUG
                cachehit ++;
            #endif
            return true;
        }
        #ifdef DEBUG 
            else{ cachemiss ++; }
        #endif
        // else if step_a->index > the furthest node in task a that precedes task_b, we cannot make a decision
    #endif

    visited.clear();
    visited.reserve(5);
    bool result = this->visit(step_a,step_b,task_a,task_b,visited);

    #ifdef CACHE
        if(result == true){
            in_cache = step_a->index; // in_cache is a reference, here we update the value in cache
        }
    #endif

    #ifdef DEBUG
        if(visited.size() > 0){
            visittotalsize += visited.size();
            visitcount ++;
            visitmax = visitmax > visited.size() ? visitmax : visited.size();
            visitmin = visitmin < visited.size() ? visitmin : visited.size();
        }
    #endif

    return result;
}


bool DisjointSet::visit(tree_node_cpp* step_a, tree_node_cpp* step_b, unsigned int task_a, unsigned int task_b, robin_hood::unordered_set<int> &visited){
    
#ifdef DFS
    bool b_in_visited = visited.count(task_b);
    if(b_in_visited){
        return false;
    }
    visited.insert(task_b);
#endif


    // this covers ancestor in DPST
    if(precede_dpst(step_a,step_b) == true){
        return true;
    }

    set_info* a_set_info = find_helper(task_a);
    set_info* b_set_info = find_helper(task_b);
    int Sa = a_set_info->set_id;

    //optimization
    if(this->all_tasks[Sa].this_task_state == ACTIVE){
        return true;
    }

#ifndef DFS
    // bfs nt joins
    deque<tree_node_cpp*> steps;
    // steps.push_back(step_b);
    int last_push_task = -1;

    // prepare for lsa
    deque<tree_node_cpp*> all_lsa_query_node;
    robin_hood::unordered_set<int> lsa_added_to_q;

    // 2/22 update
    deque<lsa_info> all_lsa; 

#ifdef FB
    for(auto nt_join = b_set_info->nt->begin(); nt_join != b_set_info->nt->end(); nt_join++){
#else
    for(auto nt_join = b_set_info->nt->rbegin(); nt_join != b_set_info->nt->rend(); nt_join++){
#endif
        int task_id = (*nt_join).task_id;

        tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
        tree_node_cpp* last_step_node = task_node->children_list_tail;

        steps.push_back(last_step_node);
    }

    if(b_set_info->lsa.task_id != -1){
        all_lsa.push_back(b_set_info->lsa);
    }

    visited.insert(task_b);

    while(true){
        while(steps.size() > 0){
            tree_node_cpp* step = steps.front();
            steps.pop_front();
            int step_task = step->corresponding_task_id;

            if(visited.count(step_task)){
                continue;
            }

            if(precede_dpst(step_a,step)){
                return true;
            }

            // loop through its nt joins
            set_info* step_set = find_helper(step_task);

            #ifdef FB
                for(auto nt_join = step_set->nt->begin(); nt_join != step_set->nt->end(); nt_join++){
            #else
                for(auto nt_join = step_set->nt->rbegin(); nt_join != step_set->nt->rend(); nt_join++){
            #endif
                    int task_id = (*nt_join).task_id;

                    tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
                    tree_node_cpp* last_step_node = task_node->children_list_tail;

                    steps.push_back(last_step_node);
                }
                last_push_task = step_task;

            // prepare for lsa
            lsa_info new_lsa = step_set->lsa;
            if(new_lsa.task_id != -1){
                all_lsa.push_back(new_lsa);
            }

            visited.insert(step_task);
        }



        // bfs lsa
        // check nt in lsa and goes up until lsa become null
        // if lsa in visited, do not check its nt, just check the new lsa and add it to the dequeue.
        int last_check_lsa = -1;
        set_info* lsa_set_info;

        while(all_lsa.size() > 0){

            lsa_info the_lsa = all_lsa.front();
            all_lsa.pop_front();

            int lsa_task = the_lsa.task_id;
            lsa_set_info = find_helper(lsa_task);

            if(visited.count(lsa_task)){
                continue;
            }

            // add its lsa to the dequeue
            lsa_info one_lsa = lsa_set_info->lsa;
            if(one_lsa.task_id != -1){
                all_lsa.push_back(one_lsa);
            }

            if(!the_lsa.lsa_nt){
                visited.insert(lsa_task);
                continue;
            }

            #ifdef FB
                for(auto nt_join = lsa_set_info->nt->begin(); nt_join != lsa_set_info->nt->end(); nt_join++){
            #else
                for(auto nt_join = the_lsa.lsa_nt->rbegin(); nt_join != the_lsa.lsa_nt->rend(); nt_join++){
            #endif
                    int task_id = (*nt_join).task_id;

                    if(visited.count(task_id)){
                        continue;
                    }

                    set_info* nt_set_info = find_helper(task_id);

                    tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
                    tree_node_cpp* last_step_node = task_node->children_list_tail;

                    if(precede_dpst(step_a,last_step_node)){
                        return true;
                    }

                    // insert nt's nt's to steps...
                    for(auto nt_nt_join = nt_set_info->nt->begin(); nt_nt_join != nt_set_info->nt->end(); nt_nt_join++){
                        int task_id_2 = (*nt_nt_join).task_id;

                        tree_node_cpp* task_node_2 = (tree_node_cpp*) this->all_tasks[task_id_2].node_in_dpst;
                        tree_node_cpp* last_step_node_2 = task_node_2->children_list_tail;

                        steps.push_back(last_step_node_2);
                    }

                    // insert nt's lsa to all_lsa
                    lsa_info nt_lsa = nt_set_info->lsa;
                    if(nt_lsa.task_id != -1){
                        all_lsa.push_back(nt_lsa);
                    }

                    visited.insert(task_id);
                    
                }

            visited.insert(lsa_task);
        }

        if(steps.size() == 0){
            return false;
        }
    } // end while(true)

#else
    // DFS
    // nt
#ifdef FB
    for(auto nt_join = b_set_info->nt->begin(); nt_join != b_set_info->nt->end(); nt_join++){
#else
    for(auto nt_join = b_set_info->nt->rbegin(); nt_join != b_set_info->nt->rend(); ++nt_join){
#endif
        int task_id = (*nt_join).task_id;
        tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
        tree_node_cpp* last_step_node = task_node->children_list_tail;

        if(visit(step_a, last_step_node, task_a, task_id, visited)){
            return true;
        }
    }


    //lsa
    lsa_info one_lsa = b_set_info->lsa;
    
    while (one_lsa.task_id != -1)
    {
        
        set_info* lsa_set_info = find_helper(one_lsa.task_id);
    #ifdef FB
        for(auto lsa_nt = lsa_set_info->nt->begin(); lsa_nt != lsa_set_info->nt->end(); ++lsa_nt){
    #else
        for(auto lsa_nt = lsa_set_info->nt->rbegin(); lsa_nt != lsa_set_info->nt->rend(); ++lsa_nt){
    #endif
        // for(auto lsa_nt = lsa_set_info->nt->begin(); lsa_nt != lsa_set_info->nt->end(); lsa_nt++){
            tree_node_cpp* step_before_this_nt = (*lsa_nt).last_node_before_this_nt;

            // assert(lsa_deepest_reachable_node != NULL);
            // if(precede_dpst(step_before_this_nt,lsa_deepest_reachable_node)){
            // if(step_before_this_nt->is_parent_nth_child <= lsa_deepest_reachable_node->is_parent_nth_child){
                int task_id = (*lsa_nt).task_id;
                tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
                tree_node_cpp* last_step_node = task_node->children_list_tail;
                // assert(last_step_node->this_node_type == STEP);

                if(visit(step_a, last_step_node, task_a, task_id, visited)){
                    return true;
                }
            // }

        }
        one_lsa = lsa_set_info->lsa;
    }
    
    return false;
#endif
}

int DisjointSet::get_cache_size(){
    printf("cache hit %d, cache miss %d, hit rate = %f \n", cachehit, cachemiss, (double) cachehit / (double) totalprecede);
    printf("same step count: %d \n", samestepcount);
    if(visitcount > 0){
        printf("min visited size: %d, max visited size: %d , average nt visit size %f \n", visitmin, visitmax, (double) visittotalsize / (double) visitcount);
    }
    return this->cache.size();
}

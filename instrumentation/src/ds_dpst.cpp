#include "ds_dpst.h"

int DisjointSet::get_tree_join_count(){
    return this->tree_join_count;
}

hclib_finish::hclib_finish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address){
    this->finish_id = finish_id;
    this->belong_to_task_id = belong_to_task_id;
    this->node_in_dpst = node_in_dpst;
    this->finish_address = finish_address;
}

void DisjointSet::add_task_to_finish(int finish_id, int task_id){
    this->all_finishes.at(finish_id)->task_in_this_finish.push_back(task_id);
}

void DisjointSet::addFinish(int finish_id, hclib_finish *finish){
    this->all_finishes.insert(pair<int, hclib_finish*>(finish_id, finish));
    // this->all_finishes[finish_id] = finish;
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
    this->all_tasks.insert(pair<int, hclib_task>(task_id, task));

    if(task_id == 0){
        return;
    }

    set_info* task_set_info = find_helper(task_id);
    set_info* parent_set_info = find_helper(task.parent_id);
    
    if(parent_set_info->nt->size() > 0){
        assert(last_node_reachable_in_parent != NULL);
        lsa_info new_lsa = {
            .task_id = task.parent_id,
            .last_node_reachable_in_lsa = last_node_reachable_in_parent
        };
        task_set_info->lsa = new_lsa;
    }
    else{
        task_set_info->lsa = parent_set_info->lsa;
    }
};

hclib_task DisjointSet::get_task_info(int task_id){
    return this->all_tasks[task_id];
}

void DisjointSet::update_task_dpst_node(int task_id, void *new_node){
    this->all_tasks.at(task_id).node_in_dpst = new_node;
}


DisjointSet::DisjointSet(){
    this->all_finishes.reserve(1000);
    this->cache.reserve(20000);
    this->all_tasks.reserve(5000);
    this->parent_aka_setnowin.reserve(5000);

}

void DisjointSet::addSet(int task_index){
    lsa_info null_lsa = {
        .task_id = -1,
        .last_node_reachable_in_lsa = NULL
    };

    vector<nt_info> *nontreejoins = new vector<nt_info>();

    set_info* new_set = new set_info(task_index,0,null_lsa,nontreejoins);
    this->parent_aka_setnowin.insert(pair<int,set_info*>(task_index,new_set));
    // this->parent_aka_setnowin[task_index] = new_set;
}

set_info* DisjointSet::find_helper(int k){
    assert(k != -1);
    set_info* current_set_info = parent_aka_setnowin.at(k);
    if (current_set_info->set_id != k)
    {
        *current_set_info = *find_helper(current_set_info->set_id);
    }
    return current_set_info;
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
    this->tree_join_count++;
    
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

    // union nt
    set<nt_info> unique_nt;
    vector<nt_info>* a_nt = a_set_info->nt;
    vector<nt_info>* b_nt = b_set_info->nt;

    for(auto i=a_nt->begin(); i != a_nt->end(); ++i){
        unique_nt.insert(*i);
    }

    for(auto i = b_nt->begin(); i != b_nt->end(); ++i){
        unique_nt.insert(*i);
    }

    std::vector<nt_info>().swap(*a_nt);
    std::vector<nt_info>().swap(*b_nt);
    std::vector<nt_info> *new_nt = new std::vector<nt_info>(unique_nt.begin(), unique_nt.end());
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
    nt_info new_nt = {
        .task_id = nt_task_id,
        .last_node_before_this_nt = last_node_before_nt
    };
    task_set->nt->push_back(new_nt);
    if(this->all_tasks[nt_task_id].this_task_state != JOINED){
        this->all_tasks[nt_task_id].this_task_state = JOINED;
    }
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
    for (std::pair<int, hclib_task> element: this->all_tasks) {
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
    for (std::pair<int, set_info*> element: parent_aka_setnowin) {
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

    for (std::pair<int, set_info*> element: parent_aka_setnowin) {
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

    for (std::pair<int, set_info*> element: parent_aka_setnowin) {
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

int bool_count = 0;
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

bool DisjointSet::precede(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b){
    // printf("precede \n");
    if(step_a->index == step_b->index){
        return true;
    }

    cache_key key(task_a,task_b);
    bool in_cache = cache.find(key) != cache.end();
    if(in_cache){
        if(step_a->index <= cache.at(key)->index){
            return true;
        }
    }

    unordered_set<int> visited;
    bool result = this->visit(step_a,step_b,task_a,task_b,visited);

    if(result == true){
        if(in_cache){
            cache.at(key) = step_a;
        }
        else{
            cache.insert(std::pair<cache_key,tree_node_cpp*>(key,step_a));
        }
    }
    return result;
}

bool DisjointSet::visit(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b, unordered_set<int> visited){
    cache_key key(task_a,task_b);
    bool in_cache = cache.find(key) != cache.end();
    if(in_cache){
        if(step_a->index <= cache.at(key)->index){
            return true;
        }
    }
    
    bool b_in_visited = visited.find(task_b) != visited.end();
    if(b_in_visited){
        return false;
    }
    visited.insert(task_b);


    // this covers ancestor in DPST
    if(precede_dpst(step_a,step_b) == true){
        // if(in_cache){
        //     cache.at(key) = step_a;
        // }
        // else{
        //     cache.insert(std::pair<cache_key,tree_node_cpp*>(key,step_a));
        // }
        return true;
    }

    set_info* a_set_info = find_helper(task_a);
    set_info* b_set_info = find_helper(task_b);
    int Sa = a_set_info->set_id;
    int Sb = b_set_info->set_id;

    //optimization
    if(this->all_tasks[Sa].this_task_state == ACTIVE){
        // if(in_cache){
        //     cache.at(key) = step_a;
        // }
        // else{
        //     cache.insert(std::pair<cache_key,tree_node_cpp*>(key,step_a));
        // }
        return true;
    }

    // nt joins
    for(auto nt_join = b_set_info->nt->begin(); nt_join != b_set_info->nt->end(); nt_join++){
        int task_id = (*nt_join).task_id;
        tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
        tree_node_cpp* last_step_node = task_node->children_list_tail;
        assert(last_step_node->this_node_type == STEP);

        if(visit(step_a, last_step_node, task_a, task_id, visited)){
            return true;
        }
    }

    // lsa
    lsa_info one_lsa = b_set_info->lsa;
    
    while (one_lsa.task_id != -1)
    {
        tree_node_cpp* lsa_deepest_reachable_node = one_lsa.last_node_reachable_in_lsa;
        assert(lsa_deepest_reachable_node != NULL);
        
        set_info* lsa_set_info = find_helper(one_lsa.task_id);
        for(auto lsa_nt = lsa_set_info->nt->begin(); lsa_nt != lsa_set_info->nt->end(); lsa_nt++){

            tree_node_cpp* step_before_this_nt = (*lsa_nt).last_node_before_this_nt;

            assert(lsa_deepest_reachable_node != NULL);
            if(precede_dpst(step_before_this_nt,lsa_deepest_reachable_node)){
                int task_id = (*lsa_nt).task_id;
                tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id].node_in_dpst;
                tree_node_cpp* last_step_node = task_node->children_list_tail;
                assert(last_step_node->this_node_type == STEP);

                if(visit(step_a, last_step_node, task_a, task_id, visited)){
                    return true;
                }
            }

        }
        one_lsa = lsa_set_info->lsa;
    }
    
    return false;
}

int DisjointSet::get_cache_size(){
    return this->cache.size();
}

bool DisjointSet::easy_precede(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b){
    if(step_a->index == step_b->index){
        return true;
    }

    cache_key key(task_a,task_b);
    bool in_cache = cache.find(key) != cache.end();
    if(in_cache){
        if(this->precede_dpst(step_a,cache.at(key))){
            return true;
        }
    }

    bool result = false;
    if(precede_dpst(step_a,step_b) == true){
        result = true;
    }

    set_info* a_set_info = find_helper(task_a);
    int Sa = a_set_info->set_id;
    if(this->all_tasks[Sa].this_task_state == ACTIVE){
        result = true;
    }

    if(result && in_cache){
        cache.at(key) = step_a;
    }
    else if(result){
        cache.insert(std::pair<cache_key,tree_node_cpp*>(key,step_a));
    }

    return result;
}
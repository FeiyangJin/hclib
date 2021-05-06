#include "ds_dpst.h"

hclib_finish::hclib_finish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address){
    this->finish_id = finish_id;
    this->belong_to_task_id = belong_to_task_id;
    this->node_in_dpst = node_in_dpst;
    this->finish_address = finish_address;
}

void DisjointSet::add_task_to_finish(int finish_id, int task_id){
    this->all_finishes[finish_id]->task_in_this_finish.push_back(task_id);
}

void DisjointSet::addFinish(int finish_id, hclib_finish *finish){
    this->all_finishes[finish_id] = finish;
}

void DisjointSet::end_finish_merge(int finish_id, tree_node_cpp* query_node){
    hclib_finish *finish = this->all_finishes[finish_id];
    int finish_owner_task = finish->belong_to_task_id;
    for(vector<int>::iterator i = finish->task_in_this_finish.begin(); i != finish->task_in_this_finish.end(); i++){
        int sub_task_id = *i;
        this->update_task_state(sub_task_id,JOINED);
        this->mergeBtoA(finish_owner_task, sub_task_id, query_node);
    }
}

hclib_task::hclib_task(int task_id, int parent_id, void *node_in_dpst, void *task_address, task_state state){
    this->task_id = task_id;
    this->parent_id = parent_id;
    this->node_in_dpst = node_in_dpst;
    this->task_address = task_address;
    this->this_task_state = state;
}

/**
 * @brief  Add a struct hclib_task to map and update lsa 
 * @note   
 * @param  task_id: task_id
 * @param  *task: the struct hclib_task
 * @retval None
 */
void DisjointSet::addTask(int task_id, hclib_task *task, tree_node_cpp *last_node_reachable_in_parent){
    this->all_tasks[task_id] = task;

    if(task_id == 0){
        lsa_info null_lsa = {
            .task_id = -1,
            .last_node_reachable_in_lsa = NULL
        };
        this->lsa[0] = null_lsa;
        return;
    }

    int task_set = Find(task_id);
    int parent_set = Find(task->parent_id);
    
    if(this->ntcounts(parent_set) > 0){
        assert(last_node_reachable_in_parent != NULL);
        lsa_info new_lsa = {
            .task_id = task->parent_id,
            .last_node_reachable_in_lsa = last_node_reachable_in_parent
        };
        this->setlsa(task_set, new_lsa);
    }
    else{
        this->setlsa(task_set, this->lsa[parent_set]);
    }
};

hclib_task* DisjointSet::get_task_info(int task_id){
    return this->all_tasks[task_id];
}

void DisjointSet::update_task_dpst_node(int task_id, void *new_node){
    this->all_tasks[task_id]->node_in_dpst = new_node;
}


DisjointSet::DisjointSet(){

}

// void DisjointSet::Union(int a, int b){
//     int Sa = Find(a);
//     int Sb = Find(b);

//     if (Sa == Sb) {
//         return;
//     }

//     if(rank[Sa] > rank[Sb]){
//         parent_aka_setnowin[Sb] = Sa;
//     }
//     else if(rank[Sa] < rank[Sb]){
//         parent_aka_setnowin[Sa] = Sb;
//     }
//     else{
//         parent_aka_setnowin[Sb] = Sa;
//         rank[Sa] ++;
//     }
    
// }

void DisjointSet::addSet(int set_index){
    this->parent_aka_setnowin[set_index] = set_index;
    this->rank[set_index] = 0;

    vector<nt_info> *nontreejoins = new vector<nt_info>();
    this->nt[set_index] = *nontreejoins;

    lsa_info null_lsa = {
        .task_id = -1,
        .last_node_reachable_in_lsa = NULL
    };
    this->lsa[set_index] = null_lsa;
}

int DisjointSet::Find(int k){
    assert(k != -1);
    if (parent_aka_setnowin[k] != k)
    {
        parent_aka_setnowin[k] = Find(parent_aka_setnowin[k]);
    }
    return parent_aka_setnowin[k];
}

/**
 * @brief  task a called get(b), merge the two disjoint sets
 * @note   the new set could be a_set or b_set, depends on rank
 * @param  a: task a task_id
 * @param  b: task b task_id
 * @retval None
 */
void DisjointSet::mergeBtoA(int a, int b, tree_node_cpp* query_node){
    int a_set = Find(a);
    int b_set = Find(b);

    assert(a_set >= 0);
    assert(b_set >= 0);

    if (a_set == b_set) {
        return;
    }

    // union nt
    set<nt_info> unique_nt;
    vector<nt_info> a_nt = nt.at(a_set);
    vector<nt_info> b_nt = nt.at(b_set);

    for(auto i=a_nt.begin(); i != a_nt.end(); ++i){
        unique_nt.insert(*i);
    }

    for(auto i = b_nt.begin(); i != b_nt.end(); ++i){
        unique_nt.insert(*i);
    }

    std::vector<nt_info>().swap(nt.at(a_set));
    std::vector<nt_info>().swap(nt.at(b_set));
    std::vector<nt_info> new_nt(unique_nt.begin(), unique_nt.end());
    // set new lsa
    lsa_info new_lsa = this->lsa[a_set];

    // now just join B to A
    // int new_set = a_set;
    // this->parent_aka_setnowin[b_set] = a_set;
    // union two sets
    int new_set = -1;
    if(rank[a_set] > rank[b_set]){
        parent_aka_setnowin[b_set] = a_set;
        new_set = a_set;
    }
    else if(rank[a_set] < rank[b_set]){
        parent_aka_setnowin[a_set] = b_set;
        new_set = b_set;
    }
    else{
        parent_aka_setnowin[b_set] = a_set;
        rank[a_set] ++;
        new_set = a_set;
    }
 
    this->nt[new_set] = new_nt;
    this->lsa[new_set] = new_lsa;
}

void DisjointSet::addnt(int task, int nt_task_id, tree_node_cpp* last_node_before_nt){
    int task_set = Find(task);
    nt_info new_nt = {
        .task_id = nt_task_id,
        .last_node_before_this_nt = last_node_before_nt
    };
    nt[task_set].push_back(new_nt);
    // if(this->all_tasks[nt_task_id]->this_task_state != JOINED){
    //     this->all_tasks[nt_task_id]->this_task_state = JOINED;
    // }
}

int DisjointSet::ntcounts(int task_id){
    return nt[Find(task_id)].size();
}

int DisjointSet::ntcounts_task(int task_id){
    return this->nt[task_id].size();
}

int DisjointSet::getlsa(int task_id){
    return this->lsa[Find(task_id)].task_id;
}

int DisjointSet::getlsa_task(int task_id){
    return this->lsa[task_id].task_id;
}

void DisjointSet::setlsa(int task_id, lsa_info new_lsa){
    this->lsa[Find(task_id)] = new_lsa;
}

string state_string[4] = {"Active", "Blocked", "Finished_not_Joined", "Joined"};

void DisjointSet::print_all_tasks(){
    for (std::pair<int, hclib_task*> element: this->all_tasks) {
        hclib_task *task = element.second;
        int task_id = element.first;
        printf("task %d, parent is %d, has %d nt joins, lsa is %d, now in set: %d, task state is: ", 
            task_id, task->parent_id, this->nt[task_id].size(), this->lsa[task_id].task_id, Find(task_id));

        int state = static_cast<int>(this->all_tasks[task_id]->this_task_state);
        std::cout << state_string[state];

        if(this->lsa[task_id].task_id != -1){
            assert(this->lsa[task_id].last_node_reachable_in_lsa != NULL);
            printf("    lsa last reachable node index is %d \n",this->lsa[task_id].last_node_reachable_in_lsa->index);
        }
        else{
            printf("\n");
        }
    }
}

void DisjointSet::print_nt(int set_id){
    vector<nt_info> ntjoins = this->nt[set_id];
    printf("set %d has non-tree joins: \n",set_id);
    for(auto join = ntjoins.begin(); join != ntjoins.end(); join++){
        printf("task %d, DPST index of last node before this join %d \n",(*join).task_id,(*join).last_node_before_this_nt->index);
    }
}

void DisjointSet::printds(){
    for (std::pair<int, int> element: parent_aka_setnowin) {
        printf("%d is in set: %d \n", element.first, Find(element.first));
    };

    for (std::pair<int, std::vector<nt_info>> element: nt) {
        int task_id = element.first;
        printf("%d has nt joins: ", task_id);

        for(auto item = element.second.begin(); item != element.second.end(); ++item){
            printf("%d ",(*item).task_id);
        }
        printf("\n");
    };

    for(std::pair<int,lsa_info> element: lsa){
        printf("task %d has lsa %d \n",element.first,element.second.task_id);
    }

}

void DisjointSet::printdsbyset(){
    unordered_map<int, vector<int>> all_sets;

    for (std::pair<int, int> element: parent_aka_setnowin) {
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

        printf("\n    nt_joins: ");
        for(auto nt_join = nt[the_set.first].begin(); nt_join != nt[the_set.first].end(); nt_join++){
            printf("%d ",(*nt_join).task_id);
        }

        printf("\n      lsa is: %d ", this->lsa[the_set.first].task_id);

        printf("\n\n");
    }
}

void DisjointSet::print_table(){
    unordered_map<int, vector<int>> all_sets;

    for (std::pair<int, int> element: parent_aka_setnowin) {
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

        for(auto nt_join = nt[the_set.first].begin(); nt_join != nt[the_set.first].end(); nt_join++){
            printf("%d,",(*nt_join).task_id);
        }

        printf(" | ");

        printf("%d", this->lsa[the_set.first].task_id);

        printf("\n\n");
    }

}

void DisjointSet::update_task_state(int task_id, task_state new_state){
    this->all_tasks[task_id]->this_task_state = new_state;
}

int DisjointSet::find_task_node_index(int task_id){
    tree_node_cpp* node = (tree_node_cpp*) this->all_tasks[task_id]->node_in_dpst;
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

bool DisjointSet::precede_dpst(tree_node_cpp* node1, tree_node_cpp* node2){
    if(node1->index == node2->index){
        return true;
    }
    else if(node1->parent->index == node2->parent->index){
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
            return false;
        }
        else{
            return true;
        }

    }

    return false;
}

bool DisjointSet::precede(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b){
    //return true;
    //printf("    precede \n");
    set<int> visited;
    return this->visit(step_a,step_b,task_a,task_b,visited);
}

bool DisjointSet::visit(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b, set<int> visited){
    if(task_a == task_b){
        if(step_a->is_parent_nth_child <= step_b->is_parent_nth_child){
            return true;
        }
        return false;
    };

    const bool b_in_visited = visited.find(task_b) != visited.end();
    if(b_in_visited){
        return false;
    }

    visited.insert(task_b);

    int Sa = this->Find(task_a);
    int Sb = this->Find(task_b);

    // optimization 1
    if(this->all_tasks[Sa]->this_task_state == ACTIVE){
        //printf("    return in optimiaztion 1 \n");
        return true;
    }

    // this covers ancestor in DPST
    if(precede_dpst(step_a,step_b) == true){
        //printf("    return in precede_dpst with step_a: %d step_b: %d \n",step_a->index,step_b->index);
        return true;
    }

    // nt joins
    for(auto nt_join = this->nt[Sb].begin(); nt_join != this->nt[Sb].end(); nt_join++){
        //printf("inside nt check \n");
        int task_id = (*nt_join).task_id;
        tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id]->node_in_dpst;
        tree_node_cpp* last_step_node = task_node->children_list_tail;
        assert(last_step_node->this_node_type == STEP);

        if(visit(step_a, last_step_node, task_a, task_id, visited)){
            //printf("    return in nt joins \n");
            return true;
        }
    }

    // lsa
    lsa_info one_lsa = this->lsa[Sb];
    
    while (one_lsa.task_id != -1)
    {
        //printf("inside lsa check \n");
        tree_node_cpp* lsa_deepest_reachable_node = one_lsa.last_node_reachable_in_lsa;
        assert(lsa_deepest_reachable_node != NULL);
        
        for(auto lsa_nt = this->nt[one_lsa.task_id].begin(); lsa_nt != this->nt[one_lsa.task_id].end(); lsa_nt++){

            tree_node_cpp* step_before_this_nt = (*lsa_nt).last_node_before_this_nt;

            assert(lsa_deepest_reachable_node != NULL);
            if(precede_dpst(step_before_this_nt,lsa_deepest_reachable_node)){
            //if(this->precede(step_before_this_nt,lsa_deepest_reachable_node,(*lsa_nt).task_id,one_lsa.task_id)){
                int task_id = (*lsa_nt).task_id;
                tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id]->node_in_dpst;
                tree_node_cpp* last_step_node = task_node->children_list_tail;
                assert(last_step_node->this_node_type == STEP);

                if(visit(step_a, last_step_node, task_a, task_id, visited)){
                    //printf("    return in lsa nt \n");
                    return true;
                }
            }

        }
        one_lsa = this->lsa[Find(one_lsa.task_id)];
    }
    
    return false;
}
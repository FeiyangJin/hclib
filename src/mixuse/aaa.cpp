#include "aaa.h"

hclib_finish::hclib_finish(int finish_id, int belong_to_task_id, void *node_in_dpst, void *finish_address){
    this->finish_id = finish_id;
    this->belong_to_task_id = belong_to_task_id;
    this->node_in_dpst = node_in_dpst;
    this->finish_address = finish_address;
    //this->task_in_this_finish = new vector<int>();
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
    all_tasks[task_id] = task;

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



string state_string[4] = {"Active", "Blocked", "Finished_not_Joined", "Joined"};

void DisjointSet::print_all_tasks(){
    for (std::pair<int, hclib_task*> element: this->all_tasks) {
        hclib_task *task = element.second;
        int task_id = element.first;
        printf("query node %d ", this->find_helper(task_id).query_node_in_current_set == NULL ? -1:this->find_helper(task_id).query_node_in_current_set->index);
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
    set_info new_set = {
        .set_id = set_index,
        .query_node_in_current_set = NULL
    };
    this->parent_aka_setnowin[set_index] = new_set;
    this->rank[set_index] = 0;

    vector<nt_info> nontreejoins;
    this->nt[set_index] = nontreejoins;

    lsa_info null_lsa = {
        .task_id = -1,
        .last_node_reachable_in_lsa = NULL
    };
    this->lsa[set_index] = null_lsa;
}


set_info DisjointSet::find_helper(int k){
    assert(k != -1);
    if (parent_aka_setnowin[k].set_id != k)
    {
        set_info result = find_helper(parent_aka_setnowin[k].set_id);

        // the first returned recursion will go into this if
        // because the immediately returned set is the previous parent
        if(result.query_node_in_current_set == NULL && parent_aka_setnowin[k].query_node_in_current_set != NULL){
            parent_aka_setnowin[k].set_id = result.set_id;
        }
        else{
            parent_aka_setnowin[k] = result;
        }
        
    }

    return parent_aka_setnowin[k];
}

int DisjointSet::Find(int k){
    return find_helper(k).set_id;
}

/**
 * @brief  task a called get(b), merge the two disjoint sets
 * @note   the new set could be a_set or b_set, depends on rank
 * @param  a: task a task_id
 * @param  b: task b task_id
 * @param query_node: the step node in DPST that we should query on
 * @retval None
 */
void DisjointSet::mergeBtoA(int a, int b, tree_node_cpp* query_node){
    assert(query_node->this_node_type == STEP);

    int a_set = Find(a);
    int b_set = Find(b);

    assert(a_set >= 0);
    assert(b_set >= 0);

    if (a_set == b_set) {
        return;
    }

    // union nt
    vector<nt_info> a_nt = nt.at(a_set);
    vector<nt_info> b_nt = nt.at(b_set);
    for(auto i = b_nt.begin(); i != b_nt.end(); ++i){
        a_nt.push_back(*i);
    }

    // set new lsa
    lsa_info new_lsa = this->lsa[a_set];

    // now just join B to A
    int new_set = a_set;
    this->parent_aka_setnowin[b_set].set_id = a_set;
    this->parent_aka_setnowin[b_set].query_node_in_current_set = query_node;
    // union two sets
    // int new_set = -1;
    // if(rank[a_set] > rank[b_set]){
    //     parent_aka_setnowin[b_set].set_id = a_set;
    //     parent_aka_setnowin[b_set].query_node_in_current_set = query_node;
    //     new_set = a_set;
    // }
    // else if(rank[a_set] < rank[b_set]){
    //     parent_aka_setnowin[a_set].set_id = b_set;
    //     parent_aka_setnowin[a_set].query_node_in_current_set = query_node;
    //     new_set = b_set;
    // }
    // else{
    //     parent_aka_setnowin[b_set].set_id = a_set;
    //     parent_aka_setnowin[b_set].query_node_in_current_set = query_node;
    //     rank[a_set] ++;
    //     new_set = a_set;
    // }

    this->nt[new_set] = a_nt;
    this->lsa[new_set] = new_lsa;
}

void DisjointSet::addnt(int task, int nt_task_id, tree_node_cpp* last_node_before_nt){
    int task_set = Find(task);
    nt_info new_nt = {
        .task_id = nt_task_id,
        .last_node_before_this_nt = last_node_before_nt
    };
    nt[task_set].push_back(new_nt);
    if(this->all_tasks[nt_task_id]->this_task_state != JOINED){
        this->all_tasks[nt_task_id]->this_task_state = JOINED;
    }
}

int DisjointSet::ntcounts(int task_id){
    return nt[Find(task_id)].size();
}

int DisjointSet::ntcounts_task(int task_id){
    return this->nt[task_id].size();
}

void DisjointSet::print_nt(int set_id){
    vector<nt_info> ntjoins = this->nt[set_id];
    printf("set %d has non-tree joins: \n",set_id);
    for(auto join = ntjoins.begin(); join != ntjoins.end(); join++){
        printf("task %d, DPST index of last node before this join %d \n",(*join).task_id,(*join).last_node_before_this_nt->index);
    }
}

int DisjointSet::getlsa(int task_id){
    return this->lsa[Find(task_id)].task_id;
}

int DisjointSet::getlsa_task(int task_id){
    return this->lsa[task_id].task_id;
}

void DisjointSet::setlsa(int task_id, lsa_info new_lsa){
    int task_set = Find(task_id);

    this->lsa[task_set] = new_lsa;
}

void DisjointSet::printds(){
    for (std::pair<int, set_info> element: parent_aka_setnowin) {
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

    for (std::pair<int, set_info> element: parent_aka_setnowin) {
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

    for (std::pair<int, set_info> element: parent_aka_setnowin) {
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
    while (node1->depth != node2->depth)
    {
        if (node1->depth > node2->depth)
        {
            node1 = node1->parent;
        }
        else{
            node2 = node2->parent;
        }
    }

    tree_node_cpp* node1_last_node;
    tree_node_cpp* node2_last_node;

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

bool DisjointSet::precede(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b){
    set<int> visited;
    return this->visit(step_a,step_b,task_a,task_b,visited);
}

bool DisjointSet::visit(tree_node_cpp* step_a, tree_node_cpp* step_b, int task_a, int task_b, set<int> visited){
    const bool b_in_visited = visited.find(task_b) != visited.end();
    if(b_in_visited){
        return false;
    }

    visited.insert(task_b);

    set_info Sa = this->find_helper(task_a);
    set_info Sb = this->find_helper(task_b);

    if(Sa.set_id == Sb.set_id){
        return true;
    }

    // this covers ancestor in DPST
    tree_node_cpp* query_node = step_a;
    if(Sa.query_node_in_current_set != NULL){
        query_node = Sa.query_node_in_current_set;
    }

    tree_node_cpp *lca_lc = find_lca_left_child_cpp(query_node,step_b);
    if(lca_lc->this_node_type != FUTURE && lca_lc->this_node_type != ASYNC){
        return true;
    }

    
    // nt joins
    for(auto nt_join = this->nt[Sb.set_id].begin(); nt_join != this->nt[Sb.set_id].end(); nt_join++){
        int task_id = (*nt_join).task_id;
        tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id]->node_in_dpst;
        tree_node_cpp* last_step_node = task_node->children_list_tail;
        assert(last_step_node->this_node_type == STEP);

        if(visit(step_a, last_step_node, task_a, task_id, visited)){
            return true;
        }
    }

    // lsa
    lsa_info one_lsa = this->lsa[Sb.set_id];
    
    while (one_lsa.task_id != -1)
    {
        tree_node_cpp* lsa_deepest_reachable_node = one_lsa.last_node_reachable_in_lsa;
        assert(lsa_deepest_reachable_node != NULL);
        
        for(auto lsa_nt = this->nt[one_lsa.task_id].begin(); lsa_nt != this->nt[one_lsa.task_id].end(); lsa_nt++){

            tree_node_cpp* step_before_this_nt = (*lsa_nt).last_node_before_this_nt;


            assert(lsa_deepest_reachable_node != NULL);
            if(step_before_this_nt->is_parent_nth_child <= lsa_deepest_reachable_node->is_parent_nth_child){
                int task_id = (*lsa_nt).task_id;
                tree_node_cpp* task_node = (tree_node_cpp*) this->all_tasks[task_id]->node_in_dpst;
                tree_node_cpp* last_step_node = task_node->children_list_tail;
                assert(last_step_node->this_node_type == STEP);

                if(visit(step_a, last_step_node, task_a, task_id, visited)){
                    return true;
                }
            }

        }
        one_lsa = this->lsa[Find(one_lsa.task_id)];
    }
    
    return false;
}
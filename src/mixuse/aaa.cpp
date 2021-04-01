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

void DisjointSet::end_finish_merge(int finish_id){
    hclib_finish *finish = this->all_finishes[finish_id];
    int finish_owner_task = finish->belong_to_task_id;
    for(vector<int>::iterator i = finish->task_in_this_finish.begin(); i != finish->task_in_this_finish.end(); i++){
        int sub_task_id = *i;
        this->update_task_state(sub_task_id,JOINED);
        this->mergeBtoA(finish_owner_task, sub_task_id);
    }
}

hclib_task::hclib_task(int task_id, int parent_id, void *node_in_dpst, void *task_address, task_state state){
    this->task_id = task_id;
    this->parent_id = parent_id;
    this->node_in_dpst = node_in_dpst;
    this->task_address = task_address;
    this->this_task_state = state;
}


void DisjointSet::addTask(int task_id, hclib_task *task){
    all_tasks[task_id] = task;
    if(this->ntcounts(task->parent_id) > 0){
        this->setlsa(task_id, task->parent_id);
    }
    else{
        this->setlsa(task_id, -1);
    }
};

hclib_task* DisjointSet::get_task_info(int task_id){
    return this->all_tasks[task_id];
}


void DisjointSet::update_task_parent(int task_id, int new_parent_id){
    this->all_tasks[task_id]->parent_id = new_parent_id;

    // also need to update nt joins and lsa
    if(this->ntcounts(new_parent_id) > 0){
        this->lsa[task_id] = new_parent_id;
    }
    else{
        this->lsa[task_id] = -1;
    }

}

void DisjointSet::break_previous_steps(int task_id, int task_id_for_previous_steps){
    hclib_task *whole_task = this->all_tasks[task_id];
    int previous_parent_id = whole_task->parent_id;
    void *previous_dpst_node = whole_task->node_in_dpst;
    void *previous_task_address = whole_task->task_address;
    task_state new_state = JOINED;

    hclib_task *task_for_previous_steps = new hclib_task(task_id_for_previous_steps,previous_parent_id,previous_dpst_node,previous_task_address,new_state);

    this->all_tasks[task_id_for_previous_steps] = task_for_previous_steps;
    this->lsa[task_id_for_previous_steps] = this->lsa[task_id];
    this->nt[task_id_for_previous_steps] = this->nt[task_id];
    this->parent_aka_setnowin[task_id_for_previous_steps] = task_id_for_previous_steps;

    this->nt[task_id].clear();
    this->nt[task_id].push_back(task_id_for_previous_steps);
}

string state_string[4] = {"Active", "Blocked", "Finished_not_Joined", "Joined"};

void DisjointSet::print_all_tasks(){
    for (std::pair<int, hclib_task*> element: this->all_tasks) {
        hclib_task *task = element.second;
        int task_id = element.first;
        printf("task %d, parent is %d, has %d nt joins, lsa is %d, task state is: ", task_id, task->parent_id, this->ntcounts(task_id), this->lsa[task_id]);
        int state = static_cast<int>(this->all_tasks[task_id]->this_task_state);
        std::cout << state_string[state] << std::endl;
    }
}

void DisjointSet::update_task_dpst_node(int task_id, void *new_node){
    this->all_tasks[task_id]->node_in_dpst = new_node;
}


DisjointSet::DisjointSet(){

}

void DisjointSet::addSet(int set_index){
    this->parent_aka_setnowin[set_index] = set_index;
    vector<int> nontreejoins;
    nt[set_index] = nontreejoins;
    lsa[set_index] = -1;
}

int DisjointSet::Find(int k){
    if (parent_aka_setnowin[k] != k)
    {
        parent_aka_setnowin[k] = Find(parent_aka_setnowin[k]);
    }

    return parent_aka_setnowin[k];
}

void DisjointSet::mergeBtoA(int a, int b){
    int Sa = Find(a);
    int Sb = Find(b);

    if (Sa == Sb) {
        return;
    }

    // union nt
    vector<int> a_nt = nt.at(Sa);
    vector<int> b_nt = nt.at(Sb);
    for(auto i = b_nt.begin(); i != b_nt.end(); ++i){
        a_nt.push_back(*i);
    }
    nt[Sa] = a_nt;

    // union Sb into Sa
    parent_aka_setnowin[Sb] = Sa;
}

void DisjointSet::addnt(int task, int nt_task_id){
    //int Sa = Find(task);
    nt[task].push_back(nt_task_id);
    if(this->all_tasks[nt_task_id]->this_task_state != JOINED){
        this->all_tasks[nt_task_id]->this_task_state = JOINED;
    }
}

int DisjointSet::ntcounts(int task_id){
    //int Sa = Find(task_id);
    return nt[task_id].size();
}

int DisjointSet::getlsa(int task_id){
    return this->lsa[task_id];
}

void DisjointSet::setlsa(int task_id, int lsa){
    //int Sa = Find(task_id);
    this->lsa[task_id] = lsa;
}

void DisjointSet::printds(){
    for (std::pair<int, int> element: parent_aka_setnowin) {
        printf("%d is in set: %d \n", element.first, Find(element.first));
    };

    for (std::pair<int, std::vector<int>> element: nt) {
        printf("%d has %d nt joins \n", element.first, element.second.size());
    };
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
            printf("%d ",*nt_join);
        }

        printf("\n      lsa is: %d ", this->lsa[the_set.first]);

        printf("\n\n");
    }
}

void printSets(vector<int> const &universe, DisjointSet &ds){
    for (int i: universe) {
        printf("element %d is in set %d \n",i,ds.Find(i));
    }
    printf("\n");
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
            printf("%d,",*nt_join);
        }

        printf(" | ");

        printf("%d", this->lsa[the_set.first]);

        printf("\n\n");
    }

}

void DisjointSet::update_task_state(int task_id, task_state new_state){
    this->all_tasks[task_id]->this_task_state = new_state;
}
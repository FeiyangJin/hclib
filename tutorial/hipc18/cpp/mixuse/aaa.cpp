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
};

hclib_task* DisjointSet::get_task_info(int task_id){
    return this->all_tasks[task_id];
}

DisjointSet::DisjointSet(){

}

void DisjointSet::addSet(int set_index){
    parent[set_index] = set_index;
    rank[set_index] = 0;
    vector<int> nontreejoins;
    nt[set_index] = nontreejoins;
    lsa[set_index] = -1;
}

int DisjointSet::Find(int k){
    if (parent[k] != k)
    {
        parent[k] = Find(parent[k]);
    }

    return parent[k];
}

void DisjointSet::Union(int a, int b){
    int x = Find(a);
    int y = Find(b);

    if (x == y) {
        return;
    }

    if (rank[x] > rank[y]) {
        parent[y] = x;
    }
    else if (rank[x] < rank[y]) {
        parent[x] = y;
    }
    else {
        parent[x] = y;
        rank[y]++;
    }
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
    
    // Sa.lsa = Sa.lsa, do nothing 

    // union Sb into Sa
    parent[Sb] = Sa;
}

void DisjointSet::addnt(int task, int nt_task_id){
    int Sa = Find(task);
    nt[Sa].push_back(nt_task_id);
}

int DisjointSet::ntcounts(int task_id){
    int Sa = Find(task_id);
    return nt[Sa].size();
}

int DisjointSet::getlsa(int task_id){
    return this->lsa[Find(task_id)];
}

void DisjointSet::setlsa(int task_id, int lsa){
    int Sa = Find(task_id);
    this->lsa[Sa] = lsa;
}

void DisjointSet::printds(){
    for (std::pair<int, int> element: parent) {
        printf("%d is in set: %d \n", element.first, Find(element.first));
    };

    for (std::pair<int, std::vector<int>> element: nt) {
        printf("%d has %d nt joins \n", element.first, element.second.size());
    };
}

void DisjointSet::printdsbyset(){
    unordered_map<int, vector<int>> all_sets;

    for (std::pair<int, int> element: parent) {
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
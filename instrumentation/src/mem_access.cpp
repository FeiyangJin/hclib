#include <cassert>

#include "mem_access.h"

MemAccess_t::MemAccess_t(access_info t_a_n, addr_t r, bool is_promise){
  this->task_and_node.node_in_dpst = t_a_n.node_in_dpst;
  this->task_and_node.task_id = t_a_n.task_id;
  this->rip = r;
  this->promise_task = is_promise;
#ifdef LINK_READER
  this->next = nullptr;
  this->prev = nullptr;
#endif
}

MemAccess_t::~MemAccess_t(){
  this->next = nullptr;
  this->prev = nullptr;
}

MemAccessList_t::MemAccessList_t(addr_t addr, bool is_read, 
                                 access_info task_and_node,
                                 addr_t rip, std::size_t mem_size,
                                 int first_finish_id, bool is_promise) 
  : start_addr( ALIGN_BY_PREV_MAX_GRAIN_SIZE(addr) ) {

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  if (is_read){
    for (int i=start; i < (start + grains); ++i){
      #ifdef LINK_READER
        MemAccess_t* first_reader = new MemAccess_t(task_and_node, rip, is_promise);
        this->readers[i] = first_reader;
        this->readers_tail[i] = first_reader;
      #else
        this->readers[i] = new std::vector<MemAccess_t>();
        this->readers[i]->push_back(MemAccess_t(task_and_node,rip,is_promise));
      #endif

      #ifndef LOOP_READERS
        this->readers_finish_id[i] = new std::unordered_set<int>();
        this->readers_finish_id[i]->insert(first_finish_id);
      #endif

    }
  }
  else{
    for (int i=start; i < (start + grains); ++i){
      this->writers[i] = new MemAccess_t(task_and_node, rip, is_promise);
    }
  }

}

MemAccessList_t::~MemAccessList_t() {
  for(int i=0; i < NUM_SLOTS; i++) {
    if(readers[i]){
      #ifdef LINK_READER
        delete readers[i];
        readers[i] = nullptr;
        if(readers_tail[i]){
          delete readers_tail[i];
          readers_tail[i] = nullptr;
        }
      #else
        readers[i]->clear();
        readers[i] = nullptr;
      #endif

    }
  }
  
  for(int i=0; i < NUM_SLOTS; i++) {
    if(writers[i]) {
      delete writers[i];
      writers[i] = nullptr;
    }
  }

}

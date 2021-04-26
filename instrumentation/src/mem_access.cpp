#include <cassert>

#include "mem_access.h"

MemAccess_t::MemAccess_t(access_info t_a_n, addr_t r){
  this->task_and_node.node_in_dpst = t_a_n.node_in_dpst;
  this->task_and_node.task_id = t_a_n.task_id;
  this->rip = r;
}

MemAccessList_t::MemAccessList_t(addr_t addr, bool is_read, 
                                 access_info task_and_node,
                                 addr_t rip, std::size_t mem_size) 
  : start_addr( ALIGN_BY_PREV_MAX_GRAIN_SIZE(addr) ) {

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  if (is_read){
    for (int i=start; i < (start + grains); ++i){
      std::vector<MemAccess_t*>* all_readers = new std::vector<MemAccess_t*>();
      all_readers->push_back(new MemAccess_t(task_and_node, rip));
      readers[i] = all_readers;
    }
  }
  else{
    for (int i=start; i < (start + grains); ++i){
      writers[i] = new MemAccess_t(task_and_node, rip);
    }
  }

}

MemAccessList_t::~MemAccessList_t() {
  for(int i=0; i < NUM_SLOTS; i++) {
    if(readers[i]) {
      delete readers[i];
      readers[i] = nullptr;
    }
  }
  
  for(int i=0; i < NUM_SLOTS; i++) {
    if(writers[i]) {
      delete writers[i];
      writers[i] = nullptr;
    }
  }

}

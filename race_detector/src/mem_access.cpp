#include <cassert>

#include "mem_access.h"

MemAccess_t::MemAccess_t(tree_node_cpp* step_node){
  this->step_node = step_node;

  #ifdef LINK_READER
    this->next = nullptr;
  #endif
}

MemAccess_t::MemAccess_t(tree_node_cpp* step_node, addr_t rip){
  this->step_node = step_node;
  this->rip = rip;

  #ifdef LINK_READER
    this->next = nullptr;
  #endif
}


MemAccessList_t::MemAccessList_t(addr_t addr, bool is_read, tree_node_cpp* step_node, addr_t rip, std::size_t mem_size){
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  if (is_read){
    for (int i=start; i < (start + grains); ++i){
        MemAccess_t* first_reader = new MemAccess_t(step_node, rip);
        this->readers[i] = first_reader;
    }
  }
  else{
    for (int i=start; i < (start + grains); ++i){
      this->writers[i] = new MemAccess_t(step_node, rip);
    }
  }
}


MemAccessList_t::MemAccessList_t(addr_t addr, bool is_read, tree_node_cpp* step_node, std::size_t mem_size){
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  if (is_read){
    for (int i=start; i < (start + grains); ++i){
        MemAccess_t* first_reader = new MemAccess_t(step_node);
        this->readers[i] = first_reader;
    }
  }
  else{
    for (int i=start; i < (start + grains); ++i){
      this->writers[i] = new MemAccess_t(step_node);
    }
  }
}

MemAccessList_t::~MemAccessList_t() {
  for(int i=0; i < NUM_SLOTS; i++) {
    if(readers[i]){
      #ifdef LINK_READER
        delete readers[i];
        readers[i] = nullptr;
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

void MemAccessList_t::clear() {
  for (int i = 0; i < NUM_SLOTS && readers[i]; i++) {
#ifdef LINK_READER
    MemAccess_t *m = readers[i];
    do {
      MemAccess_t *next = m->next;
      delete m;
      m = next;
    } while (m);
#endif
    readers[i] = nullptr;
  }

  for (int i = 0; i < NUM_SLOTS && writers[i]; i++) {
    delete writers[i];
    writers[i] = nullptr;
  }
}

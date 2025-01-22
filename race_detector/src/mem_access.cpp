#include <cassert>

#include "mem_access.h"

MemAccess_t::MemAccess_t(tree_node_cpp* step_node)
  : step_node(step_node)
#ifdef LINK_READER
  , next(nullptr)
#endif
{}

MemAccess_t::MemAccess_t(tree_node_cpp* step_node, addr_t rip)
  : step_node(step_node), rip(rip)
#ifdef LINK_READER
  , next(nullptr)
#endif
{}


MemAccessList_t::MemAccessList_t(addr_t addr, bool is_read, tree_node_cpp* step_node, addr_t rip, std::size_t mem_size)
{
  initialize(addr, is_read, step_node, rip, mem_size);
}

MemAccessList_t::MemAccessList_t(addr_t addr, bool is_read, tree_node_cpp* step_node, std::size_t mem_size)
{
  initialize(addr, is_read, step_node, 0, mem_size);
}

void MemAccessList_t::initialize(addr_t addr, bool is_read, tree_node_cpp* step_node, addr_t rip, std::size_t mem_size) {
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  if (is_read) {
    for (int i = start; i < (start + grains); ++i) {
      MemAccess_t* first_reader = new MemAccess_t(step_node, rip);
      #ifdef LINK_READER
        this->readers[i] = first_reader;
      #else
        this->readers[i][0] = first_reader;
        this->reader_index[i] = 1;
      #endif
    }
  } else {
    for (int i = start; i < (start + grains); ++i) {
      this->writers[i] = new MemAccess_t(step_node, rip);
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
        for (int j = 0; j < MEM_ACCESS_SIZE; j++) {
          if (readers[i][j]) {
            delete readers[i][j];
            readers[i][j] = nullptr;
          }
        }
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
    readers[i] = nullptr;
#else
    for (int j = 0; j < MEM_ACCESS_SIZE; j++) {
      if (readers[i][j]) {
        delete readers[i][j];
        readers[i][j] = nullptr;
      }
    }
#endif
  }

  for (int i = 0; i < NUM_SLOTS && writers[i]; i++) {
    delete writers[i];
    writers[i] = nullptr;
  }
}

/*
Author: Feiyang Jin
Email: feiyanglovesu@gmail.com
mem_access.h contains the definition of MemAccessList_t, which is what we will
save inside shadow_entries (defined in shadow_memory.h). 
EX.
static ShadowMem<MemAccessList_t> *shadow_mem = new ShadowMem<MemAccessList_t>();

As each MemAccessList_t records access to a range of 16 bytes memory address,
inside MemAccessList_t, we will have 4 slots. Each slot represent 4 bytes. 
A memory access may touch one or multiple slot.
*/


#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>
#include "struct_def.h"

// #define LINK_READER

using addr_t = uint64_t;

#define READER_SIZE 4

#ifndef LOG_KEY_SIZE
#define LOG_KEY_SIZE  4
#endif

#ifndef LOG_TBL_SIZE
#define LOG_TBL_SIZE 20
#endif

#ifndef ADDR_TO_KEY
#define ADDR_TO_KEY(addr) ((addr_t) ((addr_t)addr >> LOG_KEY_SIZE))
#endif

#define GRAIN_SIZE 4
#define LOG_GRAIN_SIZE 2

#define MAX_GRAIN_SIZE (1 << LOG_KEY_SIZE) // 1 << 4 = 2^4 = 16
#define NUM_SLOTS (MAX_GRAIN_SIZE / GRAIN_SIZE) // 16 / 4 = 4
 
// extract the lower 4 bits of addr and right shift by 2 bits
// the result index is a 2 bits number in the range [0,3] 
#define ADDR_TO_MEM_INDEX(addr)                                       \
  (((addr_t)addr & (addr_t)(MAX_GRAIN_SIZE-1)) >> LOG_GRAIN_SIZE)

// number of grains = size >> 2
#define SIZE_TO_NUM_GRAINS(size) (size >> LOG_GRAIN_SIZE)

// // a mask that keeps all the bits set except for the least significant bits
// // that represent the max grain size
// #define MAX_GRAIN_MASK (~(addr_t)(MAX_GRAIN_SIZE-1))

// // If the value is already divisible by MAX_GRAIN_SIZE, return the value; 
// // otherwise return the previous / next value divisible by MAX_GRAIN_SIZE.
// #define ALIGN_BY_PREV_MAX_GRAIN_SIZE(addr) ((addr_t) (addr & MAX_GRAIN_MASK))
// #define ALIGN_BY_NEXT_MAX_GRAIN_SIZE(addr)                  \
//   ((addr_t) ((addr+(MAX_GRAIN_SIZE-1)) & MAX_GRAIN_MASK))


class MemAccess_t {
public:
  tree_node_cpp* step_node;
  addr_t rip;
#ifdef LINK_READER
  MemAccess_t* next;
#endif
  MemAccess_t(tree_node_cpp* step_node, addr_t rip);

  // ~MemAccess_t();
};

/*
  Because each MemAccessList_t is put inside a shadow_entry, it will handle
  a range of 16 bytes memory.
  We divide the 16 bytes into 4 slots so that we can distinguish access to the 
  same shadow_entry but has different access size. 
*/
class MemAccessList_t {
public:
  addr_t start_addr;

  #ifdef LINK_READER
    MemAccess_t* readers[NUM_SLOTS] = {nullptr, nullptr, nullptr, nullptr};
  #else
    MemAccess_t* readers[NUM_SLOTS][READER_SIZE] = {{nullptr, nullptr, nullptr, nullptr},
                                                  {nullptr, nullptr, nullptr, nullptr},
                                                  {nullptr, nullptr, nullptr, nullptr},
                                                  {nullptr, nullptr, nullptr, nullptr}};
    unsigned int reader_index[NUM_SLOTS] = {0, 0, 0, 0};
  #endif

  MemAccess_t* writers[NUM_SLOTS] = {nullptr, nullptr, nullptr, nullptr};

  MemAccessList_t(addr_t addr, bool is_read, tree_node_cpp* step_node, addr_t rip, std::size_t mem_size);
  ~MemAccessList_t();

  void initialize(addr_t addr, bool is_read, tree_node_cpp* step_node, addr_t rip, std::size_t mem_size);
  void clear();
  
}; // end class MemAccessList_t

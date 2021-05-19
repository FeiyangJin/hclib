#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "struct_def.h"

using addr_t = uint64_t;

#define LOG_KEY_SIZE  4
#define LOG_TBL_SIZE 20

#define LOOP_READERS
#define LINK_READER

#ifndef ADDR_TO_KEY
#define ADDR_TO_KEY(addr) ((uint64_t) (uint64_t)addr)
#endif

#define GRAIN_SIZE 4
#define LOG_GRAIN_SIZE 2

#define MAX_GRAIN_SIZE (1 << LOG_KEY_SIZE) // 1 << 4 = 2^4 = 16
#define NUM_SLOTS (MAX_GRAIN_SIZE / GRAIN_SIZE) // 16 / 4 = 4
 
// a mask that keeps all the bits set except for the least significant bits
// that represent the max grain size
#define MAX_GRAIN_MASK (~(addr_t)(MAX_GRAIN_SIZE-1))

// If the value is already divisible by MAX_GRAIN_SIZE, return the value; 
// otherwise return the previous / next value divisible by MAX_GRAIN_SIZE.
#define ALIGN_BY_PREV_MAX_GRAIN_SIZE(addr) ((addr_t) (addr & MAX_GRAIN_MASK))
#define ALIGN_BY_NEXT_MAX_GRAIN_SIZE(addr)                  \
  ((addr_t) ((addr+(MAX_GRAIN_SIZE-1)) & MAX_GRAIN_MASK))

// compute (addr % 16) / GRAIN_SIZE 
#define ADDR_TO_MEM_INDEX(addr)                                       \
  (((addr_t)addr & (addr_t)(MAX_GRAIN_SIZE-1)) >> LOG_GRAIN_SIZE)

// compute size / GRAIN_SIZE 
#define SIZE_TO_NUM_GRAINS(size) (size >> LOG_GRAIN_SIZE)
class MemAccess_t {
public:
  bool promise_task;
  access_info task_and_node;
  addr_t rip;
#ifdef LINK_READER
  MemAccess_t* next;
  MemAccess_t* prev;
#endif
  MemAccess_t(access_info t_a_n, addr_t r, bool is_promise);
  ~MemAccess_t();
};

class MemAccessList_t {
public:
  addr_t start_addr;

  #ifdef LINK_READER
    MemAccess_t* readers[NUM_SLOTS] = {};
    MemAccess_t* readers_tail[NUM_SLOTS] = {};
  #else
    std::vector<MemAccess_t>* readers[NUM_SLOTS] = {};
  #endif

  #ifndef LOOP_READERS
    std::unordered_set<int>* readers_finish_id[NUM_SLOTS] = {};
  #endif


  MemAccess_t* writers[NUM_SLOTS] = {};

  MemAccessList_t(addr_t addr, bool is_read, access_info task_and_node, addr_t rip, std::size_t mem_size, int first_finish_id, bool is_promise);
  ~MemAccessList_t();
  
}; // end class MemAccessList_t

#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>
#include <list>
#include "struct_def.h"

using addr_t = uint64_t;

#define LOG_KEY_SIZE  4
#define LOG_TBL_SIZE 20

// macro for address manipulation for shadow mem
#define ADDR_TO_KEY(addr) ((addr_t) ((addr_t)addr >> LOG_KEY_SIZE))

#define GRAIN_SIZE 4
#define LOG_GRAIN_SIZE 2
#define MAX_GRAIN_SIZE (1 << LOG_KEY_SIZE)
#define NUM_SLOTS (MAX_GRAIN_SIZE / GRAIN_SIZE)
 
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
  access_info task_and_node;
  addr_t rip;
  MemAccess_t* next;
  MemAccess_t* prev;
  MemAccess_t(access_info t_a_n, addr_t r);
  //MemAccess_t(access_info t_a_n, addr_t r): task_and_node(t_a_n), rip(r) {}
};

class MemAccessList_t {
public:
  addr_t start_addr;
  MemAccess_t* readers[NUM_SLOTS] = {};
  //std::vector<MemAccess_t>* readers[NUM_SLOTS] = {};
  // MemAccess_t** readers[NUM_SLOTS] = {};
  // std::list<MemAccess_t*>* readers[NUM_SLOTS] = {};
  MemAccess_t* writers[NUM_SLOTS] = {};

  MemAccessList_t(addr_t addr, bool is_read, access_info task_and_node, addr_t rip, std::size_t mem_size);
  ~MemAccessList_t();
  
}; // end class MemAccessList_t

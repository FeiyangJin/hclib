#include <cstdio>
#include <cstdint>
#include <unordered_map>
#include <iterator>
#include "mem_access.h"

#ifndef __SHADOWMEM_H__                                                        
#define __SHADOWMEM_H__ 

#define LOG_KEY_SIZE  4
#define LOG_TBL_SIZE 20

// macro for address manipulation for shadow mem
// #define ADDR_TO_KEY(addr) ((uint64_t) ((uint64_t)addr >> LOG_KEY_SIZE))
// #define ADDR_TO_KEY(addr) ((uint64_t) (uint64_t)addr)
#define ADDR_TO_KEY(addr) ((uint64_t) (uint64_t)addr & 0xFFFFFFF)

class ShadowMem {
public:
  MemAccessList_t* hash_table[1<<30] = {nullptr};
  // std::unordered_map<uint64_t, MemAccessList_t*> huge_table;
  ShadowMem() {
    // huge_table.reserve(1<<40);
  }

  inline MemAccessList_t* find(uint64_t key) {
    return hash_table[key];
    // auto slot = huge_table.find(key);
    // if(slot == huge_table.end()){
    //   return nullptr;
    // }
    // return slot->second;
  }

  inline void insert(uint64_t key, MemAccessList_t *val) {
    hash_table[key] = val;
  }

  ~ShadowMem() {};
};

#endif // __SHADOWMEM_H__  

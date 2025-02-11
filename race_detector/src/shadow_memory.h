/*
Author: Feiyang Jin
Email: feiyanglovesu@gmail.com
shadow_memory.h contains the class ShadowMem, which will be used to save
access to each memory location.

The overall structure is:
ShadowMem has a shadow_dir

shadow_dir = [*shadow_tbl, *shadow_tbl, ...]      size: 2^24

shadow_tbl has a shadow_entries = [T*, T*, ...]   size: 2^20

Each slot in shadow_entries represent consecutive 16 bytes of memory


Pointer to slot:
For most system, 48 out of 64 bits of a pointer is used

[01010.......][101......][1001]
  24 bits      20 bits    4 bits 
Table index    
              slot index
                         ignored after converting pointer to key 

ADDR_TO_KEY(addr): convert a pointer address to a key (to the ShadoeMem)
*/

#include <cstdio>
#include <cstdint>

#ifndef __SHADOWMEM_H__                                                        
#define __SHADOWMEM_H__ 

#define LOG_KEY_SIZE  4
#define LOG_TBL_SIZE 20

// macro for address manipulation for shadow mem
// Notice this means each shadow_entry will handle consecutive 16 bytes of memory
// This is because the goal of shadow memory is to record access to a range of memory, 
// instead of each byte of memory. 
#define ADDR_TO_KEY(addr) ((uint64_t) ((uint64_t)addr >> LOG_KEY_SIZE))


template < typename T >
class ShadowMem {
public:
  // shadow_tbl contains an array of pointers of type T
  // the array is of size 1 << 20 = 2^20 = 1048576
  struct shadow_tbl { 
	  T *shadow_entries[1<<LOG_TBL_SIZE]; 
  }; 

  struct shadow_tbl **shadow_dir;

  // replace first parameter type with addr_t
  inline void clear_shadow_mem(uint64_t addr, int bytes) {
    int first_tbl_idx = addr >> (LOG_TBL_SIZE + LOG_KEY_SIZE);
    int last_tbl_idx = (addr + bytes) >> (LOG_TBL_SIZE + LOG_KEY_SIZE);
    int last_entry = ((addr + bytes) >> LOG_KEY_SIZE) & ((1<<LOG_TBL_SIZE) - 1);
    int tbl_size = 1 << LOG_TBL_SIZE;

    shadow_tbl *first_tbl = shadow_dir[first_tbl_idx];
    if (first_tbl) {
      int first_entry = (addr >> LOG_KEY_SIZE) & ((1<<LOG_TBL_SIZE) - 1);
      int last_entry_in_first_tbl = (first_tbl_idx == last_tbl_idx ? last_entry : tbl_size - 1);
      for (int j = first_entry; j <= last_entry_in_first_tbl && first_tbl->shadow_entries[j]; j++) {
        T *cell = first_tbl->shadow_entries[j];
        cell->clear();
      }
    }

    if (first_tbl_idx == last_tbl_idx) {
      return;
    }

    for (int i = first_tbl_idx + 1; i < last_tbl_idx && shadow_dir[i]; i++) {
      shadow_tbl *tbl = shadow_dir[i];
      for (int j = 0; j < tbl_size && tbl->shadow_entries[j]; j++) {
        T *cell = tbl->shadow_entries[j];
        cell->clear();
      }
    }

    shadow_tbl *last_tbl = shadow_dir[last_tbl_idx];
    if (last_tbl) {
      for (int j = 0; j <= last_entry && last_tbl->shadow_entries[j]; j++) {
        T *cell = last_tbl->shadow_entries[j];
        cell->clear();
      }
    }
  }

  inline T** find_slot(uint64_t key, bool alloc) {
    // shadow_dir has 2^24 table
    // 1. Choose the table by computing index = key >> 20
    // because key has 44 bits information, and we only use 24 bits for table index
    shadow_tbl *volatile *dest = &(shadow_dir[key>>LOG_TBL_SIZE]);
    shadow_tbl *tbl = *dest;

    if (!alloc && !tbl) {
      return NULL;
    } 
    else if (tbl == NULL) {
      struct shadow_tbl *new_tbl = new struct shadow_tbl();
      *dest = new_tbl;
      tbl = new_tbl;
      // do {
      //   tbl = __sync_val_compare_and_swap(dest, tbl, new_tbl);
      // } while(tbl == NULL);
      // if(tbl != new_tbl) { // someone got to the allocation first
      //   delete new_tbl; 
      // }
    }

    // 2. Choose the slot by extracting the lower LOG_TBL_SIZE bits of key
    // so slot index is the lower 20 bits of key
    T** slot =  &tbl->shadow_entries[key&((1<<LOG_TBL_SIZE) - 1)];
    return slot;
  }

// public:
  ShadowMem() {
    // Most consumer systems utilize 48 bits out of 64 bits for virtual memory addressing
    // This means even though a pointer is 64 bits, the upper 16 bits are always 0, only the lower 48 bits are possibly set. 
    shadow_dir = new struct shadow_tbl *[1<<(48 - LOG_TBL_SIZE - LOG_KEY_SIZE)](); 
  }

  inline T* find(uint64_t key) {
    T **slot = find_slot(key, false);
    if (slot == NULL)
      return NULL;
    return *slot;
  }


void insert_to_slot(T *volatile *slot, T *val) { 
    *slot = val;
}

  //  clear()
  //  return the value at the memory location when insert occurs
  //  If the value returned != val, insertion failed because someone
  //  else got to the slot first.  
  inline T * insert(uint64_t key, T *val) {
    T *volatile *slot = find_slot(key, true);
    T *old_val = *slot;
    *slot = val;
   
    // Note that old_val may not be val if someone else got to insert first
    return old_val;
  }

  /* XXX: we don't synchronize on erase --- this is called whenever  
   * we malloc a new block of memory (so shadow memory associated with the 
   * allocation is cleared), or when we return from spawned function 
   * (the cactus stack corresponding to the spawned function is cleared).
   * If another thread is accessing this memory while we clear it, the program
   * is accessing freed pointer or deallocated stack, which we assume does 
   * not occur.
   */
  void erase(uint64_t key) {
    T **slot = find_slot(key, false);
    if (slot != NULL) {
      if (*slot != NULL) {
        delete *slot;
        *slot = NULL;
      }
    }
  }

  ~ShadowMem() { }

};

#endif // __SHADOWMEM_H__  

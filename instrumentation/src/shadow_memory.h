#include <cstdio>
#include <cstdint>

#ifndef __SHADOWMEM_H__                                                        
#define __SHADOWMEM_H__ 

#define LOG_KEY_SIZE  4
#define LOG_TBL_SIZE 20

// macro for address manipulation for shadow mem
#define ADDR_TO_KEY(addr) ((uint64_t) ((uint64_t)addr >> LOG_KEY_SIZE))


template < typename T >
class ShadowMem {
public:
  // 1 << 20 = 1048576 = 2 ^ 20
  // shadow_entries is an array of size 1048576
  struct shadow_tbl { T *shadow_entries[1<<LOG_TBL_SIZE]; };

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
    // dest = key >> 20
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
    T** slot =  &tbl->shadow_entries[key&((1<<LOG_TBL_SIZE) - 1)];
    return slot;
  }

// public:
  ShadowMem() {
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

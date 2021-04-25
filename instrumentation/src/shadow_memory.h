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
private:
  struct shadow_tbl { T *shadow_entries[1<<LOG_TBL_SIZE]; };

  struct shadow_tbl **shadow_dir;

  inline T** find_slot(uint64_t key, bool alloc) {
    /* I think this volatile is necessary and sufficient ... */
    shadow_tbl *volatile *dest = &(shadow_dir[key>>LOG_TBL_SIZE]);
    shadow_tbl *tbl = *dest;

    if (!alloc && !tbl) {
      return NULL;
    } else if (tbl == NULL) {
      struct shadow_tbl *new_tbl = new struct shadow_tbl();
      do {
        tbl = __sync_val_compare_and_swap(dest, tbl, new_tbl);
      } while(tbl == NULL);

      if(tbl != new_tbl) { // someone got to the allocation first
        delete new_tbl; 
      }
    }
    T** slot =  &tbl->shadow_entries[key&((1<<LOG_TBL_SIZE) - 1)];
    return slot;
  }

public:
  ShadowMem() {
    shadow_dir = new struct shadow_tbl *[1<<(48 - LOG_TBL_SIZE - LOG_KEY_SIZE)]();
  }

  inline T* find(uint64_t key) {
    T **slot = find_slot(key, false);
    if (slot == NULL)
      return NULL;
    return *slot;
  }

  //  clear()
  //  return the value at the memory location when insert occurs
  //  If the value returned != val, insertion failed because someone
  //  else got to the slot first.  
  inline T * insert(uint64_t key, T *val) {
    T *volatile *slot = find_slot(key, true);
    T *old_val = *slot;
    
    while(old_val == NULL) {
      // retry as long as the old_val is still NULL
      old_val = __sync_val_compare_and_swap(slot, old_val,val);
      /* *slot = val; */
      /* old_val = val; */
    }
   
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

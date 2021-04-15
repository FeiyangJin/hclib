#include <stdio.h>
#include "assert.h"
#include "shadow_memory.h"
#include "mem_access.h"
#include "ds_to_hclib.h"

using addr_t = uint64_t;

extern ShadowMem<MemAccessList_t> shadow_mem;

int read_print_count = 0;
extern "C" void asap_check_read(int *addr, int bytes) {
  if(read_print_count < 5){
    printf("\nasap_check_read: %p, bytes: %d\n", addr, bytes);
    ds->print_table();
    read_print_count++;
  }
    
}

bool write_print_once = false;
extern "C" void asap_check_write(int *addr, int bytes) {
  if(!write_print_once){
    printf("\nasap_check_write: %p, bytes: %d\n", addr, bytes);
    write_print_once = true;
  }

}

extern "C" __attribute__((weak)) void ds_print(){
    printf("hello from check.cpp \n");
}

extern "C" bool precede(access_info previous_step, access_info current_step){
    return true;
}

access_info current_task_and_step;

void handle_write(MemAccessList_t* slot, addr_t rip, addr_t addr,
                                 size_t mem_size) {

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);
  
  assert(start >= 0);
  assert(start < NUM_SLOTS);

  // This may not be true, e.g. when start == 1...
  assert((start + grains) <= NUM_SLOTS);

  for (int i{start}; i < (start + grains); ++i) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == NULL) {
      writer = new MemAccess_t(current_task_and_step, rip);

      if(slot->writers[i] == NULL) {
        slot->writers[i] = writer;
      }

      if(writer == slot->writers[i]) continue;
      else { // was NULL but some other logically parallel strand updated it
          delete writer;
          writer = slot->writers[i];
      }
    }

    bool race = false;
    race = !precede(writer->task_and_node, current_task_and_step); 
    
    // replace the last writer regardless
    slot->writers[i]->task_and_node = current_task_and_step;
    slot->writers[i]->rip     = rip;

  }

  for(int i = start; i < (start + grains); i++) {
    MemAccess_t *reader = slot->readers[i];
    if (reader == NULL) continue;
    bool race = false;
    race = !precede(reader->task_and_node, current_task_and_step);
  }
  
}
#include <stdio.h>
#include "assert.h"
#include "shadow_memory.h"
#include "mem_access.h"
#include "ds_to_hclib.h"

static ShadowMem<MemAccessList_t> shadow_mem;


extern "C" __attribute__((weak)) void ds_print(){
    printf("hello from check.cpp \n");
}

extern "C" bool precede(access_info previous_step, access_info current_step){
    return true;
}

void handle_read(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  for(int i = start; i < (start + grains); i++) {
    MemAccess_t *writer = slot->writers[i]; // can be NULL
    if (writer == NULL) continue;
    bool race = false;
    race = !precede(writer->task_and_node, current_task_and_step);
  } // end of all grains writer

  
  for(int i = start; i < (start + grains); i++) {
    std::vector<MemAccess_t*> *readers = slot->readers[i];

    if(readers == NULL) {
      MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip);
      slot->readers[i] = new std::vector<MemAccess_t*>;
      slot->readers[i]->push_back(new_reader);
    }
    else{
      bool update = false;
      for(auto reader = readers->begin(); reader != readers->end(); reader++){
        if(precede((*reader)->task_and_node,current_task_and_step)){
          update = true;
          // TODO: how to remove reader from this vector ?
        }
        else{
          tree_node_cpp* current_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(current_task_and_step.task_id)->node_in_dpst);
          tree_node_cpp* reader_task_dpst_node = (tree_node_cpp*)(ds->get_task_info((*reader)->task_and_node.task_id)->node_in_dpst);
          if(current_task_dpst_node->this_node_type == FUTURE || reader_task_dpst_node->this_node_type == FUTURE){
            update = true;
          }
        }
      } // end of checking all readers in a single grain

      if(update == true){
        MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip);
        slot->readers[i]->push_back(new_reader);
      }
    }

  } // end of checking all grains readers.
}



void handle_write(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

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
    if(race){
      printf("we find a race \n");
      assert(0);
    }

    // replace the last writer regardless
    slot->writers[i]->task_and_node = current_task_and_step;
    slot->writers[i]->rip     = rip;

  }

  for(int i = start; i < (start + grains); i++) {
    std::vector<MemAccess_t*> *readers = slot->readers[i];
    if (readers == NULL) continue;
    bool race = false;
    for(auto reader = readers->begin(); reader != readers->end(); reader ++){
      race = !precede((*reader)->task_and_node, current_task_and_step);
    }

    if(race){
      printf("we find a race \n");
      assert(0);
    }
    else{
      // clear all readers if no race
      slot->readers[i]->clear();
    }

  }
  
}


int write_print_count = 0;
extern "C" void asap_check_write(int *addr, int bytes) {

  // printf("pointer is %xl, size: %d \n",addr, sizeof(addr));
  // addr_t address = (addr_t)(addr);
  // printf("address is %xl, size: %d \n",address,sizeof(address));
  //auto slot = shadow_mem.find(ADDR_TO_KEY(address));
  
  // if(slot == nullptr){
  //   MemAccessList_t *mem_list  = new MemAccessList_t(*addr, false, current_task_and_step, NULL, bytes);
  //   slot = shadow_mem.insert(ADDR_TO_KEY(addr), mem_list);
  //   if (slot != mem_list) {
  //       delete mem_list;
  //   } else {
  //       return;
  //   }
  // }

  // handle_write(slot, NULL, *addr, bytes);

}


int read_print_count = 0;
extern "C" void asap_check_read(int *addr, int bytes) {
  if(hclib_ready == true){
    printf("\nasap_check_read: %p, bytes: %d\n", addr, bytes);
    if(hclib_current_task_id != NULL){
      printf("called in check.cpp, hclib current task id is: %d \n", hclib_current_task_id());
    }

    if(hclib_current_step_node != NULL){
      tree_node_cpp* curr_tree_node = (tree_node_cpp*) hclib_current_step_node();
      printf("current step node index: %d \n",curr_tree_node->index);
    }
    read_print_count++;
  }
    
}
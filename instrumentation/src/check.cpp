#include <stdio.h>
#include <typeinfo>
#include "assert.h"
#include "shadow_memory.h"
#include "mem_access.h"
#include "ds_to_hclib.h"

static ShadowMem<MemAccessList_t> *shadow_mem = new ShadowMem<MemAccessList_t>();


extern "C" __attribute__((weak)) void ds_print(){
    printf("hello from check.cpp \n");
}


extern "C" bool precede(access_info previous_step, access_info current_step){
  int p_id = previous_step.task_id;
  tree_node_cpp *p_node = (tree_node_cpp*) previous_step.node_in_dpst;
  int c_id = current_step.task_id;
  tree_node_cpp *c_node = (tree_node_cpp*) current_step.node_in_dpst;

  //printf("previous node: %d, current node %d \n",p_node->index,c_node->index);
  return ds->precede(p_node, c_node, p_id, c_id);
}


void handle_read(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);
  access_info current_task_and_step = {
    .task_id = hclib_current_task_id(),
    .node_in_dpst = hclib_current_step_node()
  };

  for(int i = start; i < (start + grains); i++) {
    MemAccess_t *writer = slot->writers[i]; // can be NULL
    if(writer == nullptr){
      continue;
    }

    bool race = false;
    race = !precede(writer->task_and_node, current_task_and_step);
    if(race){
      printf("we find a race \n");
      assert(0);
    }
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
      auto reader = readers->begin();

      while(reader != readers->end()){
        if(precede((*reader)->task_and_node,current_task_and_step)){
          update = true;
          reader = readers->erase(reader);
        }
        else{
          tree_node_cpp* current_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(current_task_and_step.task_id)->node_in_dpst);
          tree_node_cpp* reader_task_dpst_node = (tree_node_cpp*)(ds->get_task_info((*reader)->task_and_node.task_id)->node_in_dpst);
          if(current_task_dpst_node->this_node_type == FUTURE || reader_task_dpst_node->this_node_type == FUTURE){
            update = true;
          }
          ++reader;
        }
      } // remove reader happened-before us while iterating the reader list

      // for(auto reader = readers->begin(); reader != readers->end(); reader++){
      //   if(precede((*reader)->task_and_node,current_task_and_step)){
      //     update = true;
      //     // TODO: how to remove reader from this vector ?
      //   }
      //   else{
      //     tree_node_cpp* current_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(current_task_and_step.task_id)->node_in_dpst);
      //     tree_node_cpp* reader_task_dpst_node = (tree_node_cpp*)(ds->get_task_info((*reader)->task_and_node.task_id)->node_in_dpst);
      //     if(current_task_dpst_node->this_node_type == FUTURE || reader_task_dpst_node->this_node_type == FUTURE){
      //       update = true;
      //     }
      //   }
      // } // end of checking all readers in a single grain

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
  access_info current_task_and_step = {
    .task_id = hclib_current_task_id(),
    .node_in_dpst = hclib_current_step_node()
  };

  //printf("    handle write, current node is %d, address %p \n",((tree_node_cpp*)hclib_current_step_node())->index ,addr);
  for (int i=start; i < (start + grains); ++i) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == NULL) {
      writer = new MemAccess_t(current_task_and_step, rip);
      slot->writers[i] = writer;
      continue;
    }

    assert(writer->task_and_node.node_in_dpst != nullptr);
    bool race = false;
    race = !precede(writer->task_and_node, current_task_and_step); 
    if(race){
      printf("we find a race !!!!!!!!!! \n");
      assert(0);
    }

    // replace the last writer regardless
    slot->writers[i]->task_and_node.node_in_dpst = current_task_and_step.node_in_dpst;
    slot->writers[i]->task_and_node.task_id = current_task_and_step.task_id;
    slot->writers[i]->rip     = rip;
    assert(writer->task_and_node.node_in_dpst != nullptr);
  } // end of checking writers

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
  //printf("asap check write, address %p \n",addr);
  if(hclib_ready == true){
    //printf("asap check write, current node is %d, address %p \n",((tree_node_cpp*)hclib_current_step_node())->index ,addr);
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));

    access_info current_task_and_step = {
      .task_id = hclib_current_task_id(),
      .node_in_dpst = hclib_current_step_node()
    };

    if(((tree_node_cpp*)current_task_and_step.node_in_dpst)->this_node_type != STEP){
      return;
    }

    if(slot == nullptr){
      MemAccessList_t *mem_list  = new MemAccessList_t(*addr, false, current_task_and_step, 0, bytes);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_write(slot, 0, (addr_t)addr, bytes);
  }

}


int read_print_count = 0;
extern "C" void asap_check_read(int *addr, int bytes) {
  if(hclib_ready == true){
    //printf("asap check read, address %p \n",addr);
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));

    access_info current_task_and_step = {
      .task_id = hclib_current_task_id(),
      .node_in_dpst = hclib_current_step_node()
    };

    if(((tree_node_cpp*)current_task_and_step.node_in_dpst)->this_node_type != STEP){
      return;
    }

    if(slot == nullptr){
      MemAccessList_t *mem_list  = new MemAccessList_t(*addr, true, current_task_and_step, 0, bytes);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_read(slot,0,(addr_t)addr,bytes);

    //printf("\nasap_check_read: %p, bytes: %d\n", addr, bytes);
    //printf("called in check.cpp, hclib current task id is: %d \n", hclib_current_task_id());
    //tree_node_cpp* curr_tree_node = (tree_node_cpp*) hclib_current_step_node();
    //printf("current step node index: %d \n",curr_tree_node->index);
    
  }
    
}
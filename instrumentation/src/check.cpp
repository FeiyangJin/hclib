#include <stdio.h>
#include <typeinfo>
#include "assert.h"
#include "shadow_memory.h"
#include "mem_access.h"
#include "ds_to_hclib.h"
#include <chrono>
#include <ctime> 

static ShadowMem<MemAccessList_t> *shadow_mem = new ShadowMem<MemAccessList_t>();
access_info current_task_and_step;

extern "C" __attribute__((weak)) void ds_print(){
    printf("hello from check.cpp \n");
}


extern "C" bool precede(access_info previous_step, access_info current_step){
  int p_id = previous_step.task_id;
  tree_node_cpp *p_node = (tree_node_cpp*) previous_step.node_in_dpst;
  int c_id = current_step.task_id;
  tree_node_cpp *c_node = (tree_node_cpp*) current_step.node_in_dpst;

  return ds->precede(p_node, c_node, p_id, c_id);
}


void handle_read(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  for(int i = start; i < (start + grains); i++) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == nullptr){
      continue;
    }

    bool race = !precede(writer->task_and_node, current_task_and_step);
  } // end of all grains writer

  
  for(int i = start; i < (start + grains); i++) {
    MemAccess_t* reader = slot->readers[i];

    if(reader == nullptr){
      reader = new MemAccess_t(current_task_and_step, rip);
      continue;
    }
    else if(reader->next == nullptr){
      // we only have 1 reader
      if(precede(reader->task_and_node,current_task_and_step)){
        reader->rip = rip;
        reader->task_and_node.node_in_dpst = current_task_and_step.node_in_dpst;
        reader->task_and_node.task_id = current_task_and_step.task_id;
      }
      else{
          tree_node_cpp* current_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(current_task_and_step.task_id)->node_in_dpst);
          tree_node_cpp* reader_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(reader->task_and_node.task_id)->node_in_dpst);
          if(current_task_dpst_node->this_node_type == FUTURE || reader_task_dpst_node->this_node_type == FUTURE){ 
            MemAccess_t* current_reader = new MemAccess_t(current_task_and_step, rip);
            reader->next = current_reader;
            current_reader->prev = reader;
          }
      }
    }
    else{
      bool update = false;
      MemAccess_t* last_reader = reader;
      while(reader != nullptr){
        if(precede(reader->task_and_node,current_task_and_step)){
          update = true;
          if(reader->prev != nullptr){
            reader->prev->next = reader->next;
          }
          
          if(reader->next != nullptr){
            reader->next->prev = reader->prev;
          }
          last_reader = reader;
          reader = reader->next;
        }
        else{
          tree_node_cpp* current_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(current_task_and_step.task_id)->node_in_dpst);
          tree_node_cpp* reader_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(reader->task_and_node.task_id)->node_in_dpst);
          if(current_task_dpst_node->this_node_type == FUTURE || reader_task_dpst_node->this_node_type == FUTURE){update = true;}
          last_reader = reader;
          reader = reader->next;
        }

      }

      if(update){
        MemAccess_t* current_reader = new MemAccess_t(current_task_and_step, rip);
        last_reader->next = current_reader;
        current_reader->prev = last_reader;
      }
    }
      
      // MemAccess_t *readers = slot->readers[i];
      // if(readers == NULL){
      //   readers = new MemAccess_t(current_task_and_step, rip);
      //   continue;
      // }
      // slot->readers[i]->task_and_node = current_task_and_step;
      // slot->readers[i]->rip = rip;

    // std::vector<MemAccess_t> *readers = slot->readers[i];
    // if(readers == nullptr) {
    //   readers = new std::vector<MemAccess_t>();
    //   readers->push_back(MemAccess_t(current_task_and_step, rip));
    //   continue;
    // }
    // else{
    //   bool update = false;
    //   auto reader = readers->begin();
    //   while(reader != readers->end()){
    //     auto start = std::chrono::system_clock::now();
    //     bool p = precede((*reader).task_and_node,current_task_and_step);
    //     auto end = std::chrono::system_clock::now();
    //     std::chrono::duration<double> elapsed_seconds = end-start;
    //     // cout << "precede takes time: " << elapsed_seconds.count() << endl;
    //     if(precede((*reader).task_and_node,current_task_and_step)){
    //       update = true;
    //       reader = readers->erase(reader);
    //     }
    //     else{
    //       tree_node_cpp* current_task_dpst_node = (tree_node_cpp*)(ds->get_task_info(current_task_and_step.task_id)->node_in_dpst);
    //       tree_node_cpp* reader_task_dpst_node = (tree_node_cpp*)(ds->get_task_info((*reader).task_and_node.task_id)->node_in_dpst);
    //       if(current_task_dpst_node->this_node_type == FUTURE || reader_task_dpst_node->this_node_type == FUTURE){
    //         update = true;
    //       }
    //       ++reader;
    //     }
    //   } // remove reader happened-before us while iterating the reader list
    //   if(update == true){
    //     slot->readers[i]->push_back(MemAccess_t(current_task_and_step, rip));
    //   }
    // }

  } // end of checking all grains readers.
}


void handle_write(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  for (int i=start; i < (start + grains); ++i) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == NULL) {
      writer = new MemAccess_t(current_task_and_step, rip);
      slot->writers[i] = writer;
      continue;
    }

    assert(writer->task_and_node.node_in_dpst != nullptr);
    bool race = !precede(writer->task_and_node, current_task_and_step); 
    // if(race){
    //   printf("we find a race !!!!!!!!!! \n");
    //   tree_node_cpp* p_node = (tree_node_cpp*)writer->task_and_node.node_in_dpst;
    //   tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
    //   printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, writer->task_and_node.task_id, current_task_and_step.task_id);
    //   printf("addr %lx, mem_size %zu \n",addr,mem_size);
    //   printf("previous op is %lx, current op is %lx\n", writer->rip, rip);
    //   // hclib_print_dpst();
    //   // //ds->print_all_tasks();
    //   // //ds->print_table();
    //   assert(0);
    // }

    // replace the last writer
    slot->writers[i]->task_and_node.node_in_dpst = current_task_and_step.node_in_dpst;
    slot->writers[i]->task_and_node.task_id = current_task_and_step.task_id;
    slot->writers[i]->rip     = rip;
    assert(writer->task_and_node.node_in_dpst != nullptr);
  } // end of checking writers


  for(int i = start; i < (start + grains); i++) {
    //std::vector<MemAccess_t> *readers = slot->readers[i];
    MemAccess_t* reader = slot->readers[i];
    if (reader == nullptr) continue;

    //auto reader = readers->begin();
    while(reader != nullptr){
      //reader++;
      bool race = !precede(reader->task_and_node, current_task_and_step);
      //MemAccess_t* temp = reader->next;
      reader = reader->next;
      if(reader != nullptr){
        delete reader->prev;
        reader->prev = nullptr;
      }
    }

    //clear all readers if no race
    delete slot->readers[i];
    slot->readers[i] = nullptr;
    //slot->readers[i]->clear();
    
  }
  
}


int check_write_count = 0;
extern "C" void asap_check_write(int *addr, int bytes) {

  if(hclib_ready == true){
    void *pc = __builtin_return_address(0);
    // auto start = std::chrono::system_clock::now();
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));
    // auto end = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed_seconds = end-start;
    //cout << elapsed_seconds.count() << endl;

    current_task_and_step.task_id = hclib_current_task_id();
    current_task_and_step.node_in_dpst = hclib_current_step_node();

    if(((tree_node_cpp*)current_task_and_step.node_in_dpst)->this_node_type != STEP){
      return;
    }

    if(slot == nullptr){
      MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, false, current_task_and_step, (addr_t)pc, bytes);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_write(slot, (addr_t)pc, (addr_t)addr, bytes);
  }

}


unsigned long read_count = 0;
extern "C" void asap_check_read(int *addr, int bytes) {

  if(hclib_ready == true){
    //printf("read count %lu \n", read_count);
    read_count++;
    void *pc = __builtin_return_address(0);
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));

    current_task_and_step.task_id = hclib_current_task_id();
    current_task_and_step.node_in_dpst = hclib_current_step_node();

    if(((tree_node_cpp*)current_task_and_step.node_in_dpst)->this_node_type != STEP){
      return;
    }

    if(slot == nullptr){
      MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, true, current_task_and_step, (addr_t)pc, bytes);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_read(slot,0,(addr_t)addr,bytes);
  }
    
}
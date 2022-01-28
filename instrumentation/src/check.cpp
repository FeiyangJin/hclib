#include <stdio.h>
#include <typeinfo>
#include "assert.h"
#include "shadow_memory.h"
#include "mem_access.h"
#include "ds_to_hclib.h"

static ShadowMem<MemAccessList_t> *shadow_mem = new ShadowMem<MemAccessList_t>();
access_info current_task_and_step;
static int current_finish_id;
static bool is_step = false;
static bool is_future = false;
static bool is_asap_promise_task = false;
static unsigned long check_write_count = 0;
static unsigned long check_read_count = 0;

static int current_step_id = -1;
robin_hood::unordered_set<int*> address_already_visit;
bool step_first_write = true;
static int a_count = 0;
static int reachability_count = 0;

extern "C" __attribute__((weak)) void ds_print_check_write_count(){
  printf("check write count: %lu \n", check_write_count);
}

extern "C" __attribute__((weak)) void ds_print_check_read_count(){
  printf("check read count: %lu \n", check_read_count);
  printf("ds find count %d \n",ds->get_find_count());
  // printf("duplicate read skipped count %d \n",a_count);
  printf("reachability check %d \n",reachability_count);
}

extern "C" void ds_promise_task(bool b){
  is_asap_promise_task = b;
}


//int bool_count = 0;
extern "C" bool precede(access_info previous_step, access_info current_step){
  reachability_count++;
  // return true;
  int p_id = previous_step.task_id;
  tree_node_cpp *p_node = (tree_node_cpp*) previous_step.node_in_dpst;
  int c_id = current_step.task_id;
  tree_node_cpp *c_node = (tree_node_cpp*) current_step.node_in_dpst;

  bool result = ds->precede(p_node, c_node, p_id, c_id);

  return result;
}


extern "C" void handle_read(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);

  for(int i = start; i < (start + grains); i++) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == nullptr){
      continue;
    }

    bool race = !precede(writer->task_and_node, current_task_and_step);
    if(race){
      printf("we find a read-write race !!!!!!!!!! \n");
      tree_node_cpp* p_node = (tree_node_cpp*)writer->task_and_node.node_in_dpst;
      tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
      printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, writer->task_and_node.task_id, current_task_and_step.task_id);
      printf("addr %lx, mem_size %zu \n",addr,mem_size);
      printf("previous op is %lx, current op is %lx\n", writer->rip, rip);
      assert(0);
    }
  } // end of all grains writer

  for(int i = start; i < (start + grains); i++) {

      #ifdef LINK_READER
          MemAccess_t* reader = slot->readers[i];
          if(reader == nullptr){ // 1. we have no previous reader
            MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
            slot->readers[i] = new_reader;
            slot->readers_tail[i] = new_reader;
            continue;
          }
          // else if (reader->next == nullptr){ // 2. we only have one reader
          //   if(reader->task_and_node.task_id == c_id){
          //     reader->rip = rip;
          //     reader->task_and_node = current_task_and_step;
          //     continue;
          //   }
          //   // otherwise add the reader directly
          //   MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
          //   slot->readers_tail[i]->next = new_reader;
          //   new_reader->prev = slot->readers_tail[i];

          //   slot->readers_tail[i] = new_reader;
          // }
          else{ // 3. we have more than 1 reader
            bool update = true;
            // int c_id = current_task_and_step.task_id;
            // while(reader != nullptr){
            //   if(reader->task_and_node.task_id == c_id){
            //     reader->task_and_node = current_task_and_step;
            //     update = false;
            //     break;
            //   }
            //   reader = reader->next;
            // }
            if(update){
              MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
              slot->readers_tail[i]->next = new_reader;
              new_reader->prev = slot->readers_tail[i];

              slot->readers_tail[i] = new_reader;
            }
          }
      #elif defined(VECTOR_READER_LIST)
          vector<MemAccess_t> *reader = slot->readers[i];

          if(reader == nullptr){
            slot->readers[i] = new std::vector<MemAccess_t>();
            // slot->readers[i]->reserve(10);
            slot->readers[i]->push_back(MemAccess_t(current_task_and_step,rip,is_asap_promise_task));
          }
          else if (reader->size() == 1)
          {
            if(reader->at(0).task_and_node.task_id == c_id){
              reader->at(0) = MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
              continue;
            }
            reader->push_back(MemAccess_t(current_task_and_step, rip, is_asap_promise_task));
          }
          else{
            // if we have a vector of readers, how should we decide:
            // Add the new reader or not
            // Remove any previous reader or not
            #ifdef LOOP_READERS
              bool update = true;
              auto r = reader->begin();
              while(r != reader->end()){
                if(r->task_and_node.task_id == c_id){
                  r->task_and_node.node_in_dpst = current_task_and_step.node_in_dpst;
                  r->rip = rip;
                  update = false;
                  break;
                }
                r++;
              }
              if(update){
                reader->push_back(MemAccess_t(current_task_and_step, rip, is_asap_promise_task));
              }
            #else
              reader->push_back(MemAccess_t(current_task_and_step, rip, is_asap_promise_task));
            #endif
          }
      #else
          unordered_map<int,MemAccess_t> *reader = slot->readers[i];
          if(reader == nullptr){
            slot->readers[i] = new std::unordered_map<int,MemAccess_t>();
            slot->readers[i]->insert(std::pair<int,MemAccess_t>(c_id, MemAccess_t(current_task_and_step,rip,is_asap_promise_task)));
            continue;
          }
          else{
            #ifdef LOOP_READERS
              bool update = true;
              auto r = reader->begin();
              while(r != reader->end()){
                if(r->task_and_node.task_id == c_id){
                  r->task_and_node.node_in_dpst = current_task_and_step.node_in_dpst;
                  update = false;
                  break;
                }

                r++;
              }

              if(update){
                reader->push_back(MemAccess_t(current_task_and_step, rip, is_asap_promise_task));
              }
            #else
              if(reader->find(c_id) == reader->end()){
                reader->insert(std::pair<int,MemAccess_t>(c_id, MemAccess_t(current_task_and_step,rip,is_asap_promise_task)));
              }
              else{
                reader->at(c_id) = MemAccess_t(current_task_and_step,rip,is_asap_promise_task);
              }
            #endif
          }
      #endif
  } // end of all grains readers
}


extern "C" void handle_write(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);
  for (int i=start; i < (start + grains); ++i) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == NULL) {
      slot->writers[i] = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
      continue;
    }

    bool race = !precede(writer->task_and_node, current_task_and_step); 
    if(race){
      printf("we find a write-write race !!!!!!!!!! \n");
      tree_node_cpp* p_node = (tree_node_cpp*)writer->task_and_node.node_in_dpst;
      tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
      printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, writer->task_and_node.task_id, current_task_and_step.task_id);
      printf("addr %lx, mem_size %zu \n",addr,mem_size);
      printf("previous op is %lx, current op is %lx\n", writer->rip, rip);
      assert(0);
    }

    // update writer
    writer->task_and_node = current_task_and_step;
    writer->rip     = rip;
  } // end of checking writers


  for(int i = start; i < (start + grains); i++) {
    #ifdef LINK_READER
        MemAccess_t* reader = slot->readers[i];
        if (reader == nullptr) continue;

        while(reader != nullptr){
          bool race = !precede(reader->task_and_node, current_task_and_step);
          if(race){
            printf("we find a write-read race !!!!!!!!!! \n");
            tree_node_cpp* p_node = (tree_node_cpp*)reader->task_and_node.node_in_dpst;
            tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
            printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, reader->task_and_node.task_id, current_task_and_step.task_id);
            printf("addr %lx, mem_size %zu \n",addr,mem_size);
            printf("previous op is %lx, current op is %lx\n", reader->rip, rip);
            assert(0);
          }
          
          reader = reader->next;
          if(reader != nullptr){
            delete reader->prev;
          }
        }
        
        slot->readers[i] = nullptr;
        slot->readers_tail[i] = nullptr;
    #elif defined(VECTOR_READER_LIST)
      vector<MemAccess_t>* reader = slot->readers[i];
      if (reader == nullptr) continue;

      auto r = reader->begin();
      // std::unordered_set<int> past_ids;
      while(r != reader->end()){
          // if(past_ids.find(r->task_and_node.task_id) != past_ids.end()){
          //   r++;
          //   continue;
          // }
          bool race = !precede(r->task_and_node, current_task_and_step);
          if(race){
            printf("we find a write-read race !!!!!!!!!! \n");
            tree_node_cpp* p_node = (tree_node_cpp*)r->task_and_node.node_in_dpst;
            tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
            printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, r->task_and_node.task_id, current_task_and_step.task_id);
            printf("addr %lx, mem_size %zu \n",addr,mem_size);
            printf("previous op is %lx, current op is %lx\n", r->rip, rip);
            assert(0);
          }
          r++;
      }
      // slot->readers[i]->clear();
      slot->readers[i] = nullptr;
    #else
        unordered_map<int,MemAccess_t>* reader = slot->readers[i];
        if (reader == nullptr) continue;
        
        auto r = reader->begin();
        while(r != reader->end()){
          bool race = !precede(r->second.task_and_node, current_task_and_step);
          if(race){
            printf("we find a write-read race !!!!!!!!!! \n");
            tree_node_cpp* p_node = (tree_node_cpp*)r->second.task_and_node.node_in_dpst;
            tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
            printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, r->second.task_and_node.task_id, current_task_and_step.task_id);
            printf("addr %lx, mem_size %zu \n",addr,mem_size);
            printf("previous op is %lx, current op is %lx\n", r->second.rip, rip);
            assert(0);
          }
          r++;

        }
        slot->readers[i]->clear();
        slot->readers[i] = nullptr;
    #endif
  }
  
}


extern "C" void asap_check_write(int *addr, int bytes) {

  if(hclib_ready == true){
    check_write_count++;

    current_task_and_step.node_in_dpst = hclib_get_current_task_info(&current_task_and_step.task_id,&current_finish_id, &is_step, &is_future);

    if(!is_step){
      return;
    }

    // int stepid = ((tree_node_cpp*) current_task_and_step.node_in_dpst)->index;
    // if(stepid != current_step_id){
    //   current_step_id = stepid;
    //   address_already_visit.clear();
    //   address_already_visit.insert(addr);
    //   step_first_write = true;
    // }
    // else{
    //   if(address_already_visit.count(addr) > 0 && !step_first_write){
    //     a_count ++;
    //     return;
    //   }
    //   else if(address_already_visit.count(addr) > 0 && step_first_write){
    //     step_first_write = false;
    //   }
    //   else{
    //     address_already_visit.insert(addr);
    //   }
    // }

    void *pc = __builtin_return_address(0);
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));

    if(slot == NULL){
      MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, false, current_task_and_step, (addr_t)pc, bytes, current_finish_id, is_asap_promise_task);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_write(slot, (addr_t)pc, (addr_t)addr, bytes);
  }

}

extern "C" __attribute__((weak)) void asap_check_read(int *addr, int bytes) {
  if(hclib_ready == true){
    check_read_count++;

    current_task_and_step.node_in_dpst = hclib_get_current_task_info(&current_task_and_step.task_id,&current_finish_id, &is_step, &is_future);

    if(!is_step){
      return;
    }

    // int stepid = ((tree_node_cpp*) current_task_and_step.node_in_dpst)->index;
    // if(stepid != current_step_id){
    //   current_step_id = stepid;
    //   address_already_visit.clear();
    //   address_already_visit.insert(addr);
    //   step_first_write = true;
    // }
    // else{
    //   if(address_already_visit.count(addr) > 0){
    //     a_count ++;
    //     return;
    //   }
    //   else{
    //     address_already_visit.insert(addr);
    //   }
    // }

    void *pc = __builtin_return_address(0);
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));

    if(slot == nullptr){
      MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, true, current_task_and_step, (addr_t)pc, bytes, current_finish_id, is_asap_promise_task);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_read(slot,(addr_t)pc,(addr_t)addr,bytes);
  }
    
}
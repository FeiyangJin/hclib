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
static int current_finish_id;
static bool is_step = false;
static bool is_future = false;
static bool is_asap_promise_task = false;

extern "C" void ds_promise_task(bool b){
  is_asap_promise_task = b;
}


extern "C" void ds_free(void* ptr){
  auto slot = shadow_mem->find(ADDR_TO_KEY(ptr));
  if(slot != nullptr){
    auto mem_size = sizeof(ptr);
    const int start = ADDR_TO_MEM_INDEX(ptr);
    const int grains = SIZE_TO_NUM_GRAINS(mem_size);
    for(int i = start; i < (start + grains); i++) {
      slot->writers[i] = nullptr;
      slot->readers[i] = nullptr;
      slot->readers_tail[i] = nullptr;
    }
  }
  free(ptr);
}

// extern "C" void ds_print(){
//     printf("hello from check.cpp \n");
// }

//int bool_count = 0;
extern "C" bool precede(access_info previous_step, access_info current_step){
  // return true;
  int p_id = previous_step.task_id;
  tree_node_cpp *p_node = (tree_node_cpp*) previous_step.node_in_dpst;
  int c_id = current_step.task_id;
  tree_node_cpp *c_node = (tree_node_cpp*) current_step.node_in_dpst;

  bool result = ds->precede(p_node, c_node, p_id, c_id);

  return result;
}

extern "C" bool dpst_precede(access_info previous_step, access_info current_step){
  tree_node_cpp *p_node = (tree_node_cpp*) previous_step.node_in_dpst;
  tree_node_cpp *c_node = (tree_node_cpp*) current_step.node_in_dpst;
  return ds->precede_dpst(p_node,c_node);
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
  } // end of all grains writer

  
  for(int i = start; i < (start + grains); i++) {
      int c_id = current_task_and_step.task_id;

      #ifdef LINK_READER
          MemAccess_t* reader = slot->readers[i];
          if(reader == nullptr){ // 1. we have no previous reader
            MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
            slot->readers[i] = new_reader;
            slot->readers_tail[i] = new_reader;
            continue;
          }
          else if (reader->next == nullptr){ // 2. we only have one reader
            if(reader->task_and_node.task_id == c_id){
              reader->rip = rip;
              reader->task_and_node = current_task_and_step;
              continue;
            }

            if(reader->promise_task || is_asap_promise_task){
              MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
              slot->readers_tail[i]->next = new_reader;
              new_reader->prev = slot->readers_tail[i];

              slot->readers_tail[i] = new_reader;
            }
          }
          else{ // 3. we have more than 1 reader
            bool update = false;
            while(reader != nullptr){
              if(reader->task_and_node.task_id == c_id){
                update = true;
                if(reader->prev != nullptr){
                  reader->prev->next = reader->next;
                }

                if(reader->next != nullptr){
                  reader->next->prev = reader->prev;
                }

                if(reader == slot->readers_tail[i]){
                  slot->readers_tail[i] = reader->prev;
                }

                if(reader == slot->readers[i]){
                  slot->readers[i] = reader->next;
                }
                delete reader;
                break;
              }
              else if(reader->promise_task){
                update = true;
              }
              reader = reader->next;
            }

            if(update || is_asap_promise_task){
              MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
              slot->readers_tail[i]->next = new_reader;
              new_reader->prev = slot->readers_tail[i];

              slot->readers_tail[i] = new_reader;
            }
          }
      #else
          vector<MemAccess_t> *reader = slot->readers[i];
          if(reader == nullptr){
            slot->readers[i] = new std::vector<MemAccess_t>();
            slot->readers[i]->push_back(MemAccess_t(current_task_and_step,rip,is_asap_promise_task));

            #ifndef LOOP_READERS
                  slot->readers_finish_id[i] = new std::unordered_set<int>();
                  // slot->readers_finish_id[i]->insert(c);
            #endif
            continue;
          }
          else if(reader->size() == 0){
            reader->push_back(MemAccess_t(current_task_and_step, rip,is_asap_promise_task));
            #ifndef LOOP_READERS
                  finish_ids->insert(c);
            #endif
          }
          else{
            #ifdef LOOP_READERS
              bool update = false;
              auto r = reader->begin();
              while(r != reader->end()){
                if(r->task_and_node.task_id == c_id){
                  r = reader->erase(r);
                  update = true;
                  break;
                }

                if(r->promise_task){
                  update = true;
                }
                r++;
              }

              if(update || is_asap_promise_task){
                reader->push_back(MemAccess_t(current_task_and_step, rip, is_asap_promise_task));
              }
            #else
              bool other_sibling_in_set = finish_ids->find(c) != finish_ids->end();
              // ds->get_cache_size();
              if(other_sibling_in_set == false){
                reader->push_back(MemAccess_t(current_task_and_step, rip));
                finish_ids->insert(c);
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
    // if(race){
    //   printf("we find a race !!!!!!!!!! \n");
    //   tree_node_cpp* p_node = (tree_node_cpp*)writer->task_and_node.node_in_dpst;
    //   tree_node_cpp* c_node = (tree_node_cpp*)current_task_and_step.node_in_dpst;
    //   printf("previous step index: %d, current step index: %d, previous task %d, current task %d \n", p_node->index, c_node->index, writer->task_and_node.task_id, current_task_and_step.task_id);
    //   printf("addr %lx, mem_size %zu \n",addr,mem_size);
    //   printf("previous op is %lx, current op is %lx\n", writer->rip, rip);
    //   assert(0);
    // }

    writer->task_and_node.node_in_dpst = current_task_and_step.node_in_dpst;
    writer->task_and_node.task_id = current_task_and_step.task_id;
    writer->rip     = rip;
  } // end of checking writers


  for(int i = start; i < (start + grains); i++) {
    #ifdef LINK_READER
        MemAccess_t* reader = slot->readers[i];
        if (reader == nullptr) continue;
        while(reader != nullptr){
          bool race = !precede(reader->task_and_node, current_task_and_step);
          reader = reader->next;
          if(reader != nullptr){
            delete reader->prev;
          }
        }
        slot->readers[i] = nullptr;
        slot->readers_tail[i] = nullptr;
    #else
        vector<MemAccess_t>* reader = slot->readers[i];
        if (reader == nullptr) continue;
        // bool race = !precede(reader->task_and_node, current_task_and_step);
        
        auto r = reader->begin();
        while(r != reader->end()){
          bool race = !precede(r->task_and_node, current_task_and_step);
          r++;
        }
        slot->readers[i]->clear();
        slot->readers[i] = nullptr;
    #endif
  }
  
}


int check_write_count = 0;
extern "C" void asap_check_write(int *addr, int bytes) {

  if(hclib_ready == true){
    void *pc = __builtin_return_address(0);
    // auto start = std::chrono::system_clock::now();
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));
    // auto slot = shadow_mem->find_slot(ADDR_TO_KEY(addr),false);
    // auto end = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed_seconds = end-start;
    //cout << elapsed_seconds.count() << endl;

    bool is_step = false;
    current_task_and_step.node_in_dpst = hclib_get_current_task_info(&current_task_and_step.task_id,&current_finish_id, &is_step, &is_future);

    if(!is_step){
      return;
    }

    if(slot == NULL){
      MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, false, current_task_and_step, (addr_t)pc, bytes, current_finish_id, is_asap_promise_task);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_write(slot, (addr_t)pc, (addr_t)addr, bytes);
  }

}


extern "C" void asap_check_read(int *addr, int bytes) {

  if(hclib_ready == true){
    void *pc = __builtin_return_address(0);
    auto slot = shadow_mem->find(ADDR_TO_KEY(addr));

    current_task_and_step.node_in_dpst = hclib_get_current_task_info(&current_task_and_step.task_id,&current_finish_id, &is_step, &is_future);

    if(!is_step){
      return;
    }

    if(slot == nullptr){
      MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, true, current_task_and_step, (addr_t)pc, bytes, current_finish_id, is_asap_promise_task);
      slot = shadow_mem->insert(ADDR_TO_KEY(addr), mem_list);
      return;
    }

    handle_read(slot,(addr_t)pc,(addr_t)addr,bytes);
  }
    
}
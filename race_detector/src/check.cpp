#include <stdio.h>
#include <typeinfo>
#include "llvm/DebugInfo/Symbolize/Symbolize.h"
#include "assert.h"
#include "shadow_memory.h"
#include "mem_access.h"
#include "ds_to_hclib.h"

extern tree_node_cpp* current_dpst_node = nullptr;
static ShadowMem<MemAccessList_t> *shadow_mem = new ShadowMem<MemAccessList_t>();
static bool is_asap_promise_task = false;
static unsigned long long check_write_count = 0;
static unsigned long long check_read_count = 0;

static int reachability_count = 0;
static unsigned long long handle_read_count = 0;
static unsigned long long handle_write_count = 0;
// #define STEPSKIP

#ifdef STEPSKIP
  static int current_step_id = -1;
  robin_hood::unordered_set<int*> address_already_visit;
  bool step_first_write_not_happened = true;
  static int a_count = 0;
  int stepid = -1;
  bool in_set;
  int set_max_size = 0;
  int set_count = 0;
  int set_total_size = 0;
#endif

bool write_new_section = false;
extern "C" __attribute__((weak)) void ds_set_write_new_section(bool b){
    write_new_section = b;
}

extern "C" __attribute__((weak)) void set_current_dpst_node(void* node){
    current_dpst_node = (tree_node_cpp*) node;
}

static llvm::symbolize::LLVMSymbolizer::Options opts{};
static llvm::symbolize::LLVMSymbolizer symbolizer{opts};
static string module_name;

extern "C" void print_debug_info(addr_t previous, addr_t current) {
  using namespace llvm;
  Expected<DILineInfo> prev_info = symbolizer.symbolizeCode(
        module_name, {previous, object::SectionedAddress::UndefSection});
  Expected<DILineInfo> curr_info = symbolizer.symbolizeCode(
      module_name, {current, object::SectionedAddress::UndefSection});
  //printf("%lu, %lu\n", previous, current);
  if (prev_info) {
    printf("Previous memory access:\n");
    printf("%s %s %d %d\n", prev_info->FileName.c_str(), prev_info->FunctionName.c_str(), prev_info->Line, prev_info->Column);
  }

  if (curr_info) {
    printf("Current memory access:\n");
    printf("%s %s %d %d\n", curr_info->FileName.c_str(), curr_info->FunctionName.c_str(), curr_info->Line, curr_info->Column);
  }
  
}

extern "C" __attribute__((weak)) void ds_print_check_write_count(){
  printf("check write count: %llu ; handle write count %llu \n", check_write_count, handle_write_count);
}

extern "C" __attribute__((weak)) void ds_print_check_read_count(){
  printf("check read count: %llu ; handle read count %llu \n", check_read_count, handle_read_count);
  printf("ds find count %d \n",ds->get_find_count());
  printf("reachability check %d \n",reachability_count);

  #ifdef STEPSKIP
    printf("duplicate access skipped count %d \n",a_count);
    printf("address already visited max size: %d, size count: %d, average size: %f \n", set_max_size, set_count, (double) set_total_size / (double) set_count);
  #endif

  #ifdef CONSTQUERY
    printf(" Notice: this run has constant reachability check. \n");
  #endif
}

extern "C" void ds_promise_task(bool b){
  is_asap_promise_task = b;
}


//int bool_count = 0;
extern "C" bool precede(access_info previous_step, access_info current_step){
  #ifdef DEBUG
    reachability_count++;
  #endif

  #ifdef CONSTQUERY
    return true;
  #endif

  tree_node_cpp *p_node = (tree_node_cpp*) previous_step.node_in_dpst;
  int p_id = p_node->corresponding_task_id;
  
  tree_node_cpp *c_node = (tree_node_cpp*) current_step.node_in_dpst;
  int c_id = c_node->corresponding_task_id;
  
  return ds->precede(p_node, c_node, p_id, c_id);
}

bool precede(tree_node_cpp* previous_step, tree_node_cpp* current_step){
  #ifdef DEBUG
    reachability_count++;
  #endif

  #ifdef CONSTQUERY
    return true;
  #endif

  int p_id = previous_step->corresponding_task_id;
  
  int c_id = current_step->corresponding_task_id;
  
  return ds->precede(previous_step, current_step, p_id, c_id);
}


extern "C" void handle_read(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  #ifdef DEBUG
    handle_read_count++;
  #endif

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);
  int i = 0;

  for(i = start; i < (start + grains); i++) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == nullptr){
      continue;
    }

    #ifdef DEBUG
      reachability_count ++;
    #endif

    #ifdef CONSTQUERY
      bool race = false;
    #else
      bool race = !ds->precede(writer->step_node,current_dpst_node,writer->step_node->corresponding_task_id,current_dpst_node->corresponding_task_id);
    #endif
    #ifdef REPORT
      if(race){
        printf("we find a write-read race !!!!!!!!!! \n");
        print_debug_info(writer->rip, rip);
      }
    #endif
  } // end of all grains writer

  for(int i = start; i < (start + grains); i++) {

      #ifdef LINK_READER
          MemAccess_t* reader = slot->readers[i];
          if(reader == nullptr){ // 1. we have no previous reader
            MemAccess_t* new_reader = new MemAccess_t(current_dpst_node, rip);
            slot->readers[i] = new_reader;
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
            // bool update = true;
            // int c_id = current_task_and_step.task_id;
            // while(reader != nullptr){
            //   if(reader->task_and_node.task_id == c_id){
            //     reader->task_and_node = current_task_and_step;
            //     update = false;
            //     break;
            //   }
            //   reader = reader->next;
            // }
            // if(update){
              // MemAccess_t* new_reader = new MemAccess_t(current_task_and_step, rip, is_asap_promise_task);
              // MemAccess_t* new_reader = new MemAccess_t(current_task_and_step);
              MemAccess_t* new_reader = new MemAccess_t(current_dpst_node, rip);
              new_reader->next = reader->next;
              // new_reader->prev = reader;
              reader->next = new_reader;

              // slot->readers_tail[i]->next = new_reader;
              // new_reader->prev = slot->readers_tail[i];

              // slot->readers_tail[i] = new_reader;
            // }
          }
      #else
          unsigned int k = slot->reader_index[i] % 3;
          slot->readers[i][k] = new MemAccess_t(current_dpst_node, rip);
          slot->reader_index[i]++;
      #endif
  }
}


extern "C" void handle_write(MemAccessList_t* slot, addr_t rip, addr_t addr, size_t mem_size) {
  #ifdef DEBUG
    handle_write_count++;
  #endif

  const int start = ADDR_TO_MEM_INDEX(addr);
  const int grains = SIZE_TO_NUM_GRAINS(mem_size);
  for (int i=start; i < (start + grains); ++i) {
    MemAccess_t *writer = slot->writers[i];
    if(writer == nullptr) {
      slot->writers[i] = new MemAccess_t(current_dpst_node, rip);
      continue;
    }

    #ifdef DEBUG
      reachability_count++;
    #endif

    #ifdef CONSTQUERY
      bool race = false;
    #else  
      bool race = !ds->precede(writer->step_node,current_dpst_node,writer->step_node->corresponding_task_id,current_dpst_node->corresponding_task_id);
    #endif

    #ifdef REPORT
      if(race){
        printf("we find a write-write race !!!!!!!!!! \n");
        print_debug_info(writer->rip, rip);
      }
    #endif

    // update writer
    writer->step_node = current_dpst_node;
    writer->rip     = rip;
  } // end of checking writers


  for(int i = start; i < (start + grains); i++) {
    #ifdef LINK_READER
        MemAccess_t* reader = slot->readers[i];
        if (reader == nullptr) continue;

        while(reader != nullptr){
          #ifdef CONSTQUERY
            bool race = false;
          #else
            bool race = !ds->precede(reader->step_node,current_dpst_node,reader->step_node->corresponding_task_id,current_dpst_node->corresponding_task_id);
          #endif

          #ifdef DEBUG
            reachability_count ++;
          #endif
          #ifdef REPORT
            if(race){
              printf("we find a read-write race !!!!!!!!!! \n");
              print_debug_info(reader->rip, rip);
            }
          #endif
          
          auto old = reader;
          reader = reader->next;
          delete old;
        }
        
        slot->readers[i] = nullptr;
    #else
        for(int j=0; j<READER_SIZE; j++){
          MemAccess_t* reader = slot->readers[i][j];
          if(reader == nullptr) break;
          bool race = !ds->precede(reader->step_node,current_dpst_node,reader->step_node->corresponding_task_id,current_dpst_node->corresponding_task_id);
          #ifdef REPORT
            if(race){
              printf("we find a read-write race !!!!!!!!!! \n");
              print_debug_info(reader->rip, rip);
            }
          #endif
          delete reader;
          slot->readers[i][j] = nullptr;
        }
    #endif
  }
  
}

extern "C" __attribute__((weak)) void asap_check_write(int *addr, int bytes) {

  if(hclib_ready == true){
    #ifdef DEBUG
      check_write_count++;
    #endif

    current_dpst_node = (tree_node_cpp*) hclib_get_current_step_node();
    if(!(current_dpst_node->this_node_type == STEP)){
      return;
    }

    #ifdef STEPSKIP
      stepid = current_dpst_node->index;
      if(stepid != current_step_id){

        int the_size = address_already_visit.size();
        if(the_size > set_max_size){
          set_max_size = the_size;
        }
        set_total_size += the_size;
        set_count ++;

        current_step_id = stepid;
        address_already_visit.clear();
        address_already_visit.insert(addr);
        step_first_write_not_happened = false;
      }
      else{
        in_set = address_already_visit.count(addr);
        if(in_set && !step_first_write_not_happened){
          a_count ++;
          return;
        }
        else if(in_set && step_first_write_not_happened){
          step_first_write_not_happened = false;
        }
        else{
          address_already_visit.insert(addr);
        }
      }
    #endif

    #ifdef DEBUG
      void *pc = __builtin_return_address(0);
      auto a = ADDR_TO_KEY(addr);
      auto slot = shadow_mem->find(a);

      if(slot == NULL || slot == nullptr){
        MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, false, current_dpst_node, (addr_t)pc, bytes);
        slot = shadow_mem->insert(a, mem_list);
        return;
      }

      if(write_new_section){
        slot->~MemAccessList_t();
      }
      else{
        handle_write(slot, (addr_t) pc, (addr_t)addr, bytes);
      }
      
      return;
    #else
      // void *pc = __builtin_return_address(0);
      auto a = ADDR_TO_KEY(addr);
      auto slot = shadow_mem->find(a);

      if(slot == NULL){
        MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, false, current_dpst_node, bytes);
        slot = shadow_mem->insert(a, mem_list);
        return;
      }

      if(write_new_section){
        slot->~MemAccessList_t();
      }
      else{
        handle_write(slot, (addr_t)nullptr, (addr_t)addr, bytes);
      }
      
      return;
    #endif
  }

}

extern "C" __attribute__((weak)) void asap_check_read(int *addr, int bytes) {
  if(hclib_ready == true){
    #ifdef DEBUG
      check_read_count++;
    #endif

    current_dpst_node = (tree_node_cpp*) hclib_get_current_step_node();
    if(!(current_dpst_node->this_node_type == STEP)){
      return;
    }


    #ifdef STEPSKIP
      stepid = current_dpst_node->index;
      if(stepid != current_step_id){
        int the_size = address_already_visit.size();
        if(the_size > set_max_size){
          set_max_size = the_size;
        }
        set_total_size += the_size;
        set_count ++;

        current_step_id = stepid;
        address_already_visit.clear();
        address_already_visit.insert(addr);
        step_first_write_not_happened = true;
      }
      else{
        if(address_already_visit.count(addr) > 0){
          a_count ++;
          return;
        }
        else{
          address_already_visit.insert(addr);
        }
      }
    #endif

    #ifdef DEBUG
        void *pc = __builtin_return_address(0);
        auto a = ADDR_TO_KEY(addr);
        auto slot = shadow_mem->find(a);

        if(slot == nullptr){
          MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, true, current_dpst_node, (addr_t)pc, bytes);
          slot = shadow_mem->insert(a, mem_list);
          return;
        }

        handle_read(slot,(addr_t)pc,(addr_t)addr,bytes);
        return;
    #else
        // void *pc = __builtin_return_address(0);
        auto a = ADDR_TO_KEY(addr);
        auto slot = shadow_mem->find(a);

        if(slot == nullptr){
          MemAccessList_t *mem_list  = new MemAccessList_t((addr_t)addr, true, current_dpst_node, bytes);
          slot = shadow_mem->insert(a, mem_list);
          return;
        }

        handle_read(slot,(addr_t)nullptr,(addr_t)addr,bytes);
        return;
    #endif
  }
    
}

extern "C" __attribute__((weak)) void asap_start(int argc, char *argv[]) {
  printf("DRDP determinacy race detector start\n");
  printf("Program to be checked: %s\n", argv[0]);
  printf("Input parameter:");
  for (int i = 1; i < argc; i++) {
    printf(" %s", argv[i]);
  }
  printf("\n");
  module_name = argv[0];
}

extern "C" __attribute__((weak)) void asap_alloc(int *addr, int bytes) {
  shadow_mem->clear_shadow_mem((addr_t)addr, bytes);
}
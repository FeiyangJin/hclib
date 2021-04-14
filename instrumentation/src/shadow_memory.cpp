#include "shadow_memory.h"
#include <stdio.h>

void shadow_memory::printhello(){
    printf("hello from shadow memory \n");
}

void shadow_memory::updateWriter(int* location, access_info new_writer){
    this->location_last_writer[location] = new_writer;
}

void shadow_memory::removeReader(int* location, int old_reader_id){
    this->location_readers[location].erase(old_reader_id);
}
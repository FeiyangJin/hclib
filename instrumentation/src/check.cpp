#include <stdio.h>
#include "shadow_memory.h"

extern "C" void asap_check_read(int *addr, int bytes) {
    printf("\nasap_check_read: %p, bytes: %d\n", addr, bytes);
}

extern "C" void asap_check_write(int *addr, int bytes) {
    printf("\nasap_check_write: %p, bytes: %d\n", addr, bytes);
}

extern "C" __attribute__((weak)) void ds_print(){
    printf("hello from check.cpp \n");
}
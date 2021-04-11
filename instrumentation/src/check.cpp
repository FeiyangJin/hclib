#include <stdio.h>

extern "C" void asap_check_read(int *addr) {
    printf("\nasap_check_read: %p\n", addr);
}

extern "C" void asap_check_write(int *addr) {
    printf("\nasap_check_write: %p\n", addr);
}

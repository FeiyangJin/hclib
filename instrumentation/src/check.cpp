#include <stdio.h>

extern "C" void asap_check_read(int *addr, int bytes) {
    printf("\nasap_check_read: %p, bytes: %d\n", addr, bytes);
}

extern "C" void asap_check_write(int *addr, int bytes) {
    printf("\nasap_check_write: %p, bytes: %d\n", addr, bytes);
}

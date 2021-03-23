#include <stdio.h>
#include "aaa_c_connector.h"

int main() {
    addSet(1);
    addSet(2);
    addSet(3);
    
    merge(2,3);
    printAll();

    printf("\n");

    merge(1,2);
    printAll();
    return 0;
}
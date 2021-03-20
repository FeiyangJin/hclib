#include <stdio.h>
#include "aaa_c_connector.h"

int main() {
    addSet(1);
    addSet(2);
    addSet(3);
    printf("the element 2 is in %d \n",findSet(2));
    //int elements[3] = {1,2,3};
    //printElementSets(elements);
    return 0;
}
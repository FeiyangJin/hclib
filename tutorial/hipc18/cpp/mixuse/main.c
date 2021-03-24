#include <stdio.h>
#include <assert.h>
#include "aaa_c_connector.h"

int main() {
    for(int i=0; i<10; i++){
        addSet(i);
    }

    //test initialization
    for (int i = 0; i < 10; i++)
    {
        assert(findSet(i) == i);
        assert(getlsa(i) == -1);
        assert(ntcounts(i) == 0);
    }
    
    // test merge
    merge(2,3);
    assert(findSet(3) == 2);

    merge(7,2);
    merge(5,7);
    assert(findSet(3) == 5);
    assert(findSet(2) == 5);
    assert(findSet(7) == 5);
    assert(findSet(5) == 5);

    // test non tree joins
    addnt(1,2);
    addnt(1,3);
    addnt(1,5);
    assert(ntcounts(1) == 3);

    // test lsa
    setlsa(4,9);
    assert(getlsa(4) == 9);

    // test merge
    merge(4,1);
    assert(ntcounts(4) == 3);

    //printAll();
    printdsbyset();
    return 0;
}
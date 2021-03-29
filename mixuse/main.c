#include <stdio.h>
#include <assert.h>
#include "aaa_c_connector.h"

int main() {
    for(int i=0; i<10; i++){
        ds_addSet(i);
    }

    //test initialization
    for (int i = 0; i < 10; i++)
    {
        assert(ds_findSet(i) == i);
        assert(ds_getlsa(i) == -1);
        assert(ds_ntcounts(i) == 0);
    }
    
    // test merge
    ds_merge(2,3);
    assert(ds_findSet(3) == 2);

    ds_merge(7,2);
    ds_merge(5,7);
    assert(ds_findSet(3) == 5);
    assert(ds_findSet(2) == 5);
    assert(ds_findSet(7) == 5);
    assert(ds_findSet(5) == 5);

    // test non tree joins
    ds_addnt(1,2);
    ds_addnt(1,3);
    ds_addnt(1,5);
    assert(ds_ntcounts(1) == 3);

    // test lsa
    ds_setlsa(4,9);
    assert(ds_getlsa(4) == 9);

    // test merge
    ds_merge(4,1);
    assert(ds_ntcounts(4) == 3);

    //printAll();
    ds_printdsbyset();

    // test task
    ds_addtask(1,0,NULL,NULL,2);
    assert(ds_parentid(1) == 0);
    assert(ds_get_dpst_node(1) == NULL);
    assert(ds_taskState(1) == 2);
    
    return 0;
}
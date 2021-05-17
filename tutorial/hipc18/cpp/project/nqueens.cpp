#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <alloca.h>
#include <cstdlib>
#include "nqueens-app-desc.h"
#include "hclib_cpp.h"


/* Checking information */

static int solutions[] = {
        1,
        0,
        0,
        2,
        10, /* 5 */
        4,
        40,
        92,
        352,
        724, /* 10 */
        2680,
        14200,
        73712,
        365596,
};
#define MAX_SOLUTIONS sizeof(solutions)/sizeof(int)

int total_count;

/*
 * <a> contains array of <n> queen positions.  Returns 1
 * if none of the queens conflict, and returns 0 otherwise.
 */
int ok(int n, char *a)
{
     int i, j;
     char p, q;

     for (i = 0; i < n; i++) {
	  p = a[i];

	  for (j = i + 1; j < n; j++) {
	       q = a[j];
	       if (q == p || q == p - (j - i) || q == p + (j - i))
		    return 0;
	  }
     }
     return 1;
}

int nqueens_fj(int n, int j, char *a){
    ds_hclib_ready(true);
    int solution = 0;
    int i;
    if(n == j){
        return 1;
    }

    ds_hclib_ready(false);
    std::vector<hclib::promise_t<int>*> pv;

    for (i = 0; i < n; i++) {
        a[j] = (char) i;
        if (ok(j + 1, a)) {
            ds_hclib_ready(false);
            hclib::async([&](){
                ds_hclib_ready(true);
                int result = nqueens_fj(n, j + 1, a);
                // solution += result;
                hclib::promise_t<int>* p = new hclib::promise_t<int>();
                p->end_put(result);
                pv.push_back(p);
            });
        }
    }

    ds_hclib_ready(true);
    for(auto i = pv.begin(); i != pv.end(); i++){
        solution += (*i)->get_future()->wait();
    }

    return solution;
}

void nqueens (int n, int j, char *a, int *solutions)
{
	int i,res;

	if (n == j) {
		/* good solution, count it */
		*solutions = 1;
		return;
	}

	*solutions = 0;

    /* try each possible position for queen <j> */
    for (i = 0; i < n; i++) {
        a[j] = (char) i;
        if (ok(j + 1, a)) {
            nqueens(n, j + 1, a,&res);
            *solutions += res;
        }
    }

}

void find_queens (int size)
{
	char *a;

	total_count=0;
	a = (char *)alloca(size * sizeof(char));
	printf("Computing N-Queens algorithm (n=%d) \n", size);
	// nqueens(size, 0, a, &total_count);
    total_count = nqueens_fj(size,0,a);
    printf("result is %d, completed ! \n",total_count);
}

int verify_queens (int size)
{
	// if ( size > MAX_SOLUTIONS ) return BOTS_RESULT_NA;
	// if ( total_count == solutions[size-1]) return BOTS_RESULT_SUCCESSFUL;
	// return BOTS_RESULT_UNSUCCESSFUL;
    return 0;
}

int main(int argc, char** argv) {
    printf("hello nqueens \n");
    if(argc < 2){
        printf("usage: ./nqueens.exe number \n");
        return -1;
    }

    char const *deps[] = { "system" }; 
    hclib::launch(deps, 1, [&]() {

        long start = hclib_current_time_ms();

        find_queens(atoi(argv[1]));

        long end = hclib_current_time_ms();
        double dur = ((double)(end-start))/1000;
        printf("nqueens Time = %f \n",dur);
        printf("DPST height is: %d \n", get_dpst_height());
        printf("cache size is %d \n",ds_get_cache_size());
        printf("number of task is %d \n",get_task_id_unique());
        printf("number of nt join %d \n", get_nt_count());
        printf("number of tree joins %d \n", ds_get_tree_join_count());
    });
    return 0;
}
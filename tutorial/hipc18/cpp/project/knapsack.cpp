#include "app-desc.h"
#include <stdlib.h>
#include <stdio.h>
#include <climits>
#include "hclib_cpp.h"

int best_so_far;

int compare(struct item *a, struct item *b)
{
     double c = ((double) a->value / a->weight) -
     ((double) b->value / b->weight);

     if (c > 0) return -1;
     if (c < 0) return 1;
     return 0;
}

int read_input(const char *filename, struct item *items, int *capacity, int *n)
{
     int i;
     FILE *f;

     if (filename == NULL) filename = "\0";
     f = fopen(filename, "r");
     if (f == NULL) {
	  fprintf(stderr, "open_input(\"%s\") failed\n", filename);
	  return -1;
     }
     /* format of the input: #items capacity value1 weight1 ... */
     fscanf(f, "%d", n);
     fscanf(f, "%d", capacity);

     for (i = 0; i < *n; ++i)
	  fscanf(f, "%d %d", &items[i].value, &items[i].weight);

     fclose(f);

     /* sort the items on decreasing order of value/weight */
     /* cilk2c is fascist in dealing with pointers, whence the ugly cast */
     qsort(items, *n, sizeof(struct item), (int (*)(const void *, const void *)) compare);

     return 0;
}

void knapsack_par(struct item *e, int c, int n, int v, int *sol, int l)
{
     int with, without, best;
     double ub;

     /* base case: full knapsack or no items */
     if (c < 0)
     {
         *sol = INT_MIN;
         return;
     }

     /* feasible solution, with value v */
     if (n == 0 || c == 0)
     {
         *sol = v;
         return;
     }

     ub = (double) v + c * e->value / e->weight;

     if (ub < best_so_far) {
	  /* prune ! */
          *sol = INT_MIN;
          return;
     }

    // hclib::finish([&](){
    //     // int a = 0;       
    // });
    hclib::async([&](){

    });
        
     knapsack_par(e + 1, c, n - 1, v, &without,l+1);
     knapsack_par(e + 1, c - e->weight, n - 1, v + e->value, &with,l+1);

     best = with > without ? with : without;


     if (best > best_so_far) best_so_far = best;

     *sol = best;
}

/* 
 * return the optimal solution for n items (first is e) and
 * capacity c. Value so far is v.
 */
void knapsack(struct item *e, int c, int n, int v, int *sol)
{
     int with, without, best;
     double ub;

     /* base case: full knapsack or no items */
     if (c < 0)
     {
         *sol = INT_MIN;
         return;
     }

     /* feasible solution, with value v */
     if (n == 0 || c == 0)
     {
         *sol = v;
         return;
     }

     ub = (double) v + c * e->value / e->weight;

     if (ub < best_so_far) {
	  /* prune ! */
          *sol = INT_MIN;
          return;
     }
     /* 
      * compute the best solution without the current item in the knapsack 
      */
        knapsack(e + 1, c, n - 1, v, &without);
        
        /* compute the best solution with the current item in the knapsack */
        knapsack(e + 1, c - e->weight, n - 1, v + e->value, &with);

     best = with > without ? with : without;

     /* 
      * notice the race condition here. The program is still
      * correct, in the sense that the best solution so far
      * is at least best_so_far. Moreover best_so_far gets updated
      * when returning, so eventually it should get the right
      * value. The program is highly non-deterministic.
      */
     if (best > best_so_far) best_so_far = best;

     *sol = best;
}


int main(int argc, char** argv) {
    printf("hello world \n");
    if(argc < 2){
        printf("usage: ./knapsack.exe input_file.input \n");
        return 0;
    }
    printf("%s \n", argv[1]);

    struct item items[MAX_ITEMS];
    int n, capacity;
    int sol = 0;

    read_input(argv[1],items,&capacity,&n);

    char const *deps[] = { "system" }; 
    hclib::launch(deps, 1, [&]() {

        long start = hclib_current_time_ms();
        // knapsack_main(items,capacity,n,&sol);
        knapsack_main_par(items,capacity,n,&sol);

        long end = hclib_current_time_ms();
        double dur = ((double)(end-start))/1000;
        printf("Knapsack Time = %f \n",dur);
        printf("DPST height is: %d \n", get_dpst_height());
        printf("cache size is %d \n",ds_get_cache_size());
        printf("number of task is %d \n",get_task_id_unique());
        printf("number of nt join %d \n", get_nt_count());
        printf("number of tree joins %d \n", ds_get_tree_join_count());
    });
    
    return 0;
}

void knapsack_main_par(struct item *e, int c, int n, int *sol)
{
    best_so_far = INT_MIN;

    knapsack_par(e, c, n, 0, sol, 0);

    printf("Best value for parallel execution is %d\n\n", *sol);
}

void knapsack_main(struct item *e, int c, int n, int *sol)
{
    best_so_far = INT_MIN;
    knapsack(e, c, n, 0, sol);
    printf("Best value is %d\n\n", *sol);
    
}
#include "hclib_cpp.h"
#include <stdio.h>
#include "app-desc.h"
#include <chrono>
#include <ctime> 
#include <iostream>

static unsigned long rand_nxt = 0;
int bots_arg_size,bots_app_cutoff_value_1,bots_app_cutoff_value_2,bots_app_cutoff_value;

static inline unsigned long my_rand(void)
{
     rand_nxt = rand_nxt * 1103515245 + 12345;
     return rand_nxt;
}

static inline void my_srand(unsigned long seed)
{
     rand_nxt = seed;
}

static inline ELM med3(ELM a, ELM b, ELM c)
{
     if (a < b) {
	  if (b < c) {
	       return b;
	  } else {
	       if (a < c)
		    return c;
	       else
		    return a;
	  }
     } else {
	  if (b > c) {
	       return b;
	  } else {
	       if (a > c)
		    return c;
	       else
		    return a;
	  }
     }
}

static inline ELM choose_pivot(ELM *low, ELM *high)
{
     return med3(*low, *high, low[(high - low) / 2]);
}

static ELM *seqpart(ELM *low, ELM *high)
{
     ELM pivot;
     ELM h, l;
     ELM *curr_low = low;
     ELM *curr_high = high;

     pivot = choose_pivot(low, high);

     while (1) {
	  while ((h = *curr_high) > pivot)
	       curr_high--;

	  while ((l = *curr_low) < pivot)
	       curr_low++;

	  if (curr_low >= curr_high)
	       break;

	  *curr_high-- = l;
	  *curr_low++ = h;
     }

     /*
      * I don't know if this is really necessary.
      * The problem is that the pivot is not always the
      * first element, and the partition may be trivial.
      * However, if the partition is trivial, then
      * *high is the largest element, whence the following
      * code.
      */
     if (curr_high < high)
	  return curr_high;
     else
	  return curr_high - 1;
}

#define swap(a, b) \
{ \
  ELM tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}

static void insertion_sort(ELM *low, ELM *high)
{
     ELM *p, *q;
     ELM a, b;

     for (q = low + 1; q <= high; ++q) {
	  a = q[0];
      ds_hclib_ready(false);
	  for (p = q - 1; p >= low && (b = p[0]) > a; p--)
	       p[1] = b;
      ds_hclib_ready(true);
	  p[1] = a;
     }
}

/*
 * tail-recursive quicksort, almost unrecognizable :-)
 */
void seqquick(ELM *low, ELM *high)
{
     ELM *p;

     while (high - low >= bots_app_cutoff_value_2) {
	  p = seqpart(low, high);
	  seqquick(low, p);
	  low = p + 1;
     }

     insertion_sort(low, high);
}

void seqmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest)
{
     // ds_hclib_ready(false);
     ELM a1, a2;

     if (low1 < high1 && low2 < high2) {
          a1 = *low1;
          a2 = *low2;
          for (;;) {
               if (a1 < a2){
               *lowdest++ = a1;
               a1 = *++low1;
               if (low1 >= high1)
                    break;
               }
               else{
               *lowdest++ = a2;
               a2 = *++low2;
               if (low2 >= high2)
                    break;
               }
          }
     }

     if (low1 <= high1 && low2 <= high2) {
          a1 = *low1;
          a2 = *low2;
          for (;;) {
               if (a1 < a2) {
               *lowdest++ = a1;
               ++low1;
               if (low1 > high1)
                    break;
               a1 = *low1;
               } else {
               *lowdest++ = a2;
               ++low2;
               if (low2 > high2)
                    break;
               a2 = *low2;
               }
          }
     }

     if (low1 > high1) {
          memcpy(lowdest, low2, sizeof(ELM) * (high2 - low2 + 1));
     } else {
          memcpy(lowdest, low1, sizeof(ELM) * (high1 - low1 + 1));
     }
}

#define swap_indices(a, b) \
{ \
  ELM *tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}

ELM *binsplit(ELM val, ELM *low, ELM *high)
{
     /*
      * returns index which contains greatest element <= val.  If val is
      * less than all elements, returns low-1
      */
     ELM *mid;

     while (low != high) {
	  mid = low + ((high - low + 1) >> 1);
	  if (val <= *mid)
	       high = mid - 1;
	  else
	       low = mid;
     }

     if (*low > val)
	  return low - 1;
     else
	  return low;
}

void cilkmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest)
{
    // ds_hclib_ready(false);
     ELM *split1, *split2;	/*
				 * where each of the ranges are broken for 
				 * recursive merge 
				 */
     long int lowsize;		/*
				 * total size of lower halves of two
				 * ranges - 2 
				 */

     if (high2 - low2 > high1 - low1) {
	  swap_indices(low1, low2);
	  swap_indices(high1, high2);
     }
     if (high2 < low2) {
	  /* smaller range is empty */
	  memcpy(lowdest, low1, sizeof(ELM) * (high1 - low1));
	  return;
     }
     if (high2 - low2 < bots_app_cutoff_value ) {
	  seqmerge(low1, high1, low2, high2, lowdest);
      ds_hclib_ready(true);
	  return;
     }


     split1 = ((high1 - low1 + 1) / 2) + low1;
     split2 = binsplit(*split1, low2, high2);
     lowsize = split1 - low1 + split2 - low2;


     *(lowdest + lowsize + 1) = *split1;

     ds_hclib_ready(false);
     hclib::finish([&](){
         hclib::async([&](){
             ds_hclib_ready(true);
             cilkmerge(low1, split1 - 1, low2, split2, lowdest);
         });
        
        cilkmerge(split1 + 1, high1, split2 + 1, high2, lowdest+lowsize+2);
     });


     return;
}

void cilksort(ELM *low, ELM *tmp, long size)
{

     long quarter = size / 4;
     ELM *A, *B, *C, *D, *tmpA, *tmpB, *tmpC, *tmpD;

     if (size < bots_app_cutoff_value_1) {
	  /* quicksort when less than 1024 elements */
	  seqquick(low, low + size - 1);
	  return;
     }
     A = low;
     tmpA = tmp;
     B = A + quarter;
     tmpB = tmpA + quarter;
     C = B + quarter;
     tmpC = tmpB + quarter;
     D = C + quarter;
     tmpD = tmpC + quarter;

    ds_hclib_ready(false);
    hclib::finish([&](){
        hclib::async([&](){
            ds_hclib_ready(true);
            cilksort(A, tmpA, quarter);
        });

        ds_hclib_ready(false);
        hclib::async([&](){
            ds_hclib_ready(true);
            cilksort(B, tmpB, quarter);
        });

        ds_hclib_ready(false);
        hclib::async([&](){
            ds_hclib_ready(true);
            cilksort(C, tmpC, quarter);
        });
     
        cilksort(D, tmpD, size - 3 * quarter);
    });

    ds_hclib_ready(false);
    hclib::finish([&](){
        hclib::async([&](){
            ds_hclib_ready(true);
            cilkmerge(A, A + quarter - 1, B, B + quarter - 1, tmpA);
        });

        cilkmerge(C, C + quarter - 1, D, low + size - 1, tmpC);
    });
     
     cilkmerge(tmpA, tmpC - 1, tmpC, tmpA + size - 1, A);
}

ELM *array, *tmp;

void scramble_array( void )
{
     unsigned long i;
     unsigned long j;

     for (i = 0; i < bots_arg_size; ++i) {
	  j = my_rand();
	  j = j % bots_arg_size;
	  swap(array[i], array[j]);
     }
}

void fill_array( void )
{
     unsigned long i;

     my_srand(1);

     /* first, fill with integers 1..size */
     for (i = 0; i < bots_arg_size; ++i) {
	  array[i] = i;
     }
}

void sort ( void )
{
     printf("Computing multisort algorithm (n=%d) ", bots_arg_size);
     cilksort(array, tmp, bots_arg_size);
     printf(" completed!\n");
}

void sort_verify ()
{
     int i, success = 1;
     for (i = 0; i < bots_arg_size; ++i)
	    if (array[i] != i){
            success = 0;
            break;
        }

    if(success){
        printf("sort is successful \n");
    }
    else{
        printf("find a sort error \n");
    }
}

int main(int argc, char* argv[]){
     printf("cilksort from bots \n");
     bots_arg_size = 1000000;
     bots_app_cutoff_value = 2*1024;
     bots_app_cutoff_value_1 = 2*1024;
     bots_app_cutoff_value_2 = 20;
     array = (ELM *) malloc(bots_arg_size * sizeof(ELM));
     tmp = (ELM *) malloc(bots_arg_size * sizeof(ELM));
     fill_array();
     scramble_array();

     char const *deps[] = { "system" };
     hclib::launch(deps, 1, [&]() {
          long start = hclib_current_time_ms();

          ds_hclib_ready(true);
          sort();
          ds_hclib_ready(false);

          long end = hclib_current_time_ms();
          double dur = ((double)(end-start))/1000;

          printf("sort time : %.3f \n",dur);
     });

     sort_verify();
     return 0;
}
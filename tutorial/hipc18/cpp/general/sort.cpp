#include "hclib_cpp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

typedef long ELM;
/* MERGESIZE must be >= 2 */
#define KILO 1024
#define MERGESIZE KILO
#define QUICKSIZE KILO
#define INSERTIONSIZE 20


#define swap(a, b) \
{ \
  ELM tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}


#define swap_indices(a, b) \
{ \
  ELM *tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}


static unsigned long rand_nxt = 0;

static inline unsigned long my_rand(void) {
  rand_nxt = rand_nxt * 1103515245 + 12345;
  return rand_nxt;
}


static inline void my_srand(unsigned long seed) {
	rand_nxt = seed;
}


static inline ELM med3(ELM a, ELM b, ELM c) {

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


static inline ELM choose_pivot(ELM *low, ELM *high) {
	return med3(*low, *high, low[(high - low) / 2]);
}


static ELM *seqpart(ELM *low, ELM *high) {

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

  if (curr_high < high)
    return curr_high;
  else
    return curr_high - 1;
}


static void insertion_sort(ELM *low, ELM *high) {

  ELM *p, *q;
  ELM a, b;

  for (q = low + 1; q <= high; ++q) {
    a = q[0];
    for (p = q - 1; p >= low && (b = p[0]) > a; p--)
      p[1] = b;
    p[1] = a;
  }
}


void seqquick(ELM *low, ELM *high) {

  ELM *p;

  while (high - low >= INSERTIONSIZE) {
    p = seqpart(low, high);
    seqquick(low, p);
    low = p + 1;
  }

  insertion_sort(low, high);
}


void seqmerge(ELM *low1, ELM *high1, 
  ELM *low2, ELM *high2, ELM *lowdest) {

  ELM a1, a2;

  if (low1 < high1 && low2 < high2) {
    a1 = *low1;
    a2 = *low2;
    for (;;) {
      if (a1 < a2) {
        *lowdest++ = a1;
        a1 = *++low1;
        if (low1 >= high1)
          break;
      } else {
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


ELM *binsplit(ELM val, ELM *low, ELM *high) {

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


void cilkmerge(ELM *low1, ELM *high1, 
    ELM *low2, ELM *high2, ELM *lowdest) {

  /*
   * Cilkmerge: Merges range [low1, high1] with range [low2, high2] 
   * into the range [lowdest, ...]  
   */

  /*
   * We want to take the middle element (indexed by split1) from the
   * larger of the two arrays.  The following code assumes that split1
   * is taken from range [low1, high1].  So if [low1, high1] is
   * actually the smaller range, we should swap it with [low2, high2] 
   */

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

  if (high1 < low1) {
    /* smaller range is empty */
    memcpy(lowdest, low2, sizeof(ELM) * (high2 - low2));
    return;
  }

  if (high2 - low2 < MERGESIZE) {
    seqmerge(low1, high1, low2, high2, lowdest);
    return;
  }
  /*
   * Basic approach: Find the middle element of one range (indexed by
   * split1). Find where this element would fit in the other range
   * (indexed by split 2). Then merge the two lower halves and the two
   * upper halves. 
   */

  split1 = ((high1 - low1 + 1) / 2) + low1;
  split2 = binsplit(*split1, low2, high2);
  lowsize = split1 - low1 + split2 - low2;

  /* 
   * directly put the splitting element into
   * the appropriate location
   */
  *(lowdest + lowsize + 1) = *split1;


 hclib::future_t<void>* f1 = hclib::async_future([=]() { 
  	cilkmerge(low1, split1 - 1, low2, split2, lowdest);
  	return;
 });

 cilkmerge(split1 + 1, high1, split2 + 1, high2, lowdest + lowsize + 2);
 f1->wait();

  /*cilk_spawn*/ //cilkmerge(low1, split1 - 1, low2, split2, lowdest);
  //cilkmerge(split1 + 1, high1, split2 + 1, high2, lowdest + lowsize + 2);
  //cilk_sync;

  return;
}


void cilksort(ELM *low, ELM *tmp, long size) {

  /*
   * divide the input in four parts of the same size (A, B, C, D)
   * Then:
   *   1) recursively sort A, B, C, and D (in parallel)
   *   2) merge A and B into tmp1, and C and D into tmp2 (in parallel)
   *   3) merbe tmp1 and tmp2 into the original array
   */

  long quarter = size / 4;
  ELM *A, *B, *C, *D, *tmpA, *tmpB, *tmpC, *tmpD;

  if (size < QUICKSIZE) {
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


   hclib::future_t<void>* f1 = hclib::async_future([=]() { 
      cilksort(A, tmpA, quarter);
      return;
    });


   hclib::future_t<void>* f2 = hclib::async_future([=]() { 
      cilksort(B, tmpB, quarter);
      return;
    });


    hclib::future_t<void>* f3 = hclib::async_future([=]() { 
      cilksort(C, tmpC, quarter);
      return;
    });


    cilksort(D, tmpD, size - 3 * quarter);
    f1->wait();
    f2->wait();
    f3->wait();

  // /*cilk_spawn*/ cilksort(A, tmpA, quarter);
  // /*cilk_spawn*/ cilksort(B, tmpB, quarter);
  // cilk_spawn cilksort(C, tmpC, quarter);
  // cilksort(D, tmpD, size - 3 * quarter);
  // /*cilk_sync;*/


    hclib::future_t<void>* f4 = hclib::async_future([=]() { 
      cilkmerge(A, A + quarter - 1, B, B + quarter - 1, tmpA);
      return;
    });

    cilkmerge(C, C + quarter - 1, D, low + size - 1, tmpC);
    f4->wait();
  /*cilk_spawn*/ //cilkmerge(A, A + quarter - 1, B, B + quarter - 1, tmpA);
  //cilkmerge(C, C + quarter - 1, D, low + size - 1, tmpC);
  //cilk_sync;

  cilkmerge(tmpA, tmpC - 1, tmpC, tmpA + size - 1, A);

  return;
}


void scramble_array(ELM *arr, unsigned long size) {

  unsigned long i;
  unsigned long j;

  for (i = 0; i < size; ++i) {
    j = my_rand();
    j = j % size;
    swap(arr[i], arr[j]);
  }
}


void fill_array(ELM *arr, unsigned long size) {

  unsigned long i;

  my_srand(1);
  /* first, fill with integers 1..size */
  for (i = 0; i < size; ++i) {
    arr[i] = i;
  }

  /* then, scramble randomly */
  scramble_array(arr, size);
}


void zero(ELM *arr, unsigned long size){
	unsigned long i;
	for (i = 0; i < size; ++i) {
    	arr[i] = 0;
  	}
}


void print_array(ELM *arr, unsigned long size){
	unsigned long i;
	int newline = 1;
	for(i=0; i<size; ++i){
		printf("%d  ", (int)arr[i]);
		newline += 1;
		if(newline % 10 == 0){
			printf("\n");
			newline = 1;
		}
	}
	printf("\n");
}


void check_result(ELM *sorted, unsigned long size){
	unsigned long i;
	for(i=0; i<size; ++i){
		if(sorted[i] != i){
			printf("find a sorted error at index %lu \n",i);
			break;
		}
	}
}


int main(int argc, char* argv[]){
	long size;
	ELM *array, *tmp;
	long i;

	size = 10000000;
	array = (ELM *) malloc(size * sizeof(ELM));
	tmp = (ELM *) malloc(size * sizeof(ELM));

	fill_array(array,size);
	

	char const *deps[] = { "system" };
  	hclib::launch(deps, 1, [&]() {
  		long start = hclib_current_time_ms();

  		//parallel sort
		cilksort(array,tmp,size);

		long end = hclib_current_time_ms();
		double dur = ((double)(end-start))/1000;

		check_result(array,size);
		printf("sort time in parallel: %.3f for array of size %ld \n",dur,size);


		//seq sort
		fill_array(array,size);
		zero(tmp,size);

		start = hclib_current_time_ms();

		seqquick(array, array + size - 1);

		end = hclib_current_time_ms();
		dur = ((double)(end-start))/1000;

		check_result(array,size);
		printf("sort time in sequential: %.3f for array of size %ld \n",dur,size);

		free(array);
		free(tmp);
	});

	return 0;

}
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


void cilkmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest) {
  ELM *split1, *split2;
  long int lowsize;

  ds_hclib_ready(true);
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

  split1 = ((high1 - low1 + 1) / 2) + low1;
  split2 = binsplit(*split1, low2, high2);
  lowsize = split1 - low1 + split2 - low2;

  *(lowdest + lowsize + 1) = *split1;

  ds_hclib_ready(false);  
  hclib::promise_t<void> *p1 = new hclib::promise_t<void>();
  hclib::async([&](){
    cilkmerge(low1, split1 - 1, low2, split2, lowdest);
    ds_hclib_ready(true);
    p1->end_put();
    ds_hclib_ready(false); 
  });

  cilkmerge(split1 + 1, high1, split2 + 1, high2, lowdest + lowsize + 2);
  p1->get_future()->wait();

  return;
}


void cilksort(ELM *low, ELM *tmp, long size) {
  ds_hclib_ready(true);

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

  ds_hclib_ready(false);
  hclib::promise_t<void> *p1 = new hclib::promise_t<void>();
  hclib::promise_t<void> *p2 = new hclib::promise_t<void>();
  hclib::promise_t<void> *p3 = new hclib::promise_t<void>();

  ds_hclib_ready(false);
  hclib::async([&](){
    cilksort(A, tmpA, quarter);
    ds_hclib_ready(true);
    p1->end_put();
    ds_hclib_ready(false);
  });

  hclib::async([&](){
    cilksort(B, tmpB, quarter);
    ds_hclib_ready(true);
    p2->end_put();
    ds_hclib_ready(false);
  });

  hclib::async([&](){
    cilksort(C, tmpC, quarter);
    ds_hclib_ready(true);
    p3->end_put();
    ds_hclib_ready(false);
  });

  cilksort(D, tmpD, size - 3 * quarter);

  p1->get_future()->wait();
  p2->get_future()->wait();
  p3->get_future()->wait();

  
  ds_hclib_ready(false);
  hclib::promise_t<void> *p4 = new hclib::promise_t<void>();
  hclib::async([&](){
    cilkmerge(A, A + quarter - 1, B, B + quarter - 1, tmpA);
    p4->end_put();
  });

  ds_hclib_ready(false);
  cilkmerge(C, C + quarter - 1, D, low + size - 1, tmpC);
  p4->get_future()->wait();

  cilkmerge(tmpA, tmpC - 1, tmpC, tmpA + size - 1, A);
  ds_hclib_ready(false);

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
  return;
}


void check_result(ELM *sorted, unsigned long size){
	unsigned long i;
	for(i=0; i<size; ++i){
		if(sorted[i] != i){
			printf("find a sorted error at index %lu \n",i);
			break;
		}
	}
  printf("result is correct from checking function \n");
  return;
}


int main(int argc, char* argv[]){
	long size;
	ELM *array, *tmp;
	long i;

  size = argc>1?atoi(argv[1]) : 10000000; // default n value is 10000000
	printf("sort array of size %ld \n", size);

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

		printf("sort time in parallel: %.3f for array of size %ld \n",dur,size);
    printf("DPST height is: %d \n", get_dpst_height());
		//seq sort
		// fill_array(array,size);
		// zero(tmp,size);

		// start = hclib_current_time_ms();

		// seqquick(array, array + size - 1);

		// end = hclib_current_time_ms();
		// dur = ((double)(end-start))/1000;

		// printf("sort time in sequential: %.3f for array of size %ld \n",dur,size);

    ds_hclib_ready(false);
    printf("cache size is %d \n",ds_get_cache_size());
    printf("number of task is %d \n",get_task_id_unique());
    printf("number of nt join %d \n", get_nt_count());
    printf("number of tree joins %d \n", ds_get_tree_join_count());
	});

  check_result(array,size);
  free(array);
	free(tmp);

	return 0;

}
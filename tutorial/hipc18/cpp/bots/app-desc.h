#define BOTS_APP_NAME "Sort"
#define BOTS_APP_PARAMETERS_DESC "N=%d:Q=%d:I=%d:M=%d"
#define BOTS_APP_PARAMETERS_LIST ,bots_arg_size,bots_app_cutoff_value_1,bots_app_cutoff_value_2,bots_app_cutoff_value

#define BOTS_APP_USES_ARG_SIZE
#define BOTS_APP_DEF_ARG_SIZE (32*1024*1024)
#define BOTS_APP_DESC_ARG_SIZE "Array size"

#define BOTS_APP_USES_ARG_CUTOFF
#define BOTS_APP_DEF_ARG_CUTOFF (2*1024)
#define BOTS_APP_DESC_ARG_CUTOFF "Sequential Merge cutoff value"

#define BOTS_APP_USES_ARG_CUTOFF_1
#define BOTS_APP_DEF_ARG_CUTOFF_1 (2*1024)
#define BOTS_APP_DESC_ARG_CUTOFF_1 "Sequential Quicksort cutoff value"

#define BOTS_APP_USES_ARG_CUTOFF_2
#define BOTS_APP_DEF_ARG_CUTOFF_2 (20)
#define BOTS_APP_DESC_ARG_CUTOFF_2 "Sequential Insertion cutoff value"

typedef long ELM;

void seqquick(ELM *low, ELM *high);
void seqmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest);
ELM *binsplit(ELM val, ELM *low, ELM *high);
void cilkmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest);
void cilksort(ELM *low, ELM *tmp, long size);
void scramble_array( void );
void fill_array( void );
void sort ( void );
void sort_par ( void );
void sort_init ( void );
void  sort_verify ( void );

#define BOTS_APP_INIT sort_init()

#define KERNEL_INIT
#define KERNEL_CALL sort()
#define KERNEL_CHECK sort_verify()
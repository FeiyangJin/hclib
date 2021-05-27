#include "serial-app.h"

#define BOTS_APP_NAME "N Queens"
#define BOTS_APP_PARAMETERS_DESC "N=%d"
#define BOTS_APP_PARAMETERS_LIST ,bots_arg_size

#define BOTS_APP_USES_ARG_SIZE
#define BOTS_APP_DEF_ARG_SIZE 14
#define BOTS_APP_DESC_ARG_SIZE "Board size"

int ok(int n, char *a);
void nqueens (int n, int j, char *a, int *solutions);
int verify_queens(int);
void find_queens (int);

#define KERNEL_CALL find_queens(bots_arg_size)

#define KERNEL_CHECK verify_queens(bots_arg_size)

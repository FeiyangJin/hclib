#include "health.h"

#define BOTS_APP_NAME "Health"
#define BOTS_APP_PARAMETERS_DESC "%s"
#define BOTS_APP_PARAMETERS_LIST ,bots_arg_file

#define BOTS_APP_USES_ARG_FILE
#define BOTS_APP_DEF_ARG_FILE "Input filename"
#define BOTS_APP_DESC_ARG_FILE "Health input file (mandatory)"

#define BOTS_APP_INIT \
   struct Village *top;\
   read_input_data(bots_arg_file);

#define KERNEL_INIT \
   allocate_village(&top, NULL, NULL, sim_level, 0);

#define KERNEL_CALL sim_village_main(top);
#define KERNEL_FINI

#define KERNEL_CHECK check_village(top);

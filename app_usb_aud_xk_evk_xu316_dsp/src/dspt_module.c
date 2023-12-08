#include "dspt_module.h"
#include "dspt_filter_module.h"

// Table containing init functions for all modules
//static module_instance_t* (* __attribute__((fptrgroup("dsp_module_fptr_grp"))) dsp_module_init[])(uint8_t id) =  {stage_filter_init};

static DSP_MODULE_INIT_ATTR dsp_module_init DSP_MODULE_INIT_ATTR module_init_functions[] = {stage_filter_init};

module_instance_t* create_module_instance(all_dsp_modules_t dsp_module, uint8_t id)
{
   return module_init_functions[dsp_module](id);
}

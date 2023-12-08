#include <xcore/channel.h>
#include "dspt_module.h"
#include "dspt_filter_module.h"

// Table containing init functions for all modules
//static module_instance_t* (* __attribute__((fptrgroup("dsp_module_fptr_grp"))) dsp_module_init[])(uint8_t id) =  {stage_filter_init};

static DSP_MODULE_INIT_ATTR dsp_module_init DSP_MODULE_INIT_ATTR module_init_functions[] = {stage_filter_init};

module_instance_t* create_module_instance(all_dsp_modules_t dsp_module, uint8_t id)
{
   return module_init_functions[dsp_module](id);
}

#pragma stackfunction 1000
void dsp_thread(chanend_t c_source, chanend_t c_dest, module_info_t *module_info, size_t num_modules)
{
    module_instance_t *modules[num_modules]; // Array of pointers
    for(int i=0; i<num_modules; i++)
    {
        modules[i] = create_module_instance(module_info[i].module, module_info[i].instance_id);
    }

    int32_t input_data[DSP_INPUT_CHANNELS] = {0};
    int32_t output_data[DSP_OUTPUT_CHANNELS] = {0};

    while(1)
    {
        int32_t *input_ptr = input_data;
        int32_t *output_ptr = output_data;
        chan_in_buf_word(c_source, (uint32_t*)input_ptr, DSP_INPUT_CHANNELS);
        for(int i=0; i<num_modules; i++)
        {
            modules[i]->process_sample(input_ptr, output_ptr, modules[i]->state, modules[i]->config);

            if(i < num_modules-1) // If we have more iterations
            {
                int32_t *temp = input_ptr;
                input_ptr = output_ptr;
                output_ptr = temp;
            }
        }
        chan_out_buf_word(c_dest, (uint32_t*)output_ptr, DSP_OUTPUT_CHANNELS);
        // Control
    }
}

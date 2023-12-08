#ifndef FILTER_MODULE_H
#define FILTER_MODULE_H

#include "dspt_module.h"
#include "dsp.h"

#define FILTERS (4)
#define MAX_CHANNELS (4)
// Go in DSP module file
typedef struct
{
    /* data */
    int32_t DWORD_ALIGNED filter_coeffs [FILTERS * DSP_NUM_COEFFS_PER_BIQUAD];
    int32_t num_inputs;
    // TODO Save as a more generic map??
    int32_t num_outputs;
    int32_t input_start_offset;
}filter_config_t;

typedef struct
{
    filter_config_t config;
    int32_t DWORD_ALIGNED filter_states[MAX_CHANNELS][FILTERS * DSP_NUM_STATES_PER_BIQUAD];
}filter_state_t;

// Public functions
DSP_MODULE_INIT_ATTR module_instance_t* stage_filter_init(uint8_t id);

DSP_MODULE_PROCESS_ATTR  void stage_filter_process(int32_t *input, int32_t *output, void *app_data_state, void *app_data_config);


#endif

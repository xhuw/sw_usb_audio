
#include <string.h>
#include <stdlib.h>
#include <xassert.h>
#include "debug_print.h"
#include "parametric_eq.h"
#include "xua_conf.h"

// TODO duplicated defines!
#define DSP_INPUT_CHANNELS (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN)
#define DSP_OUTPUT_CHANNELS (DSP_INPUT_CHANNELS)


DSP_MODULE_PROCESS_ATTR
void parametric_eq_process(int32_t *input, int32_t *output, void *app_data_state, void *app_data_config)
{
    xassert(app_data_state != NULL);
    parametric_eq_state_t *state = app_data_state;

    // 4 biquads over 4 samples take 290 reference timer cycles
    for(int i=0; i<state->config.num_outputs; i++)
    {
        output[i] = dsp_filters_biquads (( int32_t ) input[i + state->config.input_start_offset],
                                                        state->config.filter_coeffs ,
                                                        state->filter_states[i],
                                                        FILTERS ,
                                                        28);
    }
}


DSP_MODULE_INIT_ATTR
module_instance_t* parametric_eq_init(uint8_t id)
{
    module_instance_t *module_instance = malloc(sizeof(module_instance_t));

    parametric_eq_state_t *state = malloc(sizeof(parametric_eq_state_t)); // malloc_from_heap
    parametric_eq_config_t *config = malloc(sizeof(parametric_eq_config_t)); // malloc_from_heap

    memset(state->filter_states, 0, sizeof(state->filter_states));

    // b2 / a0 b1 / a0 b0 / a0 -a1 / a0 -a2 / a0
    const int32_t DWORD_ALIGNED filter_coeffs [ FILTERS * DSP_NUM_COEFFS_PER_BIQUAD] = {
    261565110 , -521424736 , 260038367 , 521424736 , -253168021 ,
    255074543 , -506484921 , 252105451 , 506484921 , -238744538 ,
    280274501 , -523039333 , 245645878 , 523039333 , -257484924 ,
    291645146 , -504140302 , 223757950 , 504140302 , -246967640 ,
    };

    memcpy(state->config.filter_coeffs, filter_coeffs, sizeof(filter_coeffs));

    state->config.num_inputs = DSP_INPUT_CHANNELS;
    state->config.num_outputs = DSP_OUTPUT_CHANNELS;
    state->config.input_start_offset = 0;

    memcpy(config, &state->config, sizeof(parametric_eq_config_t));

    module_instance->module_type = parametric_eq;
    module_instance->id = id;
    module_instance->state = state;
    module_instance->config = config;
    module_instance->process_sample = parametric_eq_process;
    return module_instance;
}

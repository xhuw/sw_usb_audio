#include <string.h>
#include <stdlib.h>
#include <xassert.h>
#include "debug_print.h"
#include <xcore/channel.h>
#include <xcore/chanend.h>
#include <xcore/parallel.h>
#include <xcore/select.h>
#include <xcore/hwtimer.h>
#include "xua_conf.h"
#include "dsp.h"
#include "dspt_module.h"
#include "dspt_control.h"
#include "parametric_eq.h"

DECLARE_JOB(dsp_data_transport_thread, (chanend_t, chanend_t, chanend_t));
DECLARE_JOB(dsp_control_thread, (chanend_t, module_instance_control_t **, size_t));
DECLARE_JOB(dsp_thread, (chanend_t, chanend_t, module_instance_t**, size_t));


void dsp_data_transport_thread(chanend_t c_data, chanend_t c_start, chanend_t c_end)
{
    int32_t input_data[DSP_INPUT_CHANNELS] = {0};
    int32_t output_data[DSP_OUTPUT_CHANNELS] = {0};

    SELECT_RES(
    CASE_THEN(c_data, event_ubm_audio_exchange),
    CASE_THEN(c_end, event_dsp_sample_done)
    )
    {
        event_ubm_audio_exchange: {
            for(int i = 0; i < DSP_INPUT_CHANNELS; i++) {
                int x = chanend_in_word(c_data);
                input_data[i] = x;
            }
            chanend_check_end_token(c_data);

            for(int i = 0; i < DSP_OUTPUT_CHANNELS; i++) {
                chanend_out_word(c_data, output_data[i]);
            }

            for(int i = 0; i < DSP_OUTPUT_CHANNELS; i++) {
                output_data[i] = 0x40000000;
            }
            chanend_out_end_token(c_data);

            // To DSP
            chan_out_buf_word(c_start, (uint32_t*)&input_data[0], DSP_INPUT_CHANNELS);
            continue;
        }
        event_dsp_sample_done: {
            // From DSP
            chan_in_buf_word(c_end, (uint32_t*)&output_data[0], DSP_OUTPUT_CHANNELS);
            continue;
        }
    }
}

#define NUM_DSP_THREADS (2)
#define MAX_MODULES_PER_THREAD (8)

/// Create module control instance from a pre-initialised module instance
static module_instance_control_t* create_module_control_instance(module_instance_t *module_instance)
{
    module_instance_control_t *module_control = malloc(sizeof(module_instance_control_t));
    module_control->module_type = module_instance->module_type;
    module_control->id = module_instance->id;
    module_control->config = module_instance->config;
    module_control->dirty = false;
    module_control->num_control_commands = module_instance->num_control_commands;
    return module_control;
}

#pragma stackfunction 1000
void dspt_xcore_main(chanend_t c_data, chanend_t c_control)
{
    channel_t chan_start_dsp, chan_end_dsp;
    chan_start_dsp = chan_alloc();
    chan_end_dsp = chan_alloc();

#if NUM_DSP_THREADS > 1
    channel_t chan_intermediate[NUM_DSP_THREADS - 1];
    for(int i=0; i<NUM_DSP_THREADS - 1; i++)
    {
        chan_intermediate[i] = chan_alloc();
    }
#endif
    // pointer to an array
    // Setup the what runs where map
    module_info_t info_thread1[MAX_MODULES_PER_THREAD], info_thread2[MAX_MODULES_PER_THREAD];
    const int32_t num_modules_thread1 = 2, num_modules_thread2 = 1;
    info_thread1[0].instance_id = 1;
    info_thread1[0].module_init_function = parametric_eq_init;
    info_thread1[1].instance_id = 2;
    info_thread1[1].module_init_function = parametric_eq_init;

    info_thread2[0].instance_id = 3;
    info_thread2[0].module_init_function = parametric_eq_init;

    const int32_t total_num_modules = num_modules_thread1 + num_modules_thread2;
    module_instance_t *modules[total_num_modules]; // Array of pointers to module instances
    module_instance_control_t *modules_control[total_num_modules];  // Array of pointers to module control instances

    for(int i=0; i<num_modules_thread1; i++)
    {
        modules[i] = info_thread1[i].module_init_function(info_thread1[i].instance_id);
        modules_control[i] = create_module_control_instance(modules[i]);
    }

    for(int i=0; i<num_modules_thread2; i++)
    {
        modules[num_modules_thread1 + i] = info_thread2[i].module_init_function(info_thread2[i].instance_id);
        modules_control[num_modules_thread1 + i] = create_module_control_instance(modules[num_modules_thread1 + i]);
    }

    PAR_JOBS(
        PJOB(dsp_data_transport_thread, (c_data, chan_start_dsp.end_a, chan_end_dsp.end_b)),
        PJOB(dsp_control_thread, (c_control, modules_control, total_num_modules)),
        PJOB(dsp_thread, (chan_start_dsp.end_b, chan_intermediate[0].end_a, &modules[0], num_modules_thread1)),
        PJOB(dsp_thread, (chan_intermediate[0].end_b, chan_end_dsp.end_a, &modules[num_modules_thread1], num_modules_thread2))
    );
}
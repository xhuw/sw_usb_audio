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
#include "agc.h"




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

//#include "autogen_main.inc"

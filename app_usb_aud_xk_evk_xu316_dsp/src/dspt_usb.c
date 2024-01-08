#include <xcore/channel.h>
#include <xcore/select.h>
#include "xua_conf.h"


extern void dsp_main(chanend_t c_source, chanend_t c_sink, chanend_t c_control);

static chanend_t g_c_to_dsp, g_c_from_dsp;

void UserBufferManagementInit()
{
    //TODO
}

static void app_dsp_source(int32_t *samples, unsigned num_samples)
{
    for(int i=0; i<num_samples; i++)
    {
        chanend_out_word(g_c_to_dsp, samples[i]);
    }
    chanend_out_end_token(g_c_to_dsp);
}

static void app_dsp_sink_non_blocking(int32_t *samples, unsigned num_samples)
{
    SELECT_RES_ORDERED(
        CASE_THEN(g_c_from_dsp, event_recv_from_dsp),
        DEFAULT_THEN(event_default)
    )
    {
        event_recv_from_dsp:
        {
            for(int i=0; i<num_samples; i++)
            {
                samples[i] = chanend_in_word(g_c_from_dsp);
            }
            chanend_check_end_token(g_c_from_dsp);
            break;
        }
        event_default:
        {
            for(int i=0; i<num_samples; i++)
            {
                samples[i] = 0x7fffffff;
            }
            break;
        }
    }
}

static void app_dsp_sink_blocking(int32_t *samples, unsigned num_samples)
{
    for(int i=0; i<num_samples; i++)
    {
        samples[i] = chanend_in_word(g_c_from_dsp);
    }
}


void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[])
{
    int32_t input[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN];
    for(int ch=0; ch<NUM_USB_CHAN_OUT; ch++) // From USB
    {
        input[ch] = (int32_t)sampsFromUsbToAudio[ch];
    }
    for(int ch=0; ch<NUM_USB_CHAN_IN; ch++) // From Audio
    {
        input[ch + NUM_USB_CHAN_OUT] = (int32_t)sampsFromAudioToUsb[ch];
    }
    app_dsp_source(input, NUM_USB_CHAN_OUT+NUM_USB_CHAN_IN);


    int32_t output[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN];
    app_dsp_sink_non_blocking(output, NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN);

    for(int ch=0; ch<NUM_USB_CHAN_OUT; ch++) // To Audio
    {
        sampsFromAudioToUsb[ch] = (unsigned)output[ch];
    }
    for(int ch=0; ch<NUM_USB_CHAN_IN; ch++) // To USB
    {
        sampsFromUsbToAudio[ch] = (unsigned)output[NUM_USB_CHAN_OUT + ch];
    }
}


void dsp_main_wrapper(chanend_t c_control) {
    channel_t c_to_dsp = chan_alloc();
    g_c_to_dsp = c_to_dsp.end_a;

    channel_t c_from_dsp = chan_alloc();
    g_c_from_dsp = c_from_dsp.end_b;

    dsp_main(c_to_dsp.end_b, c_from_dsp.end_a, c_control);
}


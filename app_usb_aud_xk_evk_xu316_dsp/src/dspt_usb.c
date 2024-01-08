#include <xcore/channel.h>
#include <xcore/select.h>
#include "xua_conf.h"


extern void dsp_main(chanend_t c_source, chanend_t c_sink, chanend_t c_control);

static chanend_t g_c_to_dsp, g_c_from_dsp;

void UserBufferManagementInit()
{
    //TODO
}

void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[])
{
    //offload_data_to_dsp_engine(g_c_to_dsp, sampsFromUsbToAudio, sampsFromAudioToUsb);
    for(int ch=0; ch<NUM_USB_CHAN_OUT; ch++) // From USB
    {
        chanend_out_word(g_c_to_dsp, sampsFromUsbToAudio[ch]);
    }
    for(int ch=0; ch<NUM_USB_CHAN_IN; ch++) // From Audio
    {
        chanend_out_word(g_c_to_dsp, sampsFromAudioToUsb[ch]);
    }
    chanend_out_end_token(g_c_to_dsp);

    // Do a non blocking receive from DSP
    SELECT_RES_ORDERED(
        CASE_THEN(g_c_from_dsp, event_recv_from_dsp),
        DEFAULT_THEN(event_default)
    )
    {
        event_recv_from_dsp:
        {
            // Note: We're routing USB OUT -> USB IN and I2S ADC -> I2S DAC
            for(int ch=0; ch<NUM_USB_CHAN_OUT; ch++) // To Audio
            {
                sampsFromAudioToUsb[ch] = chanend_in_word(g_c_from_dsp);
            }
            for(int ch=0; ch<NUM_USB_CHAN_IN; ch++) // To USB
            {
                sampsFromUsbToAudio[ch] = chanend_in_word(g_c_from_dsp);
            }
            chanend_check_end_token(g_c_from_dsp);
            break;
        }
        event_default:
        {
            for(int ch=0; ch<NUM_USB_CHAN_OUT; ch++) // To Audio
            {
                sampsFromAudioToUsb[ch] = 0x7fffffff;
            }
            for(int ch=0; ch<NUM_USB_CHAN_IN; ch++) // To USB
            {
                sampsFromUsbToAudio[ch] = 0x7fffffff;;
            }
            break;
        }
    }
}


void dsp_main_wrapper(chanend_t c_control) {
    channel_t c_to_dsp = chan_alloc();
    g_c_to_dsp = c_to_dsp.end_a;

    channel_t c_from_dsp = chan_alloc();
    g_c_from_dsp = c_from_dsp.end_b;

    dsp_main(c_to_dsp.end_b, c_from_dsp.end_a, c_control);
}


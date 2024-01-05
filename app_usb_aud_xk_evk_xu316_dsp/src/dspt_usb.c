#include <xcore/channel.h>
#include "xua_conf.h"

extern void dspt_xcore_main(chanend_t c_data, chanend_t c_control);

static chanend_t g_c_to_dsp;

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

    // Note: We're routing USB OUT -> USB IN and I2S ADC -> I2S DAC
    for(int ch=0; ch<NUM_USB_CHAN_OUT; ch++) // To Audio
    {
        sampsFromAudioToUsb[ch] = chanend_in_word(g_c_to_dsp);
    }
    for(int ch=0; ch<NUM_USB_CHAN_IN; ch++) // To USB
    {
        sampsFromUsbToAudio[ch] = chanend_in_word(g_c_to_dsp);
    }
    chanend_check_end_token(g_c_to_dsp);
}


void dsp_main(chanend_t c_control) {
    channel_t c_data = chan_alloc();
    g_c_to_dsp = c_data.end_a;
    dspt_xcore_main(c_data.end_b, c_control);
}


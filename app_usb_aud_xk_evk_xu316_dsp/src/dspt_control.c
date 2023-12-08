#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xassert.h>
#include "debug_print.h"
#include <xcore/channel.h>
#include <xcore/chanend.h>
#include <xcore/select.h>
#include "dspt_control.h"

#define MAX_CONTROL_PAYLOAD_LEN 64
void dsp_control_thread(chanend_t c_control)
{
    int8_t payload[MAX_CONTROL_PAYLOAD_LEN] = {0};
    uint8_t temp = chan_in_byte(c_control);
    printf("dsp_control_thread received init token %d\n", temp);

    SELECT_RES(
        CASE_THEN(c_control, event_do_control)
    )
    {
        event_do_control: {
            control_req_t req;
            chan_in_buf_byte(c_control, (uint8_t*)&req, sizeof(control_req_t));
            if(req.cmd_id & 0x80) // Read command
            {
                debug_printf("read cmd\n");
                // Return read payload
                chan_out_buf_byte(c_control, (uint8_t*)payload, req.payload_len);
            }
            else // write command
            {
                debug_printf("write cmd\n");
                // Receive write payload
                chan_in_buf_byte(c_control, (uint8_t*)payload, req.payload_len);
            }
        }
        continue;
    }
}

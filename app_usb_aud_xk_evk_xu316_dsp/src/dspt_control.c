#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xassert.h>
#include "debug_print.h"
#include <xcore/channel.h>
#include <xcore/chanend.h>
#include <xcore/select.h>
#include "dspt_control.h"
#include "dspt_module.h"

#define APP_CONTROL_CMD_CONFIG 0x00
#define APP_CONTROL_CMD_CONFIG_OFFSET 0x01

static module_instance_t* get_module_instance(module_instance_t **modules, uint32_t res_id, size_t num_modules)
{
    for(int i=0; i<num_modules; i++)
    {
        if(modules[i]->id == res_id)
        {
            return modules[i];
        }
    }
    return NULL;
}


#define MAX_CONTROL_PAYLOAD_LEN 64
void dsp_control_thread(chanend_t c_control, module_instance_t** modules, size_t num_modules)
{
    uint32_t current_config_offset = 0;
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
                if((req.cmd_id & 0x7f) == APP_CONTROL_CMD_CONFIG_OFFSET)
                {
                    chan_out_buf_byte(c_control, (uint8_t*)&current_config_offset, req.payload_len);
                }
                else if((req.cmd_id & 0x7f) == APP_CONTROL_CMD_CONFIG)
                {
                    //debug_printf("APP_CONTROL_CMD_CONFIG\n");
                    module_instance_t *module = get_module_instance(modules, req.res_id, num_modules);
                    xassert(module != NULL);

                    memcpy((uint8_t*)payload, (uint8_t*)module->config + current_config_offset, req.payload_len);

                    // Return read payload
                    chan_out_buf_byte(c_control, (uint8_t*)payload, req.payload_len);
                }
                else
                {
                    xassert(0);
                }
            }
            else // write command
            {
                // Receive write payload
                if(req.cmd_id == APP_CONTROL_CMD_CONFIG_OFFSET)
                {
                    chan_in_buf_byte(c_control, (uint8_t*)&current_config_offset, req.payload_len);
                }
                else if(req.cmd_id == APP_CONTROL_CMD_CONFIG)
                {
                    module_instance_t *module = get_module_instance(modules, req.res_id, num_modules);

                    xassert(module != NULL);

                    chan_in_buf_byte(c_control, (uint8_t*)module->config + current_config_offset, req.payload_len);
                }
                else
                {
                    xassert(0);
                }
            }
        }
        continue;
    }
}

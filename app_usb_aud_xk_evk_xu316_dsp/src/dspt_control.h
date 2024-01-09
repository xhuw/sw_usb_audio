#ifndef DSPT_CONTROL_H
#define DSPT_CONTROL_H

#include "adsp_module.h"
#include "xcore/chanend.h"
#include <stdint.h>
#include <xcore/parallel.h>

typedef struct
{
    uint16_t res_id;
    uint16_t cmd_id;
    uint16_t payload_len;
    uint8_t direction;
}control_req_t;

DECLARE_JOB(dsp_control_thread, (chanend_t, module_instance_t**, size_t));



#endif

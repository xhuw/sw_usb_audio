#ifndef DSPT_CONTROL_H
#define DSPT_CONTROL_H

#include <stdint.h>

typedef struct
{
    uint16_t res_id;
    uint16_t cmd_id;
    uint16_t payload_len;
    uint8_t direction;
}control_req_t;

#endif

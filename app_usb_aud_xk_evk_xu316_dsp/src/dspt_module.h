#ifndef DSPT_MODULE_H
#define DSPT_MODULE_H

#include <stdint.h>
#include <stdbool.h>
#include "xua_conf.h" // For NUM_USB_CHAN_OUT and NUM_USB_CHAN_IN defines

#define DSP_INPUT_CHANNELS (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN)
#define DSP_OUTPUT_CHANNELS (DSP_INPUT_CHANNELS)

#define DWORD_ALIGNED     __attribute__ ((aligned(8)))

typedef enum
{
    parametric_eq = 0,
    agc,
    num_dsp_modules
}all_dsp_modules_t;

#define DSP_MODULE_PROCESS_ATTR  __attribute__((fptrgroup("dsp_module_process_fptr_grp")))
typedef void (*dsp_module_process)(int32_t *input, int32_t *output, void *state, void *config);

typedef struct
{
    uint32_t id;    // Unique module identifier assigned by the host
    void *state;    // Pointer to the module's state memory
    void *config;   // Pointer to the shared config memory between module and the control thread
    DSP_MODULE_PROCESS_ATTR dsp_module_process process_sample;  // Pointer to the module's process_sample() function
    all_dsp_modules_t module_type;
}module_instance_t;

typedef struct
{
    all_dsp_modules_t module_type;
    uint32_t id;
    void *config;
    bool dirty; // flag specifying if the config has been written by the control thread
    uint8_t cmd_id; // cmd_id of the last command that was sent to this module instance. Valid only if dirty = true
}module_instance_control_t;


#define DSP_MODULE_INIT_ATTR  __attribute__((fptrgroup("dsp_module_init_fptr_grp")))
typedef module_instance_t* (*dsp_module_init)(uint8_t id);


typedef struct
{
    uint8_t instance_id;
    DSP_MODULE_INIT_ATTR dsp_module_init module_init_function;  // Pointer to the module's init function
}module_info_t;

#endif

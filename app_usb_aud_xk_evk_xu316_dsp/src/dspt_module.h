#ifndef DSPT_MODULE_H
#define DSPT_MODULE_H

#include <stdint.h>

#define DWORD_ALIGNED     __attribute__ ((aligned(8)))

#define DSP_MODULE_PROCESS_ATTR  __attribute__((fptrgroup("dsp_module_process_fptr_grp")))
typedef void (*dsp_module_process)(int32_t *input, int32_t *output, void *state, void *config);

typedef struct
{
    /* data */
    uint32_t id;
    void *state;
    void *config;
    //void (* __attribute__((fptrgroup("dsp_module_fptr_grp"))) process_sample)(int32_t *input, int32_t *output, void *state, void *config);
    DSP_MODULE_PROCESS_ATTR dsp_module_process process_sample;
}module_instance_t;

#define DSP_MODULE_INIT_ATTR  __attribute__((fptrgroup("dsp_module_init_fptr_grp")))
typedef module_instance_t* (*dsp_module_init)(uint8_t id);

typedef enum
{
    biquads_4_sections = 0,
    num_dsp_modules
}all_dsp_modules_t;


// Function declaration
module_instance_t* create_module_instance(all_dsp_modules_t dsp_module, uint8_t id);

typedef struct
{
    uint8_t instance_id;
    all_dsp_modules_t module;
}module_info_t;


#endif

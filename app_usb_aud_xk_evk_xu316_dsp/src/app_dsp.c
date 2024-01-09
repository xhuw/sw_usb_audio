
#include "app_dsp.h"
#include "stdbool.h"
#include "xcore/chanend.h"
#include "xcore/parallel.h"
#include "dspt_control.h"

#include <adsp_pipeline.h>

static audio_dsp_t m_dsp;

// send data to dsp
void app_dsp_source(REFERENCE_PARAM(int32_t, data)) {
    int32_t* in_data[] = {
        &data[0],
        &data[1],
        &data[2],
        &data[3]
    };
    adsp_pipeline_source(&m_dsp, in_data);
}

// read output
void app_dsp_sink(REFERENCE_PARAM(int32_t, data)) {
    static bool pipeline_full = false;
    int32_t* out_data[] = {
        &data[0],
        &data[1],
        &data[2],
        &data[3]
    };
    if(pipeline_full) {
        // once first data has been received, the pipeline 
        // is full and data will be available every cycle
        // so use blocking API from that point forward
        adsp_pipeline_sink(&m_dsp, out_data);
    } else {
        pipeline_full = adsp_pipeline_sink_nowait(&m_dsp, out_data);
    }
}



// do dsp
void app_dsp_main(chanend_t c_control) {
    adsp_pipeline_init(&m_dsp);

    adsp_module_array_t modules = adsp_pipeline_get_modules(&m_dsp);
    PAR_JOBS(
        PJOB(adsp_pipeline_main, (&m_dsp)),
        PJOB(dsp_control_thread, (c_control, modules.modules, modules.num_modules))
    );
    adsp_pipeline_main(&m_dsp);
}


// TODO control



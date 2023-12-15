#define NUM_DSP_THREADS (2)
#define MAX_MODULES_PER_THREAD (8)

#pragma stackfunction 1000
void dspt_xcore_main(chanend_t c_data, chanend_t c_control)
{
    channel_t chan_in = chan_alloc();
    channel_t chan_0 = chan_alloc();
    channel_t chan_1 = chan_alloc();

    const int32_t num_modules_thread0 = 3;
    const int32_t num_modules_thread1 = 1;
    int total_num_modules = 4;
    module_instance_t* all_modules[4];

    module_instance_t* modules0[] = {
        parametric_eq_init(0),
        parametric_eq_init(1),
        parametric_eq_init(2),
    };

    all_modules[0] = modules0[0];
    all_modules[1] = modules0[1];
    all_modules[2] = modules0[2];
    module_instance_t* modules1[] = {
        agc_init(3),
    };

    all_modules[3] = modules1[0];

     PAR_JOBS(
        PJOB(dsp_data_transport_thread, (c_data, chan_in.end_a, chan_1.end_b)),
        PJOB(dsp_control_thread, (c_control, all_modules, total_num_modules)),
        PJOB(dsp_thread, (chan_in.end_b, chan_0.end_a, modules0, num_modules_thread0)),
        PJOB(dsp_thread, (chan_0.end_b, chan_1.end_a, modules1, num_modules_thread1))
    );
}

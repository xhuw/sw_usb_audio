#pragma once

#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/parallel.h>
#include <stddef.h>
#include "adsp_module.h"

#include "agc.h"
#include "parametric_eq.h"

DECLARE_JOB(dsp_data_transport_thread, (chanend_t, chanend_t, chanend_t));
DECLARE_JOB(dsp_control_thread, (chanend_t, module_instance_t **, size_t));
DECLARE_JOB(dsp_thread, (chanend_t, chanend_t, module_instance_t**, size_t));

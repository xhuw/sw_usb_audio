cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_usb_aud_xk_evk_xu316_dsp)


set(APP_HW_TARGET XK-EVK-XU316)
include(${CMAKE_CURRENT_LIST_DIR}/../deps.cmake)
set(APP_PCA_ENABLE OFF)
set(SW_USB_AUDIO_FLAGS ${EXTRA_BUILD_FLAGS} -fcomment-asm
                                            -Wall
                                            -O3
                                            -report
                                            -lquadflash
                                            -g
                                            -fxscope
                                            -DUSB_TILE=tile[0]
                                            -DADAT_TX_USE_SHARED_BUFF=1
                                            -DQUAD_SPI_FLASH=1
                                            -DXSCOPE
                                            -DUAC_FORCE_FEEDBACK=1
                                            -DUSB_CONTROL_DESCS=1)

set(APP_COMPILER_FLAGS_2AMi2o2xxxxxx ${SW_USB_AUDIO_FLAGS})


file(GLOB C_SRC CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_LIST_DIR} src/*.c)

set(APP_C_SRCS
    "${C_SRC};build/dsp_pipeline/dsp_main.c")

#set(APP_COMPILER_FLAGS_test ${SW_USB_AUDIO_FLAGS} -DOFFSET_GEN=1 -DEXCLUDE_USB_AUDIO_MAIN -fcmdline-buffer-bytes=512)

include(${CMAKE_CURRENT_LIST_DIR}/configs_test.cmake)

set(APP_INCLUDES
    src
    src/core
    src/extensions
    ${LIB_AUDIO_DSP_PATH}/api/stages
    ${CMAKE_CURRENT_SOURCE_DIR}/build/dsp_pipeline)
set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

XMOS_REGISTER_APP()

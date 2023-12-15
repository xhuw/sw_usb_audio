cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_usb_aud_xk_evk_xu316_dsp)


set( AUTOGEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/src.autogen )
#file( GLOB MODULE_CONFIG_YAML_FILES  ${CMAKE_CURRENT_LIST_DIR}/src/yaml/*.yaml )
file( GLOB MODULE_CONFIG_YAML_FILES  ${CMAKE_CURRENT_LIST_DIR}/build/dsp_design/yaml/*.yaml )
file(GLOB TEMPLATE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/templates/*.mako)
unset(CMD_MAP_GEN_ARGS)
list(APPEND CMD_MAP_GEN_ARGS --out-dir ${AUTOGEN_DIR})
set(CMD_MAP_GEN_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/src/python/parse_config.py)

# Get output C file names
set(OUTPUT_C_FILES ${AUTOGEN_DIR}/generator/gen_cmd_map_offset.c)

add_custom_command(
    OUTPUT ${OUTPUT_C_FILES}
    COMMAND python ${CMD_MAP_GEN_SCRIPT} ${CMD_MAP_GEN_ARGS}
    DEPENDS ${MODULE_CONFIG_YAML_FILES} ${CMD_MAP_GEN_SCRIPT} ${TEMPLATE_FILES}
    COMMENT "Generating cmd_map files included in the device and host application"
    VERBATIM
)

add_custom_target(cmd_map_generation
    DEPENDS ${OUTPUT_C_FILES})

set(APP_HW_TARGET XK-EVK-XU316)
include(${CMAKE_CURRENT_LIST_DIR}/../deps.cmake)
set(APP_PCA_ENABLE ON)
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
                                            -DUAC_FORCE_FEEDBACK=1)

set(APP_COMPILER_FLAGS_1AMi2o2xxxxxx ${SW_USB_AUDIO_FLAGS} -DAUDIO_CLASS=1)

set(APP_COMPILER_FLAGS_2AMi2o2xxxxxx ${SW_USB_AUDIO_FLAGS})


file(GLOB C_SRC RELATIVE ${CMAKE_CURRENT_LIST_DIR} src/*.c)

set(APP_C_SRCS "${C_SRC};build/dsp_pipeline/dsp_main.c")
message(APP_C_SRCS "${APP_C_SRCS}")

#set(APP_COMPILER_FLAGS_test ${SW_USB_AUDIO_FLAGS} -DOFFSET_GEN=1 -DEXCLUDE_USB_AUDIO_MAIN -fcmdline-buffer-bytes=512)

include(${CMAKE_CURRENT_LIST_DIR}/configs_test.cmake)

set(APP_INCLUDES src src/core src/extensions ${AUTOGEN_DIR}/common ${AUTOGEN_DIR}/device)
set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

XMOS_REGISTER_APP()

foreach(BUILD_TARGET ${APP_BUILD_TARGETS})
    add_dependencies(${BUILD_TARGET} cmd_map_generation)
endforeach()


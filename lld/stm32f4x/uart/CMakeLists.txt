include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_static_lib_variants(
  TARGET
    thor_lld_stm32f4_uart
  SOURCES
    hw_uart_driver.cpp
    hw_uart_data.cpp
    ./../../../lld/interface/uart/common_driver/uart_common_driver.cpp
  PRV_LIBRARIES
    chimera_intf_inc
    thor_intf_inc

    lld_common_inc
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/Thor/lld"
)

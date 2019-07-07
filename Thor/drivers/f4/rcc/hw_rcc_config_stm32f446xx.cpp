/********************************************************************************
 *   File Name:
 *    hw_rcc_config_stm32f446xx.cpp
 *
 *   Description:
 *    Contains implementation details for reset and clock configuration options
 *    for the STM32F446xx.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include <Thor/drivers/f4/rcc/hw_rcc_config_stm32f446xx.hpp>

#ifdef  __cplusplus
extern "C"
{
#endif

  uint32_t SystemCoreClock = 16000000;
  const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
  const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

#ifdef  __cplusplus
}
#endif
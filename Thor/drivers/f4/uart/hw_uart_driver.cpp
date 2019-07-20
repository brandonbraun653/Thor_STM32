/********************************************************************************
 *   File Name:
 *    hw_uart_driver.hpp
 *
 *   Description:
 *    STM32 Driver for the UART Peripheral
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Chimera Includes */
#include <Chimera/types/common_types.hpp>

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/f4/uart/hw_uart_driver.hpp>
#include <Thor/drivers/f4/uart/hw_uart_prj.hpp>

#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_UART == 1 )

namespace Thor::Driver::UART
{
 
  bool isUART( const std::uintptr_t address )
  {
    bool result = false;

    for ( auto &val : periphAddressList )
    {
      if ( val == address )
      {
        result = true;
      }
    }
    return result;
  }

}    // namespace Thor::Driver::UART

#endif /* TARGET_STM32F4 && THOR_DRIVER_UART */
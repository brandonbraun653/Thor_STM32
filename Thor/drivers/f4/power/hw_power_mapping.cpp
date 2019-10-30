/********************************************************************************
 *   File Name:
 *    hw_power_mapping.hpp
 *
 *   Description:
 *    Provides structures for conversion and mapping between data types for fast
 *    runtime performance of driver code.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/f4/power/hw_power_mapping.hpp>

#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_PWR == 1 )

namespace Thor::Driver::PWR
{
#if defined( _EMBEDDED )
  RegisterMap *const PWR_PERIPH = reinterpret_cast<RegisterMap *const>( PWR_BASE_ADDR );

#elif defined( _SIM )
  RegisterMap *const PWR_PERIPH = new RegisterMap;

#endif

}

#endif /* TARGET_STM32F4 && THOR_DRIVER_PWR */
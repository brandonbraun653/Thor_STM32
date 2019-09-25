/********************************************************************************
 *   File Name:
 *    hw_iwdg_mapping.cpp
 *
 *   Description:
 *    Mappings for the watchdog timer resources
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include <Thor/drivers/f4/iwdg/hw_iwdg_mapping.hpp>
#include <Thor/drivers/f4/iwdg/hw_iwdg_types.hpp>


#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_IWDG == 1 )
namespace Thor::Driver::IWDG
{
  const Chimera::Container::LightFlatMap<std::uintptr_t, size_t> InstanceToResourceIndex = {
    { reinterpret_cast<std::uintptr_t>( IWDG_PERIPH ), 0u }
  };
}
#endif /* TARGET_STM32F4 && THOR_DRIVER_WATCHDOG */

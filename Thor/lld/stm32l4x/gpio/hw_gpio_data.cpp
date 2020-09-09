/********************************************************************************
 *  File Name:
 *    hw_gpio_mapping.cpp
 *
 *  Description:
 *    Provides structures for conversion and mapping between data types for fast
 *    runtime performance of driver code.
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* STL Includes */
#include <array>
#include <limits>

/* Driver Includes */
#include <Thor/cfg>
#include <Thor/lld/interface/gpio/gpio_prv_data.hpp>

#if defined( TARGET_STM32L4 ) && defined( THOR_LLD_GPIO )

namespace Thor::LLD::GPIO
{
  /*-------------------------------------------------------------------------------
  Peripheral Memory Maps
  -------------------------------------------------------------------------------*/
#if defined( STM32_GPIOA_PERIPH_AVAILABLE )
  RegisterMap *GPIOA_PERIPH = reinterpret_cast<RegisterMap*>( GPIOA_BASE_ADDR );
#endif
#if defined( STM32_GPIOB_PERIPH_AVAILABLE )
  RegisterMap *GPIOB_PERIPH = reinterpret_cast<RegisterMap*>( GPIOB_BASE_ADDR );
#endif
#if defined( STM32_GPIOC_PERIPH_AVAILABLE )
  RegisterMap *GPIOC_PERIPH = reinterpret_cast<RegisterMap*>( GPIOC_BASE_ADDR );
#endif
#if defined( STM32_GPIOD_PERIPH_AVAILABLE )
  RegisterMap *GPIOD_PERIPH = reinterpret_cast<RegisterMap*>( GPIOD_BASE_ADDR );
#endif
#if defined( STM32_GPIOE_PERIPH_AVAILABLE )
  RegisterMap *GPIOE_PERIPH = reinterpret_cast<RegisterMap*>( GPIOE_BASE_ADDR );
#endif
#if defined( STM32_GPIOH_PERIPH_AVAILABLE )
  RegisterMap *GPIOH_PERIPH = reinterpret_cast<RegisterMap*>( GPIOH_BASE_ADDR );
#endif

/*-------------------------------------------------------------------------------
  Configuration Maps
  -------------------------------------------------------------------------------*/
  namespace ConfigMap
  { /* clang-format off */
    LLD_CONST Reg32_t PullMap[ static_cast<size_t>( Chimera::GPIO::Pull::NUM_OPTIONS ) ] =
    {
      OPT_PUPDR::NOPULL,
      OPT_PUPDR::PULLUP,
      OPT_PUPDR::PULLDOWN
    };

    LLD_CONST Reg32_t ModeMap[ static_cast<size_t>( Chimera::GPIO::Drive::NUM_OPTIONS ) ] =
    {
      OPT_MODER::INPUT,
      OPT_MODER::OUTPUT,
      OPT_MODER::OUTPUT,
      OPT_MODER::AF,
      OPT_MODER::AF,
      OPT_MODER::ANALOG,
      OPT_MODER::INPUT
    };

    LLD_CONST Reg32_t SpeedMap[ static_cast<size_t>( Thor::LLD::GPIO::Speed::NUM_OPTIONS ) ] =
    {
      OPT_OSPEEDR::LOW,
      OPT_OSPEEDR::MEDIUM,
      OPT_OSPEEDR::HIGH,
      OPT_OSPEEDR::VERY_HIGH
    };

    LLD_CONST Reg32_t PortToIteratorMap[ static_cast<size_t>( Chimera::GPIO::Port::NUM_OPTIONS ) ] =
    {
      GPIOA_RESOURCE_INDEX,
      GPIOB_RESOURCE_INDEX,
      GPIOC_RESOURCE_INDEX,
      GPIOD_RESOURCE_INDEX,
      GPIOE_RESOURCE_INDEX,
      INVALID_RESOURCE_INDEX,
      INVALID_RESOURCE_INDEX,
      GPIOH_RESOURCE_INDEX
    };
  } /* clang-format on */

  /*-------------------------------------------------------------------------------
  Peripheral Resources
  -------------------------------------------------------------------------------*/
  namespace Resource
  { /* clang-format off */
  
  } /* clang-format on */

}    // namespace Thor::LLD::GPIO

#endif /* TARGET_STM32L4 && THOR_LLD_GPIO */
/********************************************************************************
 *  File Name:
 *    hw_wwdg_driver.cpp
 *
 *  Description:
 *    Implements the LLD interface to the WWDG hardware.
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* STL Includes */
#include <cstring>
#include <limits>

/* Chimera Includes */
#include <Chimera/algorithm>
#include <Chimera/common>
#include <Chimera/utility>

/* Driver Includes */
#include <Thor/cfg>
#include <Thor/hld/interrupt/hld_interrupt_definitions.hpp>
#include <Thor/lld/interface/wwdg/wwdg_prv_data.hpp>
#include <Thor/lld/interface/wwdg/wwdg_intf.hpp>
#include <Thor/lld/stm32l4x/wwdg/hw_wwdg_prj.hpp>
#include <Thor/lld/stm32l4x/wwdg/hw_wwdg_types.hpp>
#include <Thor/lld/stm32l4x/rcc/hw_rcc_driver.hpp>

#if defined( TARGET_STM32L4 ) && defined( THOR_LLD_WWDG )

namespace Thor::LLD::WWDG
{
  /*-------------------------------------------------------------------------------
  Variables
  -------------------------------------------------------------------------------*/
  static Driver s_wwdg_drivers[ NUM_WWDG_PERIPHS ];

  /*-------------------------------------------------------------------------------
  Private Functions
  -------------------------------------------------------------------------------*/

  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  Chimera::Status_t initialize()
  {
    /*-------------------------------------------------
    Attach all the expected peripherals to the drivers
    -------------------------------------------------*/
    if ( attachDriverInstances( s_wwdg_drivers, ARRAY_COUNT( s_wwdg_drivers ) ) )
    {
      return Chimera::Status::OK;
    }
    else
    {
      return Chimera::Status::FAIL;
    }
  }


  Driver_rPtr getDriver( const Chimera::WWDG::Channel channel )
  {
    if ( auto idx = getResourceIndex( channel ); idx != INVALID_RESOURCE_INDEX )
    {
      return &s_wwdg_drivers[ idx ];
    }
    else
    {
      return nullptr;
    }
  }


  /*-------------------------------------------------------------------------------
  Low Level Driver Implementation
  -------------------------------------------------------------------------------*/
  Driver::Driver()
  {
  }

  Driver::~Driver()
  {
  }


  Chimera::Status_t Driver::attach( RegisterMap *const peripheral )
  {
    /*------------------------------------------------
    Get peripheral descriptor settings
    ------------------------------------------------*/
    mPeriph       = peripheral;
    mResourceIndex = getResourceIndex( reinterpret_cast<std::uintptr_t>( peripheral ) );

    /*------------------------------------------------
    Handle the ISR configuration
    ------------------------------------------------*/
    Thor::LLD::IT::disableIRQ( Resource::IRQSignals[ mResourceIndex ] );
    Thor::LLD::IT::clearPendingIRQ( Resource::IRQSignals[ mResourceIndex ] );
    Thor::LLD::IT::setPriority( Resource::IRQSignals[ mResourceIndex ], Thor::Interrupt::WWDG_IT_PREEMPT_PRIORITY, 0u );

    return Chimera::Status::OK;
  }


  Chimera::Status_t Driver::reset()
  {
    return Chimera::Status::OK;
  }


  void Driver::clockEnable()
  {
    auto rcc = Thor::LLD::RCC::getPeripheralClock();
    rcc->enableClock( Chimera::Peripheral::Type::PERIPH_WWDG, mResourceIndex );
  }


  void Driver::clockDisable()
  {
    auto rcc = Thor::LLD::RCC::getPeripheralClock();
    rcc->disableClock( Chimera::Peripheral::Type::PERIPH_WWDG, mResourceIndex );
  }


  inline void Driver::enterCriticalSection()
  {
    Thor::LLD::IT::disableIRQ( Resource::IRQSignals[ mResourceIndex ] );
  }


  inline void Driver::exitCriticalSection()
  {
    Thor::LLD::IT::enableIRQ( Resource::IRQSignals[ mResourceIndex ] );
  }


  void Driver::IRQHandler()
  {

  }
}    // namespace Thor::LLD::WWDG


#if defined( STM32_WWDG1_PERIPH_AVAILABLE )
void WWDG1_IRQHandler()
{
  using namespace Thor::LLD::WWDG;
  s_wwdg_drivers[ WWDG1_RESOURCE_INDEX ].IRQHandler();
}
#endif

#endif /* TARGET_STM32L4 && THOR_DRIVER_WWDG */

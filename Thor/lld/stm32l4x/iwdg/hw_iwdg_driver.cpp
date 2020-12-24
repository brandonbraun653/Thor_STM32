/********************************************************************************
 *  File Name:
 *    hw_iwdg_driver.cpp
 *
 *  Description:
 *    Implements the LLD interface to the IWDG hardware.
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
#include <Thor/lld/interface/watchdog/watchdog_prv_data.hpp>
#include <Thor/lld/interface/watchdog/watchdog_intf.hpp>
#include <Thor/lld/interface/watchdog/watchdog_types.hpp>
#include <Thor/lld/stm32l4x/iwdg/hw_iwdg_prj.hpp>
#include <Thor/lld/stm32l4x/iwdg/hw_iwdg_types.hpp>
#include <Thor/lld/stm32l4x/rcc/hw_rcc_driver.hpp>

#if defined( TARGET_STM32L4 ) && defined( THOR_LLD_IWDG )

namespace Thor::LLD::Watchdog
{
  /*-------------------------------------------------------------------------------
  Variables
  -------------------------------------------------------------------------------*/
  static IndependentDriver s_iwdg_drivers[ IWDG::NUM_IWDG_PERIPHS ];


  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  Chimera::Status_t initializeIWDG()
  {
    /*-------------------------------------------------
    Attach all the expected mPeripherals to the drivers
    -------------------------------------------------*/
    if ( attachDriverInstances( s_iwdg_drivers, ARRAY_COUNT( s_iwdg_drivers ) ) )
    {
      return Chimera::Status::OK;
    }
    else
    {
      return Chimera::Status::FAIL;
    }
  }


  IndependentDriver_rPtr getDriver( const Chimera::Watchdog::IChannel channel )
  {
    if ( auto idx = getResourceIndex( channel ); idx != INVALID_RESOURCE_INDEX )
    {
      return &s_iwdg_drivers[ idx ];
    }
    else
    {
      return nullptr;
    }
  }


  /*-------------------------------------------------------------------------------
  Low Level Driver Implementation
  -------------------------------------------------------------------------------*/
  IndependentDriver::IndependentDriver()
  {
  }


  IndependentDriver::~IndependentDriver()
  {
  }


  Chimera::Status_t IndependentDriver::attach( IRegisterMap *const mPeripheral )
  {
    /*------------------------------------------------
    Get mPeripheral descriptor settings
    ------------------------------------------------*/
    mPeriph        = mPeripheral;
    mResourceIndex = getResourceIndex( reinterpret_cast<std::uintptr_t>( mPeripheral ) );

    return Chimera::Status::OK;
  }


  void IndependentDriver::enableClock()
  {
    auto rcc = Thor::LLD::RCC::getCoreClock();
    rcc->enableClock( Chimera::Clock::Bus::LSI );
  }


  Chimera::Status_t IndependentDriver::setPrescaler( const Reg32_t val )
  {
    /*------------------------------------------------
    Wait for SR to indicate no ongoing HW updates.
    This can take at most 5 LSI clocks.
    ------------------------------------------------*/
    while ( IWDG::PVU::get( mPeriph ) )
    {
      continue;
    }

    /*------------------------------------------------
    Assign the register value with unlock-assign-lock
    ------------------------------------------------*/
    IWDG::KEY::set( mPeriph, IWDG::KR_UNLOCK );
    IWDG::PR::set( mPeriph, val );
    IWDG::KEY::set( mPeriph, IWDG::KR_LOCK );

    /*------------------------------------------------
    Wait again...
    ------------------------------------------------*/
    while ( IWDG::PVU::get( mPeriph ) )
    {
      continue;
    }

    return Chimera::Status::OK;
  }


  Chimera::Status_t IndependentDriver::setReload( const Reg32_t val )
  {
    /*------------------------------------------------
    Wait for SR to indicate no ongoing HW updates.
    This can take at most 125uS (5 clocks @ 40kHz LSI).
    ------------------------------------------------*/
    while ( IWDG::RVU::get( mPeriph ) )
    {
      continue;
    }

    /*------------------------------------------------
    Assign the register value with unlock-assign-lock
    ------------------------------------------------*/
    IWDG::KEY::set( mPeriph, IWDG::KR_UNLOCK );
    IWDG::RL::set( mPeriph, val );
    IWDG::KEY::set( mPeriph, IWDG::KR_LOCK );

    /*------------------------------------------------
    Wait again...
    ------------------------------------------------*/
    while ( IWDG::RVU::get( mPeriph ) )
    {
      continue;
    }

    return Chimera::Status::OK;
  }


  void IndependentDriver::start()
  {
    IWDG::KEY::set( mPeriph, IWDG::KR_START );
  }


  void IndependentDriver::reload()
  {
    IWDG::KEY::set( mPeriph, IWDG::KR_REFRESH );
  }


  size_t IndependentDriver::getMaxTimeout( const Reg32_t prescaler )
  {
    size_t base_period_ms = static_cast<size_t>( 1000.0f / static_cast<float>( IWDG::PERIPH_CLOCK_FREQ_HZ ) );
    size_t actual_period_ms = base_period_ms * prescaler;

    return actual_period_ms * ( IWDG::COUNTER_MAX - IWDG::COUNTER_MIN );
  }


  size_t IndependentDriver::getMinTimeout( const Reg32_t prescaler )
  {
    size_t base_period_ms = static_cast<size_t>( 1000.0f / static_cast<float>( IWDG::PERIPH_CLOCK_FREQ_HZ ) );
    size_t actual_period_ms = base_period_ms * prescaler;

    return actual_period_ms * ( IWDG::COUNTER_MIN + 1 );
  }


  size_t IndependentDriver::getTimeout()
  {
    size_t prescaler = IWDG::PR::get( mPeriph ) >> IWDG::PR_PR_Pos;
    size_t reloadVal = IWDG::RL::get( mPeriph ) >> IWDG::RLR_RL_Pos;

    size_t base_period_ms = static_cast<size_t>( 1000.0f / static_cast<float>( IWDG::PERIPH_CLOCK_FREQ_HZ ) );
    size_t actual_period_ms = base_period_ms * prescaler;

    return actual_period_ms * ( reloadVal - IWDG::COUNTER_MIN );
  }

}    // namespace Thor::LLD::Watchdog


#endif /* TARGET_STM32L4 && THOR_DRIVER_IWDG */

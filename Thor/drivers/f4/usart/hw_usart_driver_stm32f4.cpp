/********************************************************************************
 *   File Name:
 *    hw_usart_driver_stm32f4.cpp
 *
 *   Description:
 *    STM32F4 specific driver implementation for the UART/USART driver. Both drivers
 *    are merged into one as the datasheet does not make a distinction between the
 *    two. In practice with the STM32HAL this was also found to be true.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C++ Includes */

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/f4/rcc/hw_rcc_driver.hpp>
#include <Thor/drivers/f4/usart/hw_usart_driver.hpp>
#include <Thor/drivers/f4/usart/hw_usart_mapping.hpp>
#include <Thor/drivers/f4/usart/hw_usart_prj.hpp>
#include <Thor/drivers/f4/usart/hw_usart_types.hpp>




#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_USART == 1 )

namespace Thor::Driver::USART
{
  /**
   *  Number of elements to allocate by default for any vectors used in the Driver.
   *
   *  The goal is to size this such that a dynamic allocation isn't likely to occur
   *  unless there suddenly are a lot of parties interested in being notified that
   *  events are occuring.
   */
  static constexpr size_t DFLT_VECTOR_SIZE = 5;

  static inline uint32_t deriveBaudRateConfig( const uint32_t desired );

  bool isUSART( const std::uintptr_t address )
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

  Driver::Driver( RegisterMap *const peripheral ) :
      periph( peripheral ), rxCompleteListeners( DFLT_VECTOR_SIZE, nullptr ), txCompleteListeners( DFLT_VECTOR_SIZE, nullptr )
  {
    auto address = reinterpret_cast<std::uintptr_t>( peripheral );
    peripheralType = Chimera::Peripheral::Type::USART;
    resourceIndex  = Thor::Driver::USART::InstanceToResourceIndex.find( address )->second;
  }

  Driver::~Driver()
  {
  }

  Chimera::Status_t Driver::init( const Thor::Driver::Serial::Config &cfg )
  {
    /*------------------------------------------------
    First de-initialize the driver so we know we are
    starting from a clean slate. There are no guarantees
    on what state the system is in when this is called.
    ------------------------------------------------*/
    if ( deinit() != Chimera::CommonStatusCodes::OK )
    {
      return Chimera::CommonStatusCodes::FAIL;
    }

    /*------------------------------------------------
    Ensure the clock is enabled otherwise the hardware is "dead"
    ------------------------------------------------*/
    auto rccPeriph = Thor::Driver::RCC::PeripheralController::get();
    rccPeriph->enableClock( peripheralType, resourceIndex );

    /*------------------------------------------------
    Follow the initialization sequence as defined in RM0390 pg.801 
    ------------------------------------------------*/
    /* Enable the USART by writing the UE bit to 1 */
    CR1::UE::set( periph, CR1_UE );

    /* Program the M bit to define the word length */
    CR1::M::set( periph, cfg.WordLength );

    /* Program the number of stop bits */
    CR2::STOP::set( periph, cfg.StopBits );

    /* Select the desired baud rate */
    BRR::set( periph, deriveBaudRateConfig( cfg.BaudRate ) );

    /* Set the TE bit to send an idle frame as first transmission */
    CR1::TE::set( periph, CR1_TE );


    return Chimera::CommonStatusCodes::OK;
  }

  Chimera::Status_t Driver::deinit()
  {
    auto rcc = Thor::Driver::RCC::PeripheralController::get();
    rcc->enableClock( peripheralType, resourceIndex );
    rcc->reset( peripheralType, resourceIndex );
    rcc->disableClock( peripheralType, resourceIndex );

    return Chimera::CommonStatusCodes::OK;
  }

  Chimera::Status_t Driver::reset()
  {
    /*------------------------------------------------
    Reset the hardware registers
    ------------------------------------------------*/
    deinit();

    /*------------------------------------------------
    Erases pointers to the listeners, not the listeners themselves
    ------------------------------------------------*/
    rxCompleteListeners.clear();
    txCompleteListeners.clear();

    return Chimera::CommonStatusCodes::OK;
  }

  Chimera::Status_t Driver::transmit( const uint8_t *const data, const size_t size, const size_t timeout )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::receive( uint8_t *const data, const size_t size, const size_t timeout )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::enableIT( const Chimera::Hardware::SubPeripheral periph )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::disableIT( const Chimera::Hardware::SubPeripheral periph )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::transmitIT( uint8_t *const data, const size_t size, const size_t timeout )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::receiveIT( uint8_t *const data, const size_t size, const size_t timeout )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::initDMA()
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::deinitDMA()
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::enableDMA_IT( const Chimera::Hardware::SubPeripheral periph )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::disableDMA_IT( const Chimera::Hardware::SubPeripheral periph )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::transmitDMA( uint8_t *const data, const size_t size, const size_t timeout )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::receiveDMA( uint8_t *const data, const size_t size, const size_t timeout )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::enableSignal( const InterruptSignal_t sig )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::disableSignal( const InterruptSignal_t sig )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::registerEventListener( const Chimera::Event::Trigger event, SemaphoreHandle_t *const listener )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Status_t Driver::removeEventListener( const Chimera::Event::Trigger event, SemaphoreHandle_t *const listener )
  {
    return Chimera::CommonStatusCodes::NOT_SUPPORTED;
  }

  Chimera::Hardware::Status Driver::pollTransferStatus()
  {
    return Chimera::Hardware::Status::PERIPHERAL_FREE;
  }


  static inline uint32_t calculateBRR( const uint32_t pclk, const uint32_t baud )
  {
    /*------------------------------------------------
    Taken directly from the STM32 HAL Macros
    ------------------------------------------------*/
    auto divisor          = ( 25u * pclk ) / ( 2u * baud );
    auto mantissa_divisor = divisor / 100u;
    auto fraction_divisor = ( ( divisor - ( mantissa_divisor * 100u ) ) * 16u + 50u ) / 100u;
    auto brr_value        = ( mantissa_divisor << BRR_DIV_Mantissa_Pos ) | ( fraction_divisor & BRR_DIV_Fraction );

    return brr_value;
  }

  static inline uint32_t deriveBaudRateConfig( const uint32_t desired )
  {
    auto rccSys      = Thor::Driver::RCC::SystemClock::get();
    auto periphClock = rccSys->getPeriphClock( Chimera::Peripheral::Type::USART );
    auto configVal   = calculateBRR( periphClock, desired );

    return configVal;
  }



}    // namespace Thor::Driver::USART

#endif /* TARGET_STM32F4 && THOR_DRIVER_USART */
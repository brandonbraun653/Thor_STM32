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

/* Chimera Includes */
#include <Chimera/chimera.hpp>

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/f4/rcc/hw_rcc_driver.hpp>
#include <Thor/drivers/f4/nvic/hw_nvic_driver.hpp>
#include <Thor/drivers/f4/usart/hw_usart_driver.hpp>
#include <Thor/drivers/f4/usart/hw_usart_mapping.hpp>
#include <Thor/drivers/f4/usart/hw_usart_prj.hpp>
#include <Thor/drivers/f4/usart/hw_usart_types.hpp>


#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_USART == 1 )


static std::array<Thor::Driver::USART::Driver *, Thor::Driver::USART::NUM_USART_PERIPHS> usartObjects;

namespace Thor::Driver::USART
{

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

  Driver::Driver( RegisterMap *const peripheral ) : periph( peripheral )
  {
    auto address   = reinterpret_cast<std::uintptr_t>( peripheral );
    peripheralType = Chimera::Peripheral::Type::PERIPH_USART;
    resourceIndex  = Thor::Driver::USART::InstanceToResourceIndex.find( address )->second;

    periphIRQn = USART_IRQn[ resourceIndex ];

    usartObjects[ resourceIndex ] = this;
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
    Initialize driver memory
    ------------------------------------------------*/
    enterCriticalSection();

    txTCB.reset();
    rxTCB.reset();

    rxCompleteActors.clear();
    onTXComplete.clear();
    onError.clear();

    exitCriticalSection();

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
    BRR::set( periph, calculateBRR( cfg.BaudRate ) );

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
    rxCompleteActors.clear();
    onTXComplete.clear();

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

  Chimera::Status_t Driver::enableIT( const Chimera::Hardware::SubPeripheral subPeriph )
  {
    /*------------------------------------------------
    The individual TX and RX functions take care of the nitty gritty details
    ------------------------------------------------*/
    Thor::Driver::Interrupt::enableIRQ( periphIRQn );
    return Chimera::CommonStatusCodes::OK;
  }

  Chimera::Status_t Driver::disableIT( const Chimera::Hardware::SubPeripheral subPeriph )
  {
    using namespace Chimera::Hardware;
    using namespace Thor::Driver::Interrupt;

    Chimera::Status_t result = Chimera::CommonStatusCodes::OK;

    disableIRQ( periphIRQn );
    clearPendingIRQ( periphIRQn );

    if ( ( subPeriph == SubPeripheral::TX ) || ( subPeriph == SubPeripheral::TXRX ) )
    {
      CR1::TCIE::set( periph, 0 );
      CR1::TXEIE::set( periph, 0 );
    }
    else if ( ( subPeriph == SubPeripheral::RX ) || ( subPeriph == SubPeripheral::TXRX ) )
    {

    }
    else
    {
      result = Chimera::CommonStatusCodes::INVAL_FUNC_PARAM;
    }

    enableIRQ( periphIRQn );

    return result;
  }

  Chimera::Status_t Driver::transmitIT( uint8_t *const data, const size_t size, const size_t timeout )
  {
#if defined( DEBUG )
    if ( !data || !size )
    {
      return Chimera::CommonStatusCodes::INVAL_FUNC_PARAM;
    }
#endif

    if ( !SR::TC::get( periph ) )
    {
      return Chimera::CommonStatusCodes::BUSY;
    }
    else
    {
      enterCriticalSection();
      
      /*------------------------------------------------
      Turn on the transmitter
      ------------------------------------------------*/
      CR1::TE::set( periph, CR1_TE );

      /*------------------------------------------------
      Turn on the Transmit Data register interrupt so we
      know when we can stage the next byte transfer.
      ------------------------------------------------*/
      CR1::TXEIE::set( periph, CR1_TXEIE );

      /*------------------------------------------------
      Prep the transfer control block
      ------------------------------------------------*/
      txTCB.buffer = &data[ 1 ]; /* Point to the next byte */
      txTCB.size   = size - 1u;  /* Pre-decrement to account for this first byte */
      txTCB.state  = StateMachine::TX::IT_TX_ONGOING;

      /*------------------------------------------------
      Write the byte onto the wire
      ------------------------------------------------*/
      periph->DR = data[ 0 ];

      exitCriticalSection();
    }

    return Chimera::CommonStatusCodes::OK;
  }

  Chimera::Status_t Driver::receiveIT( uint8_t *const data, const size_t size, const size_t timeout )
  {
#if defined( DEBUG )
    if ( !data || !size )
    {
      return Chimera::CommonStatusCodes::INVAL_FUNC_PARAM;
    }
#endif

    if ( SR::RXNE::get( periph ) )
    {
      /* There is a byte in the register that needs to be read by the ISR */
      return Chimera::CommonStatusCodes::BUSY;
    }
    else
    {
      enterCriticalSection();

      // do something with interrupt bits

      /*------------------------------------------------
      Prep the transfer control block to receive data
      ------------------------------------------------*/
      rxTCB.buffer = data;
      rxTCB.size   = size;
      rxTCB.state  = StateMachine::RX::IT_RX_ONGOING;

      // turn on listening

      exitCriticalSection();
    }

    return Chimera::CommonStatusCodes::OK;
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

  void Driver::IRQHandler()
  {
    using namespace Configuration::Flags;

    /*------------------------------------------------
    Following the datasheet, flags are only cleared after
    reading the SR and then DR.
    ------------------------------------------------*/
    const uint32_t statusRegister = SR::get( periph );
    const uint32_t dataRegister   = DR::get( periph );

    /*------------------------------------------------
    Cache the current state of the control registers
    ------------------------------------------------*/
    const uint32_t CR1 = CR1::get( periph );
    const uint32_t CR2 = CR2::get( periph );
    const uint32_t CR3 = CR3::get( periph );

    /*------------------------------------------------
    Figure out which interrupt flags have been set
    ------------------------------------------------*/
    const uint32_t txFlags    = statusRegister & ( FLAG_TC | FLAG_TXE );
    const uint32_t rxFlags    = statusRegister & ( FLAG_RXNE | FLAG_IDLE );
    const uint32_t errorFlags = statusRegister & ( FLAG_ORE | FLAG_PE | FLAG_NF | FLAG_FE );

    /*------------------------------------------------
    TX Related Handler
    ------------------------------------------------*/
    if ( txFlags )
    {
      /*------------------------------------------------
      TDR Empty Interrupt
      ------------------------------------------------*/
      if ( ( txFlags & FLAG_TXE ) && ( CR1 & CR1_TXEIE ) && ( txTCB.state == StateMachine::TX::IT_TX_ONGOING ) )
      {
        if ( txTCB.size )
        {
          /*------------------------------------------------
          Make sure the TX complete interrupt cannot fire
          ------------------------------------------------*/
          CR1::TCIE::set( periph, 0 );

          /*------------------------------------------------
          Transfer the byte and prep for the next character
          ------------------------------------------------*/
          periph->DR = *txTCB.buffer;

          txTCB.buffer++;
          txTCB.size--;
        }
        else
        {
          /*------------------------------------------------
          We finished pushing the last character into the TDR, so 
          now listen for the TX complete interrupt.
          ------------------------------------------------*/
          CR1::TXEIE::set( periph, 0 );
          CR1::TCIE::set( periph, CR1_TCIE );
          txTCB.state = StateMachine::IT_TX_COMPLETE;
        }
      }

      /*------------------------------------------------
      Transfer Complete Interrupt
      ------------------------------------------------*/
      if ( ( txFlags & FLAG_TC ) && ( CR1 & CR1_TCIE ) && ( txTCB.state == StateMachine::TX::IT_TX_COMPLETE ) ) 
      {
        onTXComplete.notifyAtomic( Chimera::Event::Trigger::WRITE_COMPLETE );
        onTXComplete.notifyThreaded();
        onTXComplete.executeCallbacks( nullptr );

        /*------------------------------------------------
        Exit the TX ISR cleanly by disabling related interrupts
        ------------------------------------------------*/
        CR1::TCIE::set( periph, 0 );
        CR1::TXEIE::set( periph, 0 );
        CR1::TE::set( periph, 0 );
        txTCB.state = StateMachine::TX::IT_TX_READY;
      }
    }

    /*------------------------------------------------
    RX Related Handler
    ------------------------------------------------*/
    if ( rxFlags )
    {

    // Handle RX related stuff
    //  Single reception character by character, up to a certain size
    //  Need a transfer control block structure that is volatile/protected when accessed user side.
    }

    /*------------------------------------------------
    Error Related Handler
    ------------------------------------------------*/
    if ( errorFlags )
    {
      ISRErrorFlags |= errorFlags;

      onError.notifyAtomic( Chimera::Event::Trigger::ERROR );
      onError.notifyThreaded();
      onError.executeCallbacks( reinterpret_cast<const void *const>( &errorFlags ) );

      /* All error bits are cleared by read to SR then DR, which was already performed */
    }
  }

  bool Driver::waitUntilSet( const uint32_t flag, const size_t timeout )
  {
    uint32_t srVal   = SR::get( periph );
    size_t startTime = Chimera::millis();

    while ( !( srVal & flag ) )
    {
      srVal = SR::get( periph );

      if ( ( Chimera::millis() - startTime ) > timeout )
      {
        return true;
      }
    } 

    return false;
  }

  uint32_t Driver::calculateBRR( const size_t desiredBaud )
  {
    size_t periphClock = 0u;
    size_t calculatedBRR = 0u;
    auto periphAddress = reinterpret_cast<std::uintptr_t>( periph );

    /*------------------------------------------------
    Figure out the frequency of the clock that drives the USART
    ------------------------------------------------*/
    auto rccSys = Thor::Driver::RCC::SystemClock::get();
    rccSys->getPeriphClock( Chimera::Peripheral::Type::PERIPH_USART, periphAddress, &periphClock );

    /*------------------------------------------------
    Protect from fault conditions in the math below
    ------------------------------------------------*/
    if ( !desiredBaud || !periphClock )
    {
      return 0u;
    }

    /*------------------------------------------------
    Calculate the BRR value. Mostly this was taken directly from
    the STM32 HAL Macros.
    ------------------------------------------------*/
    uint32_t over8Compensator = 2u;

    if ( CR1::OVER8::get( periph ) )
    {
      over8Compensator = 1u;
    }

    auto divisor          = ( 25u * periphClock ) / ( 2u * over8Compensator * desiredBaud );
    auto mantissa_divisor = divisor / 100u;
    auto fraction_divisor = ( ( divisor - ( mantissa_divisor * 100u ) ) * 16u + 50u ) / 100u;
    calculatedBRR         = ( mantissa_divisor << BRR_DIV_Mantissa_Pos ) | ( fraction_divisor & BRR_DIV_Fraction );

    return calculatedBRR;
  }

  void Driver::enterCriticalSection()
  {
    Thor::Driver::Interrupt::disableIRQ( periphIRQn );
  }

  void Driver::exitCriticalSection()
  {
    Thor::Driver::Interrupt::enableIRQ( periphIRQn );
  }

}    // namespace Thor::Driver::USART

void USART1_IRQHandler( void )
{
  static constexpr size_t index = 0;

  if ( usartObjects[ index ] )
  {
    usartObjects[ index ]->IRQHandler();
  }
}

void USART2_IRQHandler( void )
{
  static constexpr size_t index = 1;

  if ( usartObjects[ index ] )
  {
    usartObjects[ index ]->IRQHandler();
  }
}

void USART3_IRQHandler( void )
{
  static constexpr size_t index = 2;

  if ( usartObjects[ index ] )
  {
    usartObjects[ index ]->IRQHandler();
  }
}

void USART6_IRQHandler( void )
{
  static constexpr size_t index = 3;

  if ( usartObjects[ index ] )
  {
    usartObjects[ index ]->IRQHandler();
  }
}

#endif /* TARGET_STM32F4 && THOR_DRIVER_USART */
/********************************************************************************
 *  File Name:
 *    hld_uart_chimera.cpp
 *
 *	 Description:
 *    Implementation of Chimera UART driver hooks
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* STL Includes */
#include <memory>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/uart>

/* Thor Includes */
#include <Thor/cfg>
#include <Thor/uart>
#include <Thor/lld/interface/uart/uart_intf.hpp>
#include <Thor/lld/interface/uart/uart_detail.hpp>

/*-------------------------------------------------------------------------------
Aliases
-------------------------------------------------------------------------------*/
namespace HLD = ::Thor::UART;
namespace LLD = ::Thor::LLD::UART;

/*-------------------------------------------------------------------------------
Constants
-------------------------------------------------------------------------------*/
static constexpr size_t NUM_DRIVERS = 1; //::LLD::NUM_UART_PERIPHS;
#pragma warning( "missing hld driver hooks for uart")

/*-------------------------------------------------------------------------------
Variables
-------------------------------------------------------------------------------*/
static Chimera::UART::Driver s_raw_driver[ NUM_DRIVERS ];

namespace Chimera::UART::Backend
{
  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  Chimera::Status_t initialize()
  {
    return Thor::UART::initialize();
  }


  Chimera::Status_t reset()
  {
    return Thor::UART::reset();
  }


  bool isChannelUART( const Chimera::Serial::Channel channel )
  {
    // return Thor::LLD::UART::isSupported( channel );
    return false;
  }


  Driver_rPtr getDriver( const Chimera::Serial::Channel channel )
  {
    // auto resourceIndex = ::LLD::getResourceIndex( channel );
    // if ( resourceIndex != ::Thor::LLD::INVALID_RESOURCE_INDEX )
    // {
    //   return s_shared_driver[ resourceIndex ];
    // }
    // else
    // {
    //   return nullptr;
    // }

    return nullptr;
  }


  Chimera::Status_t registerDriver( Chimera::UART::Backend::DriverConfig &registry )
  {
// #if defined( THOR_HLD_UART )
//     registry.isSupported   = true;
//     registry.getDriver     = getDriver;
//     registry.initialize    = initialize;
//     registry.reset         = reset;
//     registry.isChannelUART = isChannelUART;
//     return Chimera::Status::OK;
// #else
    registry.isSupported   = false;
    registry.getDriver     = nullptr;
    registry.initialize    = nullptr;
    registry.reset         = nullptr;
    registry.isChannelUART = nullptr;
    return Chimera::Status::NOT_SUPPORTED;
//#endif /* THOR_HLD_UART */
  }
}    // namespace Chimera::UART::Backend


namespace Chimera::UART
{
  /*-------------------------------------------------------------------------------
  Driver Implementation
  -------------------------------------------------------------------------------*/
  Driver::Driver() : mChannel( Chimera::Serial::Channel::NOT_SUPPORTED )
  {
  }

  Driver::~Driver()
  {
  }

  /*-------------------------------------------------
  Interface: Hardware
  -------------------------------------------------*/
  Chimera::Status_t Driver::assignHW( const Chimera::Serial::Channel channel, const Chimera::Serial::IOPins &pins )
  {
    mChannel = channel;
    return ::HLD::getDriver( mChannel )->assignHW( channel, pins );
  }


  Chimera::Status_t Driver::begin( const Chimera::Hardware::PeripheralMode txMode,
                                   const Chimera::Hardware::PeripheralMode rxMode )
  {
    return ::HLD::getDriver( mChannel )->begin( txMode, rxMode );
  }


  Chimera::Status_t Driver::end()
  {
    return ::HLD::getDriver( mChannel )->end();
  }


  Chimera::Status_t Driver::configure( const Chimera::Serial::Config &config )
  {
    return ::HLD::getDriver( mChannel )->configure( config );
  }


  Chimera::Status_t Driver::setBaud( const uint32_t baud )
  {
    return ::HLD::getDriver( mChannel )->setBaud( baud );
  }


  Chimera::Status_t Driver::setMode( const Chimera::Hardware::SubPeripheral periph,
                                     const Chimera::Hardware::PeripheralMode mode )
  {
    return ::HLD::getDriver( mChannel )->setMode( periph, mode );
  }


  Chimera::Status_t Driver::write( const void *const buffer, const size_t length )
  {
    return ::HLD::getDriver( mChannel )->write( buffer, length );
  }


  Chimera::Status_t Driver::read( void *const buffer, const size_t length )
  {
    return ::HLD::getDriver( mChannel )->read( buffer, length );
  }


  Chimera::Status_t Driver::flush( const Chimera::Hardware::SubPeripheral periph )
  {
    return ::HLD::getDriver( mChannel )->flush( periph );
  }


  Chimera::Status_t Driver::toggleAsyncListening( const bool state )
  {
    return ::HLD::getDriver( mChannel )->toggleAsyncListening( state );
  }


  Chimera::Status_t Driver::readAsync( uint8_t *const buffer, const size_t len )
  {
    return ::HLD::getDriver( mChannel )->readAsync( buffer, len );
  }


  Chimera::Status_t Driver::enableBuffering( const Chimera::Hardware::SubPeripheral periph,
                                             boost::circular_buffer<uint8_t> *const userBuffer, uint8_t *const hwBuffer,
                                             const size_t hwBufferSize )
  {
    return ::HLD::getDriver( mChannel )->enableBuffering( periph, userBuffer, hwBuffer, hwBufferSize );
  }

  Chimera::Status_t Driver::disableBuffering( const Chimera::Hardware::SubPeripheral periph )
  {
    return ::HLD::getDriver( mChannel )->disableBuffering( periph );
  }


  bool Driver::available( size_t *const bytes )
  {
    return ::HLD::getDriver( mChannel )->available( bytes );
  }


  void Driver::postISRProcessing()
  {
    ::HLD::getDriver( mChannel )->postISRProcessing();
  }


  /*-------------------------------------------------
  Interface: Listener
  -------------------------------------------------*/
  Chimera::Status_t Driver::registerListener( Chimera::Event::Actionable &listener, const size_t timeout,
                                              size_t &registrationID )
  {
    return ::HLD::getDriver( mChannel )->registerListener( listener, timeout, registrationID );
  }


  Chimera::Status_t Driver::removeListener( const size_t registrationID, const size_t timeout )
  {
    return ::HLD::getDriver( mChannel )->removeListener( registrationID, timeout );
  }


  /*-------------------------------------------------
  Interface: AsyncIO
  -------------------------------------------------*/
  Chimera::Status_t Driver::await( const Chimera::Event::Trigger event, const size_t timeout )
  {
    return ::HLD::getDriver( mChannel )->await( event, timeout );
  }


  Chimera::Status_t Driver::await( const Chimera::Event::Trigger event, Chimera::Threading::BinarySemaphore &notifier,
                                   const size_t timeout )
  {
    return ::HLD::getDriver( mChannel )->await( event, notifier, timeout );
  }

  /*-------------------------------------------------
  Interface: Lockable
  -------------------------------------------------*/
  void Driver::lock()
  {
    ::HLD::getDriver( mChannel )->lock();
  }

  void Driver::lockFromISR()
  {
    ::HLD::getDriver( mChannel )->lockFromISR();
  }

  bool Driver::try_lock_for( const size_t timeout )
  {
    return ::HLD::getDriver( mChannel )->try_lock_for( timeout );
  }

  void Driver::unlock()
  {
    ::HLD::getDriver( mChannel )->unlock();
  }

  void Driver::unlockFromISR()
  {
    ::HLD::getDriver( mChannel )->unlockFromISR();
  }

}    // namespace Chimera::UART
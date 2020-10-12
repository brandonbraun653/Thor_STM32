/********************************************************************************
 *  File Name:
 *    hld_can_driver.cpp
 *
 *  Description:
 *    CAN driver for Thor
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C++ Includes */
#include <array>
#include <cstring>
#include <limits>

/* Aurora Includes */
#include <Aurora/constants/common.hpp>

/* Chimera Includes */
#include <Chimera/can>
#include <Chimera/thread>
#include <Chimera/event>

/* Thor Includes */
#include <Thor/cfg>
#include <Thor/can>
#include <Thor/lld/interface/can/can_detail.hpp>
#include <Thor/lld/interface/can/can_intf.hpp>
#include <Thor/lld/interface/can/can_types.hpp>

#if defined( THOR_HLD_CAN )

/*-------------------------------------------------------------------------------
Aliases
-------------------------------------------------------------------------------*/
namespace HLD = ::Thor::CAN;
namespace LLD = ::Thor::LLD::CAN;

using ThreadHandle = Chimera::Threading::detail::native_thread_handle_type;
using BinarySemphr = Chimera::Threading::BinarySemaphore;
using ThreadFunctn = Chimera::Function::void_func_void_ptr;

/*-------------------------------------------------------------------------------
Constants
-------------------------------------------------------------------------------*/
static constexpr size_t NUM_DRIVERS = LLD::NUM_CAN_PERIPHS;
static constexpr size_t NUM_ISR_SIG = LLD::NUM_CAN_IRQ_HANDLERS;

/*-------------------------------------------------------------------------------
Variables
-------------------------------------------------------------------------------*/
static size_t s_driver_initialized;

/*-------------------------------------------------
Instances of the CAN driver in object and ptr form
-------------------------------------------------*/
static HLD::Driver hld_driver[ NUM_DRIVERS ];
static HLD::Driver_sPtr hld_shared[ NUM_DRIVERS ];

/*-------------------------------------------------
High priority threads & handles that process more
complex ISR functionality.
-------------------------------------------------*/
static ThreadHandle s_user_isr_handle[ NUM_DRIVERS ][ NUM_ISR_SIG ];
static ThreadFunctn s_user_isr_thread_func[ NUM_DRIVERS ][ NUM_ISR_SIG ];

/*-------------------------------------------------
Wakeup signals used to allow external threads to
pend on interrupt processing events.
-------------------------------------------------*/
static BinarySemphr s_user_isr_signal[ NUM_DRIVERS ][ NUM_ISR_SIG ];

/*-------------------------------------------------------------------------------
Private Function Declarations
-------------------------------------------------------------------------------*/
#if defined( STM32_CAN1_PERIPH_AVAILABLE )
static void CAN1ISR_TXHandler( void *argument );
static void CAN1ISR_RXHandler( void *argument );
static void CAN1ISR_StatusChangeHandler( void *argument );
static void CAN1ISR_ErrHandler( void *argument );
#endif

namespace Thor::CAN
{
  using namespace Chimera::Threading;

  /*-------------------------------------------------------------------------------
  Static Functions
  -------------------------------------------------------------------------------*/
  static bool validateCfg( const Chimera::CAN::DriverConfig &cfg )
  {
    /*-------------------------------------------------
    At a bare minimum, if the user didn't set the actual
    validity flag, it's going to mark all these checks
    as invalid.
    -------------------------------------------------*/
    bool result = cfg.validity;

    /*-------------------------------------------------
    User needs to validate the pin configs too
    -------------------------------------------------*/
    result |= cfg.RXInit.validity;
    result |= cfg.TXInit.validity;

    /*-------------------------------------------------
    The buffers must be pre-allocated as these drivers
    do not use dynamic memory. If dynamic memory is
    desired, it must be allocated externally.
    -------------------------------------------------*/
    result |= ( cfg.HWInit.rxBuffer != nullptr );
    result |= ( cfg.HWInit.txBuffer != nullptr );
    result |= ( cfg.HWInit.rxElements != 0 );
    result |= ( cfg.HWInit.txElements != 0 );

    /*-------------------------------------------------
    Last but not least, does the channel even exist?
    -------------------------------------------------*/
    result |= ( cfg.HWInit.channel < Chimera::CAN::Channel::NUM_OPTIONS );

    return result;
  }

  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  Chimera::Status_t initialize()
  {
    /*------------------------------------------------
    Prevent multiple initializations (need reset first)
    ------------------------------------------------*/
    if ( s_driver_initialized == Chimera::DRIVER_INITIALIZED_KEY )
    {
      return Chimera::Status::OK;
    }

    /*------------------------------------------------
    Initialize the low level driver
    ------------------------------------------------*/
    ::LLD::initialize();

    /*------------------------------------------------
    Initialize ISR post-processing routines
    ------------------------------------------------*/
#if defined( STM32_CAN1_PERIPH_AVAILABLE )
    s_user_isr_thread_func[::LLD::CAN1_RESOURCE_INDEX ][ ::LLD::CAN_TX_ISR_SIGNAL_INDEX ] = CAN1ISR_TXHandler;
    s_user_isr_thread_func[::LLD::CAN1_RESOURCE_INDEX ][ ::LLD::CAN_RX_ISR_SIGNAL_INDEX ] = CAN1ISR_RXHandler;
    s_user_isr_thread_func[::LLD::CAN1_RESOURCE_INDEX ][ ::LLD::CAN_STS_ISR_SIGNAL_INDEX ] = CAN1ISR_StatusChangeHandler;
    s_user_isr_thread_func[::LLD::CAN1_RESOURCE_INDEX ][ ::LLD::CAN_ERR_ISR_SIGNAL_INDEX ] = CAN1ISR_ErrHandler;
#endif

    /*-------------------------------------------------
    Initialize shared references to drivers
    -------------------------------------------------*/
    for ( size_t x = 0; x < NUM_DRIVERS; x++ )
    {
#if defined( THOR_HLD_TEST ) || defined( THOR_HLD_TEST_CAN )
      hld_shared[ x ] = ::HLD::Driver_sPtr( new ::HLD::Driver() );
#else
      hld_shared[ x ] = ::HLD::Driver_sPtr( &hld_driver[ x ] );
#endif
    }

    /*-------------------------------------------------
    Lock the init sequence and exit
    -------------------------------------------------*/
    s_driver_initialized = Chimera::DRIVER_INITIALIZED_KEY;
    return Chimera::Status::OK;
  }


  Chimera::Status_t reset()
  {
    /*------------------------------------------------
    Only allow clearing of local data during testing
    ------------------------------------------------*/
#if defined( THOR_HLD_TEST ) || defined( THOR_HLD_TEST_CAN )
    s_driver_initialized = ~Chimera::DRIVER_INITIALIZED_KEY;

    for ( auto x = 0; x < NUM_DRIVERS; x++ )
    {
      hld_shared[ x ].reset();
    }
#endif

    return Chimera::Status::OK;
  }


  Driver_rPtr getDriver( const Chimera::CAN::Channel channel )
  {
    if ( auto idx = ::LLD::getResourceIndex( channel ); idx != ::Thor::LLD::INVALID_RESOURCE_INDEX )
    {
      return &hld_driver[ idx ];
    }
    else
    {
      return nullptr;
    }
  }


  Driver_sPtr getDriverShared( const Chimera::CAN::Channel channel )
  {
    if ( auto idx = ::LLD::getResourceIndex( channel ); idx != ::Thor::LLD::INVALID_RESOURCE_INDEX )
    {
      return hld_shared[ idx ];
    }
    else
    {
      return nullptr;
    }
  }


  /*-------------------------------------------------------------------------------
  Driver Implementation
  -------------------------------------------------------------------------------*/
  Driver::Driver()
  {
    mConfig.clear();
  }


  Driver::~Driver()
  {
  }

  /*------------------------------------------------
  HW Interface
  ------------------------------------------------*/
  Chimera::Status_t Driver::open( const Chimera::CAN::DriverConfig &cfg )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if ( !validateCfg( cfg ) )
    {
      return Chimera::Status::INVAL_FUNC_PARAM;
    }

    /*-------------------------------------------------
    Initialize the Low Level Driver
    -------------------------------------------------*/
    auto lld = ::LLD::getDriver( cfg.HWInit.channel );
    if ( auto sts = lld->configure( cfg ); sts != Chimera::Status::OK )
    {
      return sts;
    }

    /*-------------------------------------------------
    Initialize the Queues
    -------------------------------------------------*/
    bool queuesCreated = true;

    mTxQueue.lock();
    queuesCreated |= mTxQueue.create( cfg.HWInit.txElements, sizeof( Chimera::CAN::BasicFrame ), cfg.HWInit.txBuffer );
    mTxQueue.unlock();

    mRxQueue.lock();
    queuesCreated |= mRxQueue.create( cfg.HWInit.rxElements, sizeof( Chimera::CAN::BasicFrame ), cfg.HWInit.rxBuffer );
    mRxQueue.unlock();

    if( queuesCreated )
    {
      mTxQueue.clear( Chimera::Threading::TIMEOUT_DONT_WAIT );
      mRxQueue.clear( Chimera::Threading::TIMEOUT_DONT_WAIT );
    }
    else
    {
      return Chimera::Status::FAILED_INIT;
    }

    /*-------------------------------------------------
    Register the ISR thread handlers
    -------------------------------------------------*/
    size_t lldResourceIndex = ::LLD::getResourceIndex( cfg.HWInit.channel );

    for(auto isr_idx=0; isr_idx<::LLD::NUM_CAN_IRQ_HANDLERS; isr_idx++)
    {
      if ( s_user_isr_thread_func[ lldResourceIndex ][ isr_idx ] )
      {
        Chimera::Threading::Thread thread;
        thread.initialize( s_user_isr_thread_func[ lldResourceIndex ][ isr_idx ], nullptr, Chimera::Threading::Priority::LEVEL_5,
                           STACK_BYTES( 250 ), nullptr );
        thread.start();
        s_user_isr_handle[ lldResourceIndex ][ isr_idx ] = thread.native_handle();
      }
    }

    /*-------------------------------------------------
    Initialize the ISR events to listen to
    -------------------------------------------------*/
    lld->enableISRSignal( Chimera::CAN::InterruptType::TX_ISR );
    lld->enableISRSignal( Chimera::CAN::InterruptType::RX_ISR );

    return Chimera::Status::OK;
  }


  Chimera::Status_t Driver::close()
  {
    using namespace Chimera::CAN;

    /*-------------------------------------------------
    Clear out the queues
    -------------------------------------------------*/
    mTxQueue.clear( Chimera::Threading::TIMEOUT_DONT_WAIT );
    mRxQueue.clear( Chimera::Threading::TIMEOUT_DONT_WAIT );

    /*-------------------------------------------------
    Disable all ISR signals
    -------------------------------------------------*/
    auto lld = ::LLD::getDriver( mConfig.HWInit.channel );
    for( auto isr=0; isr < static_cast<size_t>( InterruptType::NUM_OPTIONS ); isr++)
    {
      lld->disableISRSignal( static_cast<InterruptType>( isr ) );
    }


    // De-init the lld
    // De-register threads??
    // Clear the GPIO configs to be inputs
    return Chimera::Status::OK;
  }


  Chimera::CAN::CANStatus Driver::getStatus()
  {
    return Chimera::CAN::CANStatus();
  }


  Chimera::Status_t Driver::send( const Chimera::CAN::BasicFrame &frame )
  {
    using namespace Chimera::CAN;
    using namespace Chimera::Threading;

    auto lld = ::LLD::getDriver( mConfig.HWInit.channel );
    auto box = ::LLD::Mailbox::UNKNOWN;
    auto enq = Chimera::Status::OK;

    /*-------------------------------------------------
    Ensure transmit ISR is enabled
    -------------------------------------------------*/
    lld->enableISRSignal( Chimera::CAN::InterruptType::TX_ISR );

    /*-------------------------------------------------
    Post directly to hardware mailbox if possible but
    enqueue the message if not.
    -------------------------------------------------*/
    if( lld->txMailboxAvailable( box ) )
    {
      return lld->send( box, frame );
    }
    else
    {
      mTxQueue.lock();
      if( !mTxQueue.push( &frame, TIMEOUT_DONT_WAIT ) )
      {
        enq = Chimera::Status::FULL;
      }
      mTxQueue.unlock();
    }

    return enq;
  }


  Chimera::Status_t Driver::receive( Chimera::CAN::BasicFrame &frame, const size_t timeout )
  {
    /*-------------------------------------------------
    Data is pushed into this queue via an interrupt
    based message pump. The ISR awakens a high priority
    thread that assembles the raw data from hardware
    into the rx queue, until it can be read here.
    -------------------------------------------------*/
    if( mRxQueue.try_lock_for( timeout ) )
    {
      auto result = Chimera::Status::OK;
      if( mRxQueue.available() && !mRxQueue.pop( &frame, TIMEOUT_DONT_WAIT ))
      {
        result = Chimera::Status::FAILED_READ;
      }
      mRxQueue.unlock();

      return result;
    }
    else
    {
      return Chimera::Status::LOCKED;
    }
  }


  Chimera::Status_t Driver::filter( const Chimera::CAN::Filter *const list, const size_t size )
  {
    return Chimera::Status::OK;
  }


  Chimera::Status_t Driver::flush( Chimera::CAN::BufferType buffer )
  {
    return Chimera::Status::OK;
  }


  /*------------------------------------------------
  Async IO Interface
  ------------------------------------------------*/
  Chimera::Status_t Driver::await( const Chimera::Event::Trigger event, const size_t timeout )
  {
    return Chimera::Status::NOT_SUPPORTED;

    // if ( event != Chimera::Event::TRIGGER_TRANSFER_COMPLETE )
    // {
    //   return Chimera::Status::NOT_SUPPORTED;
    // }
    // else if ( !awaitTransferComplete.try_acquire_for( timeout ) )
    // {
    //   return Chimera::Status::TIMEOUT;
    // }
    // else
    // {
    //   return Chimera::Status::OK;
    // }
  }


  Chimera::Status_t Driver::await( const Chimera::Event::Trigger event, Chimera::Threading::BinarySemaphore &notifier,
                                   const size_t timeout )
  {
    auto result = await( event, timeout );

    if ( result == Chimera::Status::OK )
    {
      notifier.release();
    }

    return result;
  }


  /*------------------------------------------------
  Listener Interface
  ------------------------------------------------*/
  Chimera::Status_t Driver::registerListener( Chimera::Event::Actionable &listener, const size_t timeout,
                                              size_t &registrationID )
  {
    return Chimera::Status::NOT_SUPPORTED;
  }


  Chimera::Status_t Driver::removeListener( const size_t registrationID, const size_t timeout )
  {
    return Chimera::Status::NOT_SUPPORTED;
  }

  /*-------------------------------------------------
  ISR Event Handlers
  -------------------------------------------------*/
  void Driver::ProcessISREvent_TX()
  {
    using namespace Chimera::CAN;
    using namespace Chimera::Threading;

    auto lld = ::LLD::getDriver( mConfig.HWInit.channel );
    auto box = ::LLD::Mailbox::UNKNOWN;

    /*-------------------------------------------------
    Get the context of the last ISR
    -------------------------------------------------*/
    auto context = lld->getISRContext( InterruptType::RX_ISR );

    /*-------------------------------------------------
    Dequeue the next frames and push them to the hw
    mailboxes for transmission.
    -------------------------------------------------*/
    bool dequeueResult = false;
    auto transmitResult = Chimera::Status::OK;

    BasicFrame tmpFrame;
    tmpFrame.clear();

    while( lld->txMailboxAvailable( box ) )
    {
      mTxQueue.lock();

      if( mTxQueue.available() )
      {
        /*-------------------------------------------------
        Pull out the next message
        -------------------------------------------------*/
        dequeueResult = mTxQueue.pop( &tmpFrame, TIMEOUT_DONT_WAIT );
        mTxQueue.unlock();

        /*-------------------------------------------------
        Write the message to the hw mailbox
        -------------------------------------------------*/
        transmitResult = lld->send( box, tmpFrame );

        /*-------------------------------------------------
        Handle any errors
        -------------------------------------------------*/
        if( !dequeueResult || ( transmitResult != Chimera::Status::OK ) )
        {
          Chimera::insert_debug_breakpoint();
          break;
        }
      }
      else
      {
        mTxQueue.unlock();
        break;
      }
    }

    /*-------------------------------------------------
    Invoke any user callback
    -------------------------------------------------*/
    // TX?
    // TX Error?

    /*-------------------------------------------------
    Signal internal semaphores for events
    -------------------------------------------------*/
    // Any events to signal?
  }


  void Driver::ProcessISREvent_RX()
  {
    using namespace Chimera::CAN;
    using namespace Chimera::Threading;

    auto lld = ::LLD::getDriver( mConfig.HWInit.channel );
    auto box = ::LLD::Mailbox::UNKNOWN;

    /*-------------------------------------------------
    Get the context of the last ISR
    -------------------------------------------------*/
    auto context = lld->getISRContext( InterruptType::RX_ISR );

    /*-------------------------------------------------
    Enqueue all available messages
    -------------------------------------------------*/
    bool enqueueResult = false;
    auto receiveResult = Chimera::Status::OK;

    BasicFrame tmpFrame;
    tmpFrame.clear();

    while( lld->rxMailboxAvailable( box ) )
    {
      receiveResult = lld->receive( box, tmpFrame );

      /*-------------------------------------------------
      Update the queue with the new frame
      -------------------------------------------------*/
      mRxQueue.lock();
      enqueueResult = mRxQueue.push( &tmpFrame, TIMEOUT_DONT_WAIT );
      mRxQueue.unlock();

      /*-------------------------------------------------
      If we can't enqueue or the RX failed somehow, the
      frame was just lost.
      -------------------------------------------------*/
      if( !enqueueResult || ( receiveResult != Chimera::Status::OK ) )
      {
        Chimera::insert_debug_breakpoint();
        break;
      }
    }

    /*-------------------------------------------------
    Invoke user call back on the RX event
    -------------------------------------------------*/
    // todo
  }


  void Driver::ProcessISREvent_Error()
  {
    // Maybe reset the driver?
    // Execute user callback
  }


  void Driver::ProcessISREvent_StatusChange()
  {
    // Not sure if there will be much to do here
    // Execute user callback
  }

}    // namespace Thor::CAN

/*-------------------------------------------------------------------------------
High Priority Threads:
These handle ISR events with the context of a full scheduler
-------------------------------------------------------------------------------*/
#if defined( STM32_CAN1_PERIPH_AVAILABLE )
static void CAN1ISR_TXHandler( void *argument )
{
  using namespace Chimera::CAN;

  /*-------------------------------------------------
  Get a reference to the semaphore that will be given
  to once an event occurs.
  -------------------------------------------------*/
  auto sig = ::LLD::getDriver( Channel::CAN0 )->getISRSignal( InterruptType::TX_ISR );

  /*-------------------------------------------------
  Acquire the signal in a blocking manner. Once the
  ISR has signaled the event, process it here.
  -------------------------------------------------*/
  while ( 1 )
  {
    sig->acquire();
    hld_driver[ ::LLD::CAN1_RESOURCE_INDEX ].ProcessISREvent_TX();
  }
}


static void CAN1ISR_RXHandler( void *argument )
{
  using namespace Chimera::CAN;

  /*-------------------------------------------------
  Get a reference to the semaphore that will be given
  to once an event occurs.
  -------------------------------------------------*/
  auto sig = ::LLD::getDriver( Channel::CAN0 )->getISRSignal( InterruptType::RX_ISR );

  /*-------------------------------------------------
  Acquire the signal in a blocking manner. Once the
  ISR has signaled the event, process it here.
  -------------------------------------------------*/
  while ( 1 )
  {
    sig->acquire();
    hld_driver[ ::LLD::CAN1_RESOURCE_INDEX ].ProcessISREvent_RX();
  }
}


static void CAN1ISR_StatusChangeHandler( void *argument )
{
  using namespace Chimera::CAN;

  /*-------------------------------------------------
  Get a reference to the semaphore that will be given
  to once an event occurs.
  -------------------------------------------------*/
  auto sig = ::LLD::getDriver( Channel::CAN0 )->getISRSignal( InterruptType::STS_ISR );

  /*-------------------------------------------------
  Acquire the signal in a blocking manner. Once the
  ISR has signaled the event, process it here.
  -------------------------------------------------*/
  while ( 1 )
  {
    sig->acquire();
    hld_driver[ ::LLD::CAN1_RESOURCE_INDEX ].ProcessISREvent_StatusChange();
  }
}


static void CAN1ISR_ErrHandler( void *argument )
{
  using namespace Chimera::CAN;

  /*-------------------------------------------------
  Get a reference to the semaphore that will be given
  to once an event occurs.
  -------------------------------------------------*/
  auto sig = ::LLD::getDriver( Channel::CAN0 )->getISRSignal( InterruptType::ERR_ISR );

  /*-------------------------------------------------
  Acquire the signal in a blocking manner. Once the
  ISR has signaled the event, process it here.
  -------------------------------------------------*/
  while ( 1 )
  {
    sig->acquire();
    hld_driver[ ::LLD::CAN1_RESOURCE_INDEX ].ProcessISREvent_StatusChange();
  }
}

#endif  /* STM32_CAN1_PERIPH_AVAILABLE */


#endif /* THOR_HLD_CAN */

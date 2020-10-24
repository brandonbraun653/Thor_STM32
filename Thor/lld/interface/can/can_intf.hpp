/********************************************************************************
 *  File Name:
 *    can_intf.hpp
 *
 *  Description:
 *    STM32 LLD CAN Interface Spec
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_LLD_CAN_DRIVER_INTERFACE_HPP
#define THOR_LLD_CAN_DRIVER_INTERFACE_HPP

/* STL Includes */
#include <limits>

/* Aurora Includes */
#include <Aurora/container>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/can>
#include <Chimera/gpio>

/* Thor Includes */
#include <Thor/lld/common/interrupts/can_interrupt_vectors.hpp>
#include <Thor/lld/common/types.hpp>
#include <Thor/lld/interface/can/can_types.hpp>
#include <Thor/lld/interface/can/can_detail.hpp>

namespace Thor::LLD::CAN
{
  /*-------------------------------------------------------------------------------
  Public Functions (Implemented by the project)
  -------------------------------------------------------------------------------*/
  /**
   *  Initializes the low level driver
   *
   *  @return Chimera::Status_t
   */
  Chimera::Status_t initialize();

  /**
   *  Gets a raw pointer to the CAN driver for a particular channel
   *
   *  @param[in]  channel     The CAN channel to get
   *  @return IDriver_sPtr    Instance of the CAN driver for the requested channel
   */
  Driver_rPtr getDriver( const Chimera::CAN::Channel channel );

  /*-------------------------------------------------------------------------------
  Public Functions (Implemented at the interface layer)
  -------------------------------------------------------------------------------*/
  /**
   *  Checks if the given hardware channel is supported on this device.
   *
   *  @param[in]  port        The CAN port to grab
   *  @param[in]  pin         Which pin on the given port
   *  @return bool
   */
  bool isSupported( const Chimera::CAN::Channel channel );

  /**
   *  Looks up an index value that can be used to access distributed resources
   *  associated with a peripheral instance. If the address is invalid, this will
   *  return INVALID_RESOURCE_INDEX.
   *
   *  @param[in]  address       Memory address the peripheral is mapped to
   *  @return RIndex_t
   */
  RIndex_t getResourceIndex( const std::uintptr_t address );

  /**
   *  Looks up an index value that can be used to access distributed resources
   *  associated with a peripheral instance. If the port is invalid, this will
   *  return INVALID_RESOURCE_INDEX.
   *
   *  @param[in]  port          Which port to get the resource index for
   *  @return RIndex_t
   */
  RIndex_t getResourceIndex( const Chimera::CAN::Channel channel );

  /**
   *  Gets the CAN port associated with a peripheral address
   *
   *  @param[in]  address       Memory address the peripheral is mapped to
   *  @return Chimera::CAN::Channel
   */
  Chimera::CAN::Channel getChannel( const std::uintptr_t address );

  /**
   *  Initializes the CAN drivers by attaching the appropriate peripheral
   *
   *  @param[in]  driverList    List of driver objects to be initialized
   *  @param[in]  numDrivers    How many drivers are in driverList
   *  @return bool
   */
  bool attachDriverInstances( Driver *const driverList, const size_t numDrivers );

  /**
   *  Takes the given filter list and organizes them by size, from largest to smalles.
   *  This helps with assigning filters to hardware banks by placing them into a logical order.
   *
   *  @note Size is determined by the number of bytes each filter takes up in a bank.
   *        Generally this is configurable from 2-8 bytes.
   *
   *  @param[in]  filterList    List of configured filters to be sorted
   *  @param[in]  listSize      Number of filters the list can hold. All lists must match this length.
   *  @param[out] indexList     On successful sort, will contain indices of the sorted filterList
   *  @return bool              True if the list was filtered, false if there was some problem
   */
  bool sortFiltersBySize( const MessageFilter *const filterList, const uint8_t listSize, uint8_t *const indexList );


  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /*-------------------------------------------------
  Virtual class that defines the expected interface.
  Useful for mocking purposes.
  -------------------------------------------------*/
  class IDriver
  {
  public:
    virtual ~IDriver() = default;

    /*-------------------------------------------------------------------------------
    Configuration
    -------------------------------------------------------------------------------*/
    /**
     *  Attaches a peripheral instance to the interaction model
     *
     *  @param[in]  peripheral    Memory mapped struct of the desired CAN peripheral
     *  @return void
     */
    virtual void attach( RegisterMap *const peripheral ) = 0;

    /**
     *  Enables the peripheral clock for the configured instance
     *  @return void
     */
    virtual void enableClock() = 0;

    /**
     *  Disables the peripheral clock for the configured instance
     *  @return void
     */
    virtual void disableClock() = 0;

    /**
     *  Configure the CAN bus driver with the appropriate settings. These are
     *  more generic settings that apply across all CAN implementations.
     *
     *  @param[in]  cfg           The configuration info
     *  @return Chimera::Status_t
     */
    virtual Chimera::Status_t configure( const Chimera::CAN::DriverConfig &cfg ) = 0;

    /**
     *  Applies the given filter list onto the hardware filter banks. The list
     *  objects will be updated to include its assigned match index (FMI), assuming
     *  the filter was successfully configured.
     *
     *  @note This operation will destroy any previous filter configuration and
     *        replace it with the one given.
     *
     *  @param[in]  filterList    List of filters that should be configured
     *  @param[in]  filterSize    How many filters are in the list
     *  @return Chimera::Status_t
     */
    virtual Chimera::Status_t applyFilters( MessageFilter *const filterList, const size_t filterSize ) = 0;

    /**
     *  Turns on interrupts for the given signal type, if supported.
     *
     *  @param[in]  signal        The ISR event to enable
     *  @return Chimera::Status_t
     */
    virtual Chimera::Status_t enableISRSignal( const Chimera::CAN::InterruptType signal ) = 0;

    /**
     *  Turns off interrupts for the given signal type, if supported.
     *
     *  @param[in]  signal        The ISR event to enable
     *  @return void
     */
    virtual void disableISRSignal( const Chimera::CAN::InterruptType signal ) = 0;

    virtual void freezeOnDebug( const bool doFreeze ) = 0;

    virtual void invokeMasterResetRequest() = 0;

    virtual void useAutoRetransmit( const bool doAutoRTX ) = 0;

    virtual void lockRXOnOverrun( const bool doLock ) = 0;

    virtual void useTXPriorityScheme( const TXPriority scheme ) = 0;

    virtual void enterSleepMode() = 0;

    virtual void exitSleepMode() = 0;

    virtual void enterDebugMode( const Chimera::CAN::DebugMode mode ) = 0;

    virtual void exitDebugMode() = 0;

    /*-------------------------------------------------------------------------------
    Control
    -------------------------------------------------------------------------------*/
    /**
     *  Flushes the TX buffer
     *  @return void
     */
    virtual void flushTX() = 0;

    /**
     *  Flushes the RX buffer
     *  @return void
     */
    virtual void flushRX() = 0;

    /*-------------------------------------------------------------------------------
    Transmit & Receive Operations
    -------------------------------------------------------------------------------*/
    /**
     *  Places the given frame into the TX circular buffer for transmission.
     *
     *  @param[in]  frame       The frame to be transmitted
     *  @return Chimera::Status_t
     */
    virtual Chimera::Status_t send( const Chimera::CAN::BasicFrame &frame ) = 0;

    /**
     *  Reads a frame off the internal RX circular buffer. This buffer is populated
     *  by the ISR when new data arrives.
     *
     *  @param[out] frame       The frame to place the received message into
     *  @return Chimera::Status_t
     */
    virtual Chimera::Status_t receive( Chimera::CAN::BasicFrame &frame ) = 0;

    /*-------------------------------------------------------------------------------
    Asynchronous Operation
    -------------------------------------------------------------------------------*/
    /**
     *  Gets the driver's semaphore associated with an ISR event. The semaphore
     *  will be given to upon event occurance, unblocking a task that is pending.
     *
     *  @warning Only a single thread should consume the signal. The purpose of
     *  this function is to allow a high priority thread in the HLD to process
     *  the event outside of the limited ISR context.
     *
     *  @param[in]  signal        Which ISR signal to get the semaphore for
     *  @return Chimera::Threading::BinarySemaphore *
     */
    virtual Chimera::Threading::BinarySemaphore *getISRSignal( Chimera::CAN::InterruptType signal ) = 0;

    /**
     *  Gets any information that was posted in relation to the last
     *  ISR event.
     *
     *  @note This is limited to read-only because of the single producer
     *  many consumer data model. The ISR produces, you consume. Copy out
     *  the data inside of a critical section to guarantee the ISR won't
     *  modify the structure while reading.
     *
     *  @param[in]  isr           The ISR event type to get the context for
     *  @return const ISREventContext *const
     */
    virtual const ISREventContext *const getISRContext( const Chimera::CAN::InterruptType isr ) = 0;

    /**
     *  Callback for the HLD to inform the LLD that the associated ISR event
     *  has been properly handled and it's ok to re-enable the signal.
     *
     *  @param[in]  isr           The ISR signal to indicate was handled
     *  @return void
     */
    virtual void setISRHandled( const Chimera::CAN::InterruptType isr ) = 0;

    /*-------------------------------------------------------------------------------
    ISR Protection Mechanisms
    -------------------------------------------------------------------------------*/
    /**
     *  Disables peripheral specific CAN interrupts. This prevents
     *  the hardware from generating events when those events would
     *  interfere with a complex operation.
     *
     *  These events will be pending in the NVIC hardware and fire
     *  as soon as exitCriticalSection() is called.
     *
     *  @return void
     */
    virtual void enterCriticalSection() = 0;

    /**
     *  Re-enables peripheral specific CAN interrupts. If an event
     *  was generated while inside a critical context, it will fire
     *  immediately upon calling this function.
     *
     *  @return void
     */
    virtual void exitCriticalSection() = 0;
  };


  /*-------------------------------------------------
  Concrete driver declaration. Implements the interface
  of the virtual class, but doesn't inherit due to the
  memory penalties. Definition is done project side.
  -------------------------------------------------*/
  class Driver
  {
  public:
    Driver();
    ~Driver();

    /*-------------------------------------------------------------------------------
    Configuration
    -------------------------------------------------------------------------------*/
    void attach( RegisterMap *const peripheral );
    void enableClock();
    void disableClock();
    Chimera::Status_t configure( const Chimera::CAN::DriverConfig &cfg );
    Chimera::Status_t applyFilters( MessageFilter *const filterList, const size_t filterSize );
    Chimera::Status_t enableISRSignal( const Chimera::CAN::InterruptType signal );
    void disableISRSignal( const Chimera::CAN::InterruptType signal );
    void enterDebugMode( const Chimera::CAN::DebugMode mode );
    void exitDebugMode();

    /*-------------------------------------------------------------------------------
    Control
    -------------------------------------------------------------------------------*/
    void flushTX();
    void flushRX();

    /*-------------------------------------------------------------------------------
    Transmit & Receive Operations
    -------------------------------------------------------------------------------*/
    Chimera::Status_t send( const Chimera::CAN::BasicFrame &frame );
    Chimera::Status_t receive( Chimera::CAN::BasicFrame &frame );

    /*-------------------------------------------------------------------------------
    Asynchronous Operation
    -------------------------------------------------------------------------------*/
    Chimera::Threading::BinarySemaphore *getISRSignal( Chimera::CAN::InterruptType signal );
    const ISREventContext *const getISRContext( const Chimera::CAN::InterruptType isr );
    void setISRHandled( const Chimera::CAN::InterruptType isr );

    /*-------------------------------------------------------------------------------
    ISR Protection Mechanisms
    -------------------------------------------------------------------------------*/
    void enterCriticalSection();
    void exitCriticalSection();

  protected:
    /*-------------------------------------------------------------------------------
    ISR Handlers
    -------------------------------------------------------------------------------*/
    void CAN1_TX_IRQHandler();
    void CAN1_FIFO0_IRQHandler();
    void CAN1_FIFO1_IRQHandler();
    void CAN1_ERR_STS_CHG_IRQHandler();

  private:
    friend void( ::CAN1_TX_IRQHandler )();
    friend void( ::CAN1_RX0_IRQHandler )();
    friend void( ::CAN1_RX1_IRQHandler )();
    friend void( ::CAN1_SCE_IRQHandler )();

    /*-------------------------------------------------
    Peripheral descriptive information
    -------------------------------------------------*/
    RegisterMap *mPeriph;
    size_t mResourceIndex;

    /*-------------------------------------------------
    Transmit/Receive Buffers
    -------------------------------------------------*/
    Aurora::Container::CircularBuffer<Chimera::CAN::BasicFrame> mTXBuffer;
    Aurora::Container::CircularBuffer<Chimera::CAN::BasicFrame> mRXBuffer;

    /*-------------------------------------------------
    ISR signaling and context buffers
    -------------------------------------------------*/
    ISREventContext mISREventContext[ NUM_CAN_IRQ_HANDLERS ];
    Chimera::Threading::BinarySemaphore mISREventSignal[ NUM_CAN_IRQ_HANDLERS ];
  };
}    // namespace Thor::LLD::CAN

#endif /* !THOR_LLD_CAN_DRIVER_INTERFACE_HPP */

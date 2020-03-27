/********************************************************************************
 *  File Name:
 *    spi.hpp
 *
 *  Description:
 *    Thor SPI high level driver
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_SPI_HPP
#define THOR_SPI_HPP

/* C/C++ Includes */
#include <cstdint>

/* Chimera Includes */
#include <Chimera/event>
#include <Chimera/gpio>
#include <Chimera/spi>
#include <Chimera/thread>

/* Thor Includes */
#include <Thor/gpio>
#include <Thor/lld/interface/spi/spi_types.hpp>

namespace Thor::SPI
{
  Chimera::Status_t initialize();

  class Driver : virtual public Chimera::SPI::ISPI,
                   public Chimera::Threading::Lockable
  {
  public:
    /*------------------------------------------------
    Class Specific Functions
    ------------------------------------------------*/
    Driver();
    ~Driver();

    void postISRProcessing();

    /*------------------------------------------------
    HW Interface
    ------------------------------------------------*/
    Chimera::Status_t init( const Chimera::SPI::DriverConfig &setupStruct ) final override;
    Chimera::SPI::DriverConfig getInit() final override;
    Chimera::Status_t deInit() final override;
    Chimera::Status_t setChipSelect( const Chimera::GPIO::State value ) final override;
    Chimera::Status_t setChipSelectControlMode( const Chimera::SPI::CSMode mode ) final override;
    Chimera::Status_t writeBytes( const void *const txBuffer, const size_t length, const size_t timeoutMS ) final override;
    Chimera::Status_t readBytes( void *const rxBuffer, const size_t length, const size_t timeoutMS ) final override;
    Chimera::Status_t readWriteBytes( const void *const txBuffer, void *const rxBuffer, const size_t length,
                                      const size_t timeoutMS ) final override;
    Chimera::Status_t setPeripheralMode( const Chimera::Hardware::PeripheralMode mode ) final override;
    Chimera::Status_t setClockFrequency( const size_t freq, const size_t tolerance ) final override;
    size_t getClockFrequency() final override;

    /*------------------------------------------------
    Async IO Interface
    ------------------------------------------------*/
    Chimera::Status_t await( const Chimera::Event::Trigger event, const size_t timeout ) final override;
    Chimera::Status_t await( const Chimera::Event::Trigger event, Chimera::Threading::BinarySemaphore &notifier,
                             const size_t timeout ) final override;

    /*------------------------------------------------
    Listener Interface
    ------------------------------------------------*/
    Chimera::Status_t registerListener( Chimera::Event::Actionable &listener, const size_t timeout,
                                        size_t &registrationID ) final override;
    Chimera::Status_t removeListener( const size_t registrationID, const size_t timeout ) final override;

  private:
    Chimera::SPI::DriverConfig config;  /**< Configuration used to set up the class */
    Thor::GPIO::Driver_uPtr SCK;          /**< SPI clock gpio pin */
    Thor::GPIO::Driver_uPtr MOSI;         /**< SPI MOSI gpio pin */
    Thor::GPIO::Driver_uPtr MISO;         /**< SPI MISO gpio pin */
    Thor::GPIO::Driver_sPtr CS;           /**< SPI Chip Select gpio pin */
    Thor::LLD::SPI::Driver_uPtr driver; /**< Low level hardware SPI driver */

    Chimera::Event::ActionableList eventListeners;
    Chimera::Threading::BinarySemaphore awaitTransferComplete; /**< Internal signal for when the current transfer has completed */
  };

}    // namespace Thor::SPI

#endif /* SPI_H_*/
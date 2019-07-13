/********************************************************************************
 *   File Name:
 *    hw_usart_driver.hpp
 *
 *   Description:
 *    STM32 Driver for the USART Peripheral
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_HW_USART_DRIVER_HPP
#define THOR_HW_USART_DRIVER_HPP

/* C++ Includes */
#include <vector>

/* Boost Includes */
#include <boost/function.hpp>

/* Chimera Includes */
#include <Chimera/threading.hpp>
#include <Chimera/types/common_types.hpp>
#include <Chimera/types/peripheral_types.hpp>

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/common/types/serial_types.hpp>
#include <Thor/drivers/f4/common/types.hpp>
#include <Thor/drivers/f4/usart/hw_usart_types.hpp>
#include <Thor/drivers/model/event_model.hpp>
#include <Thor/drivers/model/interrupt_model.hpp>
#include <Thor/drivers/model/serial_model.hpp>

#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_USART == 1 )

namespace Thor::Driver::USART
{

  class Driver : public Thor::Driver::Serial::Basic,
                 public Thor::Driver::Serial::Extended,
                 public Thor::Driver::SignalModel,
                 public Thor::Driver::EventListener
  {
  public:
    Driver( RegisterMap *const peripheral );
    ~Driver();

    Chimera::Status_t init( const Thor::Driver::Serial::Config &cfg ) final override;

    Chimera::Status_t deinit() final override;

    Chimera::Status_t reset() final override;

    Chimera::Status_t transmit( const uint8_t *const data, const size_t size, const size_t timeout ) final override;

    Chimera::Status_t receive( uint8_t *const data, const size_t size, const size_t timeout ) final override;

    Chimera::Status_t enableIT( const Chimera::Hardware::SubPeripheral periph ) final override;

    Chimera::Status_t disableIT( const Chimera::Hardware::SubPeripheral periph ) final override;

    Chimera::Status_t transmitIT( uint8_t *const data, const size_t size, const size_t timeout ) final override;

    Chimera::Status_t receiveIT( uint8_t *const data, const size_t size, const size_t timeout ) final override;

    Chimera::Status_t initDMA() final override;

    Chimera::Status_t deinitDMA() final override;

    Chimera::Status_t enableDMA_IT( const Chimera::Hardware::SubPeripheral periph ) final override;

    Chimera::Status_t disableDMA_IT( const Chimera::Hardware::SubPeripheral periph ) final override;

    Chimera::Status_t transmitDMA( uint8_t *const data, const size_t size, const size_t timeout ) final override;

    Chimera::Status_t receiveDMA( uint8_t *const data, const size_t size, const size_t timeout ) final override;

    Chimera::Status_t enableSignal( const InterruptSignal_t sig ) final override;

    Chimera::Status_t disableSignal( const InterruptSignal_t sig ) final override;

    Chimera::Status_t registerEventListener( const Chimera::Event::Trigger event,
                                             SemaphoreHandle_t *const listener ) final override;

    Chimera::Status_t removeEventListener( const Chimera::Event::Trigger event,
                                           SemaphoreHandle_t *const listener ) final override;

    Chimera::Hardware::Status pollTransferStatus() final override;

  private:
    RegisterMap *const periph;
    size_t resourceIndex;
    Chimera::Peripheral::Type peripheralType;
    std::vector<SemaphoreHandle_t *> rxCompleteListeners;
    std::vector<SemaphoreHandle_t *> txCompleteListeners;
  };
}    // namespace Thor::Driver::USART

#endif /* TARGET_STM32F4 && THOR_DRIVER_USART */
#endif /* !THOR_HW_USART_DRIVER_HPP */

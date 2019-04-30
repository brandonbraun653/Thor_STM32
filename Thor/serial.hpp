/********************************************************************************
 * File Name:
 *   serial.hpp
 *
 * Description:
 *   Wrapper around the Thor UART and USART drivers to merge the hardware layer
 *   into a common interface for the Chimera serial library.
 *
 * 2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_SERIAL_HPP
#define THOR_SERIAL_HPP

/* C++ Includes */
#include <cstdlib>
#include <cstdint>
#include <memory>

/* Thor Includes */
#include <Thor/uart.hpp>
#include <Thor/usart.hpp>

namespace Thor::Serial
{
  class SerialClass;
  using SerialClass_sPtr = std::shared_ptr<SerialClass>;
  using SerialClass_uPtr = std::unique_ptr<SerialClass>;

  class SerialClass : public Chimera::Serial::Interface
  {
  public:
    SerialClass();
    ~SerialClass() = default;

    Chimera::Status_t assignHW( const uint8_t channel, const Chimera::Serial::IOPins &pins ) final override;

    Chimera::Status_t begin( const Chimera::Serial::Modes txMode, const Chimera::Serial::Modes rxMode ) final override;

    Chimera::Status_t end() final override;

    Chimera::Status_t configure( const uint32_t baud, const Chimera::Serial::CharWid width,
                                 const Chimera::Serial::Parity parity, const Chimera::Serial::StopBits stop,
                                 const Chimera::Serial::FlowControl flow ) final override;

    Chimera::Status_t setBaud( const uint32_t baud ) final override;

    Chimera::Status_t setMode( const Chimera::Hardware::SubPeripheral periph, const Chimera::Serial::Modes mode ) final override;

    Chimera::Status_t write( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) final override;

    Chimera::Status_t read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) final override;

    Chimera::Status_t flush( const Chimera::Hardware::SubPeripheral periph ) final override;

    void postISRProcessing() final override;

    Chimera::Status_t readAsync( uint8_t *const buffer, const size_t len ) final override;


    Chimera::Status_t attachNotifier( const Chimera::Event::Trigger_t event, SemaphoreHandle_t *const semphr ) final override;

    Chimera::Status_t detachNotifier( const Chimera::Event::Trigger_t event, SemaphoreHandle_t *const semphr ) final override;

    Chimera::Status_t enableBuffering( const Chimera::Hardware::SubPeripheral periph,
                                               boost::circular_buffer<uint8_t> *const userBuffer, uint8_t *const hwBuffer,
                                               const uint32_t hwBufferSize ) final override;

    Chimera::Status_t disableBuffering( const Chimera::Hardware::SubPeripheral periph ) final override;

    bool available( size_t *const bytes = nullptr ) final override;

  private:
    uint8_t serialChannel = 0;
    Chimera::Serial::Interface_sPtr serialObject;
  };
}    // namespace Thor::Serial
#endif /* !SERIAL_H_ */

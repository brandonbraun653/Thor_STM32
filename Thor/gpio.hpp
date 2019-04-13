/********************************************************************************
 * File Name:
 *   gpio.hpp
 *
 * Description:
 *   Implements the Thor GPIO driver
 *
 * 2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/
#pragma once
#ifndef THOR_GPIO_H_
#define THOR_GPIO_H_

/* C++ Includes */
#include <cstdint>

/* Chimera Includes */
#include <Chimera/interface.hpp>

/* Thor Includes */
#include <Thor/definitions.hpp>

namespace Thor
{
  namespace GPIO
  {
    class GPIOClass;
    using GPIOClass_sPtr = std::shared_ptr<GPIOClass>;
    using GPIOClass_uPtr = std::unique_ptr<GPIOClass>;

    class GPIOClass : public Chimera::GPIO::Interface
    {
    public:
      GPIOClass();
      ~GPIOClass();

      Chimera::Status_t init( const Chimera::GPIO::Port port, const uint8_t pin ) final override;

      Chimera::Status_t setMode( const Chimera::GPIO::Drive drive, const bool pullup ) final override;

      Chimera::Status_t setState( const Chimera::GPIO::State state ) final override;

      Chimera::Status_t getState( Chimera::GPIO::State &state ) final override;

      Chimera::Status_t toggle() final override;

      /**
       *  A more advanced initialization function that allows full configuration of a pin's behavior in one go
       *
       *  @param[in]  port    The port to use
       *  @param[in]  pin     The pin to use
       *  @param[in]  speed   How "fast" you want the pin to switch. This is effectively drive strength.
       *  @param[in]  alt     Alternate function parameter as defined in the STM32 HAL to remap the GPIO to a peripheral
       *  @return void
       */
      void initAdvanced( const Thor::GPIO::PinPort port, const Thor::GPIO::PinNum pin, const Thor::GPIO::PinSpeed speed,
                         const uint32_t alt );

      static GPIO_InitTypeDef getHALInit( const Thor::GPIO::Initializer &config );

    private:
      bool initialized = false;
      Thor::GPIO::Initializer pinConfig;

      void GPIO_Init( Thor::GPIO::PinPort port, GPIO_InitTypeDef *initStruct );
      void GPIO_ClockEnable( Thor::GPIO::PinPort port );
      void GPIO_ClockDisable( Thor::GPIO::PinPort port );
    };

    extern const PinNum convertPinNum( const uint8_t num );
    extern const PinPort convertPort( const Chimera::GPIO::Port port );
    extern const PinMode convertDrive( const Chimera::GPIO::Drive drive );
    extern const PinPull convertPull( const Chimera::GPIO::Pull pull );
    extern const Initializer convertPinInit( const Chimera::GPIO::PinInit &pin );
  }    // namespace GPIO
}    // namespace Thor


#endif    // !_GPIO_H_

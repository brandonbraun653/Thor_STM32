/********************************************************************************
 *   File Name:
 *    hw_rcc_mapping.hpp
 *
 *   Description:
 *    Provides structures for conversion and mapping between data types for fast
 *    runtime performance of driver code.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_HW_RCC_MAPPING_HPP
#define THOR_HW_RCC_MAPPING_HPP

/* C++ Includes */
#include <array>

/* Chimera Includes */
#include <Chimera/types/peripheral_types.hpp>
#include <Chimera/types/gpio_types.hpp>

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/f4/gpio/hw_gpio_prj.hpp>
#include <Thor/drivers/f4/uart/hw_uart_prj.hpp>
#include <Thor/drivers/f4/usart/hw_usart_prj.hpp>
#include <Thor/drivers/f4/rcc/hw_rcc_types.hpp>
#include <Thor/drivers/f4/rcc/hw_rcc_prj.hpp>
#include <Thor/drivers/f4/system/sys_memory_map_prj.hpp>

namespace Thor::Driver::RCC
{
  namespace LookupTables
  {
    /**
     *  GPIO Peripheral Config Lookup Tables
     */
    static constexpr size_t gpioTableSize = Thor::Driver::GPIO::NUM_GPIO_PERIPHS;

    extern const PCC GPIOLookup;

    extern const RegisterConfig GPIO_ClockConfig[ gpioTableSize ];
    extern const RegisterConfig GPIO_ClockConfigLP[ gpioTableSize ];
    extern const RegisterConfig GPIO_ResetConfig[ gpioTableSize ];

    extern const Configuration::ClockType_t GPIO_SourceClock[ gpioTableSize ];

    /**
     *  UART Peripheral Config Lookup Tables
     */
    static constexpr size_t uartTableSize = Thor::Driver::UART::NUM_UART_PERIPHS;

    extern const PCC UARTLookup;

    extern const RegisterConfig UART_ClockConfig[ uartTableSize ];
    extern const RegisterConfig UART_ClockConfigLP[ uartTableSize ];
    extern const RegisterConfig UART_ResetConfig[ uartTableSize ];

    extern const Configuration::ClockType_t UART_SourceClock[ uartTableSize ];

    /**
     *  USART Peripheral Config Lookup Tables
     */
    static constexpr size_t usartTableSize = Thor::Driver::USART::NUM_USART_PERIPHS;

    extern const PCC USARTLookup;

    extern const RegisterConfig USART_ClockConfig[ usartTableSize ];
    extern const RegisterConfig USART_ClockConfigLP[ usartTableSize ];
    extern const RegisterConfig USART_ResetConfig[ usartTableSize ];
    
    extern const Configuration::ClockType_t USART_SourceClock[ usartTableSize ];
  }    // namespace LookupTables

}    // namespace Thor::Driver::RCC

#endif /* !THOR_HW_RCC_MAPPING_HPP */

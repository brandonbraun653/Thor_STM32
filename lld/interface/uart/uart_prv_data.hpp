/********************************************************************************
 *  File Name:
 *    uart_prv_data.hpp
 *
 *  Description:
 *    Declaration of data that must be defined by the LLD implementation or is
 *    shared among all possible drivers.
 *
 *  2021 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef THOR_LLD_UART_DATA_HPP
#define THOR_LLD_UART_DATA_HPP

/* STL Includes */
#include <cstddef>

/* Chimera Includes */
#include <Chimera/serial>

/* Thor Includes */
#include <Thor/lld/common/macros.hpp>
#include <Thor/lld/common/types.hpp>
#include <Thor/lld/interface/interrupt/interrupt_detail.hpp>
#include <Thor/lld/interface/usart/usart_detail.hpp>

namespace Thor::LLD::UART
{
  /*-------------------------------------------------------------------------------
  Peripheral Instances:
    Memory mapped structs that allow direct access to the registers of a peripheral
  -------------------------------------------------------------------------------*/
// #if defined( STM32_UART1_PERIPH_AVAILABLE )
//   extern RegisterMap *UART1_PERIPH;
// #endif
// #if defined( STM32_UART2_PERIPH_AVAILABLE )
//   extern RegisterMap *UART2_PERIPH;
// #endif
// #if defined( STM32_UART3_PERIPH_AVAILABLE )
//   extern RegisterMap *UART3_PERIPH;
// #endif


  /*-------------------------------------------------------------------------------
  Configuration Maps:
    These convert high level configuration options into low level register config
    options. The idea is to allow the user to specify some general options, then
    convert that over to what the peripheral understands during config/init steps.
  -------------------------------------------------------------------------------*/
  namespace ConfigMap
  {
    // extern LLD_CONST Reg32_t CharWidth[ static_cast<size_t>( Chimera::Serial::CharWid::NUM_OPTIONS ) ];
    // extern LLD_CONST Reg32_t Parity[ static_cast<size_t>( Chimera::Serial::Parity::NUM_OPTIONS ) ];
    // extern LLD_CONST Reg32_t StopBits[ static_cast<size_t>( Chimera::Serial::StopBits::NUM_OPTIONS ) ];
  }    // namespace ConfigMap


  /*-------------------------------------------------------------------------------
  Peripheral Resources:
    These objects define critical resources used in the low level driver. The goal
    is to minimize memory consumption, so these arrays only hold enough information
    for the currently configured number of peripherals. They are intended to be
    accessed directly via the _ResourceIndex_ attribute of the ConfigMap namespace.
  -------------------------------------------------------------------------------*/
  namespace Resource
  {
    // extern LLD_CONST Reg32_t RXDMASignals[ NUM_UART_PERIPHS ];
    // extern LLD_CONST Reg32_t TXDMASignals[ NUM_UART_PERIPHS ];
    // extern LLD_CONST IRQn_Type IRQSignals[ NUM_UART_PERIPHS ];
  }
}    // namespace Thor::LLD::UART

#endif /* !THOR_LLD_UART_DATA_HPP */
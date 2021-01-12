/********************************************************************************
 *  File Name:
 *    hw_uart_mapping.hpp
 *
 *  Description:
 *    Provides structures for conversion and mapping between data types for fast
 *    runtime performance of driver code.
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_HW_UART_MAPPING_HPP
#define THOR_HW_UART_MAPPING_HPP

/* Chimera Includes */
#include <Chimera/container>

/* Driver Includes */
#include <Thor/lld/stm32f4x/interrupt/hw_it_prj.hpp>
#include <Thor/lld/stm32f4x/rcc/hw_rcc_types.hpp>
#include <Thor/lld/stm32f4x/uart/hw_uart_types.hpp>

namespace Thor::LLD::UART
{
#if defined( STM32_UART4_PERIPH_AVAILABLE )
  extern RegisterMap * UART4_PERIPH;
#endif 

#if defined( STM32_UART5_PERIPH_AVAILABLE )
  extern RegisterMap * UART5_PERIPH;
#endif 

  /*------------------------------------------------
  Peripheral Memory Mapping
  ------------------------------------------------*/
  extern PeriphRegisterList PeripheralList;
  extern DMASignalList RXDMASignals;        /**< RX DMA signal identifiers for each UART instance */
  extern DMASignalList TXDMASignals;        /**< RX DMA signal identifiers for each UART instance */
  extern DriverInstanceList spiObjects;     /**< Driver objects for each UART Instance */

  extern Chimera::Container::LightFlatMap<std::uintptr_t, size_t> InstanceToResourceIndex;
  extern Chimera::Container::LightFlatMap<size_t, RegisterMap *> ChanneltoInstance;

  /*------------------------------------------------
  Gets the interrupt request number tied to a UART instance.
  ------------------------------------------------*/
  extern const IRQn_Type UART_IRQn[ NUM_UART_PERIPHS ];

  /**
   *  Initializes memory associated with mapping
   *  
   *  @return void
   */
  void initializeMapping();

  /**
   *  Checks if the given address belongs to a peripheral instance
   *
   *  @return bool
   */
  bool isUART( const std::uintptr_t address );

}    // namespace Thor::LLD::UART

#endif /* !THOR_HW_UART_MAPPING_HPP */
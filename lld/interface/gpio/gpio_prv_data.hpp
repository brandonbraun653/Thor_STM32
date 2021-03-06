/********************************************************************************
 *  File Name:
 *    gpio_prv_data.hpp
 *
 *  Description:
 *    Declaration of data that must be defined by the LLD implementation or is
 *    shared among all possible drivers.
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef THOR_LLD_GPIO_DATA_HPP
#define THOR_LLD_GPIO_DATA_HPP

/* STL Includes */
#include <cstddef>

/* Chimera Includes */
#include <Chimera/gpio>

/* Thor Includes */
#include <Thor/cfg>
#include <Thor/lld/common/macros.hpp>
#include <Thor/lld/common/types.hpp>
#include <Thor/lld/interface/gpio/gpio_detail.hpp>
#include <Thor/lld/interface/gpio/gpio_types.hpp>

#if defined( THOR_LLD_GPIO )
namespace Thor::LLD::GPIO
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  static constexpr size_t DRIVER_MAX_PORTS         = static_cast<size_t>( Chimera::GPIO::Port::NUM_OPTIONS );
  static constexpr size_t DRIVER_MAX_PINS_PER_PORT = 16;
  static constexpr size_t DRIVER_MAX_PINS          = DRIVER_MAX_PORTS * DRIVER_MAX_PINS_PER_PORT;


#if defined( STM32_GPIOA_PERIPH_AVAILABLE )
  static constexpr size_t GPIOA_PIN_RINDEX_OFFSET = 0;
#else
  static constexpr size_t GPIOA_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOA_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOB_PERIPH_AVAILABLE )
  static constexpr size_t GPIOB_PIN_RINDEX_OFFSET = GPIOA_PIN_RINDEX_OFFSET + GPIOA_NUM_PINS;
#else
  static constexpr size_t GPIOB_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOB_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOC_PERIPH_AVAILABLE )
  static constexpr size_t GPIOC_PIN_RINDEX_OFFSET = GPIOB_PIN_RINDEX_OFFSET + GPIOB_NUM_PINS;
#else
  static constexpr size_t GPIOC_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOC_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOD_PERIPH_AVAILABLE )
  static constexpr size_t GPIOD_PIN_RINDEX_OFFSET = GPIOC_PIN_RINDEX_OFFSET + GPIOC_NUM_PINS;
#else
  static constexpr size_t GPIOD_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOD_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOE_PERIPH_AVAILABLE )
  static constexpr size_t GPIOE_PIN_RINDEX_OFFSET = GPIOD_PIN_RINDEX_OFFSET + GPIOD_NUM_PINS;
#else
  static constexpr size_t GPIOE_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOE_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOF_PERIPH_AVAILABLE )
  static constexpr size_t GPIOF_PIN_RINDEX_OFFSET = GPIOE_PIN_RINDEX_OFFSET + GPIOE_NUM_PINS;
#else
  static constexpr size_t GPIOF_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOF_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOG_PERIPH_AVAILABLE )
  static constexpr size_t GPIOG_PIN_RINDEX_OFFSET = GPIOF_PIN_RINDEX_OFFSET + GPIOF_NUM_PINS;
#else
  static constexpr size_t GPIOG_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOG_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOH_PERIPH_AVAILABLE )
  static constexpr size_t GPIOH_PIN_RINDEX_OFFSET = GPIOG_PIN_RINDEX_OFFSET + GPIOG_NUM_PINS;
#else
  static constexpr size_t GPIOH_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOH_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOI_PERIPH_AVAILABLE )
  static constexpr size_t GPIOI_PIN_RINDEX_OFFSET = GPIOH_PIN_RINDEX_OFFSET + GPIOH_NUM_PINS;
#else
  static constexpr size_t GPIOI_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOI_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOJ_PERIPH_AVAILABLE )
  static constexpr size_t GPIOJ_PIN_RINDEX_OFFSET = GPIOI_PIN_RINDEX_OFFSET + GPIOI_NUM_PINS;
#else
  static constexpr size_t GPIOJ_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOJ_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOK_PERIPH_AVAILABLE )
  static constexpr size_t GPIOK_PIN_RINDEX_OFFSET = GPIOJ_PIN_RINDEX_OFFSET + GPIOJ_NUM_PINS;
#else
  static constexpr size_t GPIOK_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOK_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

#if defined( STM32_GPIOL_PERIPH_AVAILABLE )
  static constexpr size_t GPIOL_PIN_RINDEX_OFFSET = GPIOK_PIN_RINDEX_OFFSET + GPIOK_NUM_PINS;
#else
  static constexpr size_t GPIOL_PIN_RINDEX_OFFSET = INVALID_RESOURCE_INDEX;
  static constexpr size_t GPIOL_RESOURCE_INDEX    = INVALID_RESOURCE_INDEX;
#endif

  /*-------------------------------------------------------------------------------
  Project Defined Constants
  -------------------------------------------------------------------------------*/
  /*-------------------------------------------------
  These allow the project to describe available GPIOs
  in a highly configurable way.
  -------------------------------------------------*/
  extern const PortAttributes prjPortAttributes[ NUM_GPIO_PERIPHS ];

  /*-------------------------------------------------
  List of all the port addresses, organized by
  resource index.
  -------------------------------------------------*/
  extern const std::uintptr_t prjPortAddress[ NUM_GPIO_PERIPHS ];

  /*-------------------------------------------------------------------------------
  Peripheral Instances:
    Memory mapped structures that allow direct access to peripheral registers
  -------------------------------------------------------------------------------*/
#if defined( STM32_GPIOA_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOA_PERIPH;
#endif
#if defined( STM32_GPIOB_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOB_PERIPH;
#endif
#if defined( STM32_GPIOC_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOC_PERIPH;
#endif
#if defined( STM32_GPIOD_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOD_PERIPH;
#endif
#if defined( STM32_GPIOE_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOE_PERIPH;
#endif
#if defined( STM32_GPIOF_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOF_PERIPH;
#endif
#if defined( STM32_GPIOG_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOG_PERIPH;
#endif
#if defined( STM32_GPIOH_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOH_PERIPH;
#endif
#if defined( STM32_GPIOI_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOI_PERIPH;
#endif
#if defined( STM32_GPIOJ_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOJ_PERIPH;
#endif
#if defined( STM32_GPIOK_PERIPH_AVAILABLE )
  extern RegisterMap *GPIOK_PERIPH;
#endif


  /*-------------------------------------------------------------------------------
  Configuration Maps:
    These convert high level configuration options into low level register config
    options. The idea is to allow the user to specify some general options, then
    convert that over to what the peripheral understands during config/init steps.
  -------------------------------------------------------------------------------*/
  namespace ConfigMap
  {
    extern LLD_CONST Reg32_t PullMap[ static_cast<size_t>( Chimera::GPIO::Pull::NUM_OPTIONS ) ];
    extern LLD_CONST Reg32_t ModeMap[ static_cast<size_t>( Chimera::GPIO::Drive::NUM_OPTIONS ) ];
    extern LLD_CONST Reg32_t SpeedMap[ static_cast<size_t>( Thor::LLD::GPIO::Speed::NUM_OPTIONS ) ];
    extern LLD_CONST Reg32_t PortToIteratorMap[ static_cast<size_t>( Chimera::GPIO::Port::NUM_OPTIONS ) ];
  }    // namespace ConfigMap
}    // namespace Thor::LLD::GPIO

#endif /* THOR_LLD_GPIO */
#endif /* !THOR_LLD_GPIO_DATA_HPP */

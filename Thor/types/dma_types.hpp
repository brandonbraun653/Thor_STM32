/********************************************************************************
 *   File Name:
 *    dma_types.hpp
 *
 *   Description:
 *    Thor DMA types
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_DMA_TYPES_HPP
#define THOR_DMA_TYPES_HPP

/* C++ Includes */
#include <cstdint>

/* Thor Includes */
#include <Thor/headers.hpp>

namespace Thor::DMA
{
  enum class TransferDirection : uint8_t
  {
    PERIPH_TO_MEM,
    MEM_TO_PERIPH,
    MEM_TO_MEM,
    TRANSFER_DIRECTION_UNDEFINED
  };

#if defined( THOR_STM32HAL_DRIVERS ) && ( THOR_STM32HAL_DRIVERS == 1 )
  struct Initializer
  {
    DMA_Stream_TypeDef *Instance;
    uint32_t channel;
    uint32_t direction;
  };
#endif 

}

#endif /* !THOR_DMA_TYPES_HPP */
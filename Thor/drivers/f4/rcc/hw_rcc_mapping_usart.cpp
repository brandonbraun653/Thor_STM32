/********************************************************************************
 *   File Name:
 *    hw_rcc_mapping_usart.cpp
 *
 *   Description:
 *    RCC configuration maps for the USART peripherals
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include <Thor/drivers/f4/rcc/hw_rcc_mapping.hpp>
#include <Thor/drivers/f4/usart/hw_usart_mapping.hpp>

namespace Thor::Driver::RCC::LookupTables
{
/*------------------------------------------------
USART Peripheral RCC Configuration Resources
------------------------------------------------*/
#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_USART == 1 )
  /**
   *  USART clock enable register access lookup table
   *
   *  @note Indexing must match the lookup table in hw_usart_mapping.hpp
   */
  const RegisterConfig USART_ClockConfig[ usartTableSize ] = {
    /* USART1 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB2ENR ) ),
      APB2ENR_USART1EN },
    /* USART2 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB1ENR ) ),
      APB1ENR_USART2EN },
    /* USART3 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB1ENR ) ),
      APB1ENR_USART3EN },
    /* USART6 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB2ENR ) ),
      APB2ENR_USART6EN }
  };

  /**
   *  USART low power clock enable register access lookup table
   *
   *  @note Indexing must match the lookup table in hw_usart_mapping.hpp
   */
  const RegisterConfig USART_ClockConfigLP[ usartTableSize ] = {
    /* USART1 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB2LPENR ) ),
      APB2LPENR_USART1LPEN },
    /* USART2 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB1LPENR ) ),
      APB1LPENR_USART2LPEN },
    /* USART3 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB1LPENR ) ),
      APB1LPENR_USART3LPEN },
    /* USART6 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB2LPENR ) ),
      APB2LPENR_USART6LPEN }
  };

  /**
   *  USART reset register access lookup table
   *
   *  @note Indexing must match the lookup table in hw_usart_mapping.hpp
   */
  const RegisterConfig USART_ResetConfig[ usartTableSize ] = {
    /* USART1 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB2RSTR ) ),
      APB2RSTR_USART1RST },
    /* USART2 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB1RSTR ) ),
      APB1RSTR_USART2RST },
    /* USART3 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB1RSTR ) ),
      APB1RSTR_USART3RST },
    /* USART6 */
    { reinterpret_cast<decltype( RegisterConfig::reg )>( RCC_BASE_ADDR + offsetof( RegisterMap, APB2RSTR ) ),
      APB2RSTR_USART6RST }
  };

  /**
   *  USART clocking bus source identifier
   *
   *  @note Indexing must match the lookup table in hw_usart_mapping.hpp
   */
  const Configuration::ClockType_t USART_SourceClock[ usartTableSize ] = {
    /* USART1 */
    Configuration::ClockType::PCLK2,
    /* USART2 */
    Configuration::ClockType::PCLK1,
    /* USART3 */
    Configuration::ClockType::PCLK1,
    /* USART6 */
    Configuration::ClockType::PCLK2
  };

  const PCC USARTLookup = {
    USART_ClockConfig, USART_ClockConfigLP, USART_ResetConfig, USART_SourceClock, &Thor::Driver::USART::InstanceToResourceIndex,
    usartTableSize
  };

#endif /* TARGET_STM32F4 && THOR_DRIVER_USART */
}    // namespace Thor::Driver::RCC::LookupTables
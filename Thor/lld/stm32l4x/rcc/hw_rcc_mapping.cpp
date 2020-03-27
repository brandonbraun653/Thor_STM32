/********************************************************************************
 *  File Name:
 *    hw_rcc_mapping.cpp
 *
 *  Description:
 *    RCC Mapping Function Declarations
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

/* Driver Includes */
#include <Thor/cfg>
#include <Thor/lld/stm32l4x/rcc/hw_rcc_mapping.hpp>

#if defined( TARGET_STM32L4 ) && defined( THOR_LLD_RCC )

namespace Thor::LLD::RCC
{
  /*------------------------------------------------
  Chip Specific Resources
  ------------------------------------------------*/
  PeriphRegisterList PeripheralList;

  /*------------------------------------------------
  Module Functions
  ------------------------------------------------*/
  void initializeMapping()
  {
#if defined( THOR_LLD_DMA )
#pragma message( "Need to initialize the driver" )
    LookupTables::DMAInit();
#endif

#if defined( THOR_LLD_GPIO )
#pragma message( "Need to initialize the driver" )
    LookupTables::GPIOInit();
#endif

#if defined( THOR_LLD_SPI )
#pragma message( "Need to initialize the driver" )
    LookupTables::SPIInit();
#endif

#if defined( THOR_LLD_UART )
#pragma message( "Need to initialize the driver" )
    LookupTables::UARTInit();
#endif

#if defined( THOR_LLD_USART )
#pragma message( "Need to initialize the driver" )
    LookupTables::USARTInit();
#endif

#if defined( THOR_LLD_WWDG )
#pragma message( "Need to initialize the driver" )
    LookupTables::WWDGInit();
#endif
  }

  bool isRCC( const std::uintptr_t address )
  {
    for ( auto &val : periphAddressList )
    {
      if ( val == address )
      {
        return true;
        break;
      }
    }

    return false;
  }

}    // namespace Thor::LLD::RCC

#endif /* TARGET_STM32L4 && THOR_LLD_RCC */
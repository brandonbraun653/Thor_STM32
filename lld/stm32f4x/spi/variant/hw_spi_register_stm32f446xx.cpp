/********************************************************************************
 *  File Name:
 *    hw_spi_register_stm32f446xx.cpp
 *
 *  Description:
 *    Explicit STM32F446xx SPI data and routines
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include <Thor/cfg>
#include <Thor/hld/dma/hld_dma_intf.hpp>
#include <Thor/lld/stm32f4x/interrupt/hw_it_prj.hpp>
#include <Thor/lld/stm32f4x/rcc/hw_rcc_mapping.hpp>
#include <Thor/lld/stm32f4x/spi/hw_spi_driver.hpp>
#include <Thor/lld/stm32f4x/spi/hw_spi_mapping.hpp>
#include <Thor/lld/stm32f4x/spi/hw_spi_types.hpp>
#include <Thor/lld/stm32f4x/spi/variant/hw_spi_register_stm32f446xx.hpp>

#if defined( TARGET_STM32F4 ) && defined( THOR_LLD_SPI ) && defined( STM32F446xx )

namespace Thor::LLD::SPI
{
#if defined( EMBEDDED )
  RegisterMap *SPI1_PERIPH = reinterpret_cast<RegisterMap *>( SPI1_BASE_ADDR );
  RegisterMap *SPI2_PERIPH = reinterpret_cast<RegisterMap *>( SPI2_BASE_ADDR );
  RegisterMap *SPI3_PERIPH = reinterpret_cast<RegisterMap *>( SPI3_BASE_ADDR );
  RegisterMap *SPI4_PERIPH = reinterpret_cast<RegisterMap *>( SPI4_BASE_ADDR );

  Chimera::Container::LightFlatMap<std::uintptr_t, size_t> InstanceToResourceIndex{
    { reinterpret_cast<std::uintptr_t>( SPI1_PERIPH ), SPI1_RESOURCE_INDEX },
    { reinterpret_cast<std::uintptr_t>( SPI2_PERIPH ), SPI2_RESOURCE_INDEX },
    { reinterpret_cast<std::uintptr_t>( SPI3_PERIPH ), SPI3_RESOURCE_INDEX },
    { reinterpret_cast<std::uintptr_t>( SPI4_PERIPH ), SPI4_RESOURCE_INDEX }
  };

  Chimera::Container::LightFlatMap<size_t, RegisterMap *> ChannelToInstance{ { SPI1_CHANNEL_NUMBER, SPI1_PERIPH },
                                                                             { SPI2_CHANNEL_NUMBER, SPI2_PERIPH },
                                                                             { SPI3_CHANNEL_NUMBER, SPI3_PERIPH },
                                                                             { SPI4_CHANNEL_NUMBER, SPI4_PERIPH } };

#elif defined( _SIM )
  RegisterMap *SPI1_PERIPH = nullptr;
  RegisterMap *SPI2_PERIPH = nullptr;
  RegisterMap *SPI3_PERIPH = nullptr;
  RegisterMap *SPI4_PERIPH = nullptr;

  Chimera::Container::LightFlatMap<std::uintptr_t, size_t> InstanceToResourceIndex;
  Chimera::Container::LightFlatMap<size_t, RegisterMap *> ChannelToInstance;
#endif

  const std::array<uint8_t, NUM_SPI_PERIPHS> supportedChannels = { SPI1_CHANNEL_NUMBER, SPI2_CHANNEL_NUMBER,
                                                                   SPI3_CHANNEL_NUMBER, SPI4_CHANNEL_NUMBER };

  const DMASignalList RXDMASignals = {
    Thor::DMA::Source::S_SPI1_RX,
    Thor::DMA::Source::S_SPI2_RX,
    Thor::DMA::Source::S_SPI3_RX,
    Thor::DMA::Source::S_SPI4_RX,
  };

  const DMASignalList TXDMASignals = {
    Thor::DMA::Source::S_SPI1_TX,
    Thor::DMA::Source::S_SPI2_TX,
    Thor::DMA::Source::S_SPI3_TX,
    Thor::DMA::Source::S_SPI4_TX,
  };

  const IRQSignalList IRQSignals = { SPI1_IRQn, SPI2_IRQn, SPI3_IRQn, SPI4_IRQn };


  {
#if defined( _SIM )
    /*------------------------------------------------
    Allocate some memory to simulate the register blocks
    ------------------------------------------------*/
    SPI1_PERIPH = new RegisterMap;
    SPI2_PERIPH = new RegisterMap;
    SPI3_PERIPH = new RegisterMap;
    SPI4_PERIPH = new RegisterMap;

    /*------------------------------------------------
    Update the resource indexer now that the registers actually exist
    ------------------------------------------------*/
    InstanceToResourceIndex.append( reinterpret_cast<std::uintptr_t>( SPI1_PERIPH ), SPI1_RESOURCE_INDEX );
    InstanceToResourceIndex.append( reinterpret_cast<std::uintptr_t>( SPI2_PERIPH ), SPI2_RESOURCE_INDEX );
    InstanceToResourceIndex.append( reinterpret_cast<std::uintptr_t>( SPI3_PERIPH ), SPI3_RESOURCE_INDEX );
    InstanceToResourceIndex.append( reinterpret_cast<std::uintptr_t>( SPI4_PERIPH ), SPI4_RESOURCE_INDEX );

    ChannelToInstance.append( SPI1_CHANNEL_NUMBER, SPI1_PERIPH );
    ChannelToInstance.append( SPI2_CHANNEL_NUMBER, SPI2_PERIPH );
    ChannelToInstance.append( SPI3_CHANNEL_NUMBER, SPI3_PERIPH );
    ChannelToInstance.append( SPI4_CHANNEL_NUMBER, SPI4_PERIPH );
#endif

    /*------------------------------------------------
    Update the memory listing
    ------------------------------------------------*/
    PeripheralList[ SPI1_RESOURCE_INDEX ] = SPI1_PERIPH;
    PeripheralList[ SPI2_RESOURCE_INDEX ] = SPI2_PERIPH;
    PeripheralList[ SPI3_RESOURCE_INDEX ] = SPI3_PERIPH;
    PeripheralList[ SPI4_RESOURCE_INDEX ] = SPI4_PERIPH;
  }
}    // namespace Thor::LLD::SPI

#endif /* TARGET_STM32F4 && THOR_DRIVER_SPI */
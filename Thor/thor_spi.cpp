/********************************************************************************
 *   File Name:
 *    thor_spi.cpp
 *
 *   Description:
 *    Thor SPI Interface
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C/C++ Includes */
#include <cstdlib>
#include <math.h>

/* Boost Includes */
#include <boost/bind.hpp>

/* Thor Includes */
#include <Thor/spi.hpp>
#include <Thor/defaults/spi_defaults.hpp>
#include <Thor/definitions/spi_definitions.hpp>



/* Mock Includes */
#if defined( GMOCK_TEST )
#include "mock_stm32_hal_spi.hpp"
#endif

#if defined( USING_FREERTOS )
#ifdef __cplusplus
extern "C"
{
#endif

#include "FreeRTOS.h"
#include "semphr.h"

#ifdef __cplusplus
}
#endif
#endif

using namespace Thor;
using namespace Thor::SPI;
using namespace Thor::GPIO;
using namespace Thor::Interrupt;

/*------------------------------------------------
Stores references to available SPIClass objects
-------------------------------------------------*/
static SPIClass_sPtr spiObjects[ MAX_SPI_CHANNELS + 1 ];

/*------------------------------------------------
Directly maps the HAL SPI Instance pointer to a possible SPIClass object
-------------------------------------------------*/
static const SPIClass_sPtr &getSPIClassRef( SPI_TypeDef *instance )
{
  auto i = reinterpret_cast<std::uintptr_t>( instance );

  switch ( i )
  {
#if defined( SPI1 )
    case SPI1_BASE:
      return spiObjects[ 1 ];
      break;
#endif
#if defined( SPI2 )
    case SPI2_BASE:
      return spiObjects[ 2 ];
      break;
#endif
#if defined( SPI3 )
    case SPI3_BASE:
      return spiObjects[ 3 ];
      break;
#endif
#if defined( SPI4 )
    case SPI4_BASE:
      return spiObjects[ 4 ];
      break;
#endif
#if defined( SPI5 )
    case SPI5_BASE:
      return spiObjects[ 5 ];
      break;
#endif
#if defined( SPI6 )
    case SPI6_BASE:
      return spiObjects[ 6 ];
      break;
#endif

    /* If we get here, something went wrong and the program will likely crash */
    default:
      return spiObjects[ 0 ];
      break;
  };
}

/*------------------------------------------------
Directly maps the HAL SPI Instance pointer to the correct bit mask for
enabling/disabling the peripheral clock
-------------------------------------------------*/
static uint32_t spiClockMask( SPI_TypeDef *instance )
{
  auto i = reinterpret_cast<std::uintptr_t>( instance );

  switch ( i )
  {
#if defined( TARGET_STM32F4 ) || defined( TARGET_STM32F7 )
#if defined( SPI1 )
    case SPI1_BASE:
      return RCC_APB2ENR_SPI1EN;
      break;
#endif
#if defined( SPI2 )
    case SPI2_BASE:
      return RCC_APB1ENR_SPI2EN;
      break;
#endif
#if defined( SPI3 )
    case SPI3_BASE:
      return RCC_APB1ENR_SPI3EN;
      break;
#endif
#if defined( SPI4 )
    case SPI4_BASE:
      return RCC_APB2ENR_SPI4EN;
      break;
#endif
#endif

#if defined( TARGET_STM32F7 )
#if defined( SPI5 )
    case SPI5_BASE:
      return RCC_APB2ENR_SPI5EN;
      break;
#endif
#if defined( SPI6 )
    case SPI6_BASE:
      return RCC_APB2ENR_SPI6EN;
      break;
#endif
#endif /* !TARGET_STM32F7 */

    /* If we get here, something went wrong */
    default:
      return 0u;
      break;
  };
}

/*------------------------------------------------
Directly maps the HAL SPI Instance pointer to the correct register for
enabling/disabling the peripheral clock.
-------------------------------------------------*/
static volatile uint32_t *spiClockRegister( SPI_TypeDef *instance )
{
  auto i = reinterpret_cast<std::uintptr_t>( instance );

  switch ( i )
  {
#if defined( TARGET_STM32F4 ) || defined( TARGET_STM32F7 )
#if defined( SPI1 )
    case SPI1_BASE:
      return &( RCC->APB2ENR );
      break;
#endif
#if defined( SPI2 )
    case SPI2_BASE:
      return &( RCC->APB1ENR );
      break;
#endif
#if defined( SPI3 )
    case SPI3_BASE:
      return &( RCC->APB1ENR );
      break;
#endif
#if defined( SPI4 )
    case SPI4_BASE:
      return &( RCC->APB2ENR );
      break;
#endif
#endif

#if defined( TARGET_STM32F7 )
#if defined( SPI5 )
    case SPI5_BASE:
      return &( RCC->APB2ENR );
      break;
#endif
#if defined( SPI6 )
    case SPI6_BASE:
      return &( RCC->APB2ENR );
      break;
#endif
#endif /* !TARGET_STM32F7 */

    /* If we get here, something went wrong */
    default:
      return nullptr;
      break;
  };
}

#if 0
namespace Thor::SPI
{
  SPIClass::SPIClass( const uint8_t channel )
  {
    alternateCS = false;

    spi_channel         = channel;
    spi_handle.Instance = hwConfig[ spi_channel ]->instance;
    spi_handle.Init     = dflt_SPI_Init;

    /*------------------------------------
    Interrupt
    ------------------------------------*/
    ITSettingsHW.IRQn            = hwConfig[ spi_channel ]->IT_HW.IRQn;
    ITSettingsHW.preemptPriority = hwConfig[ spi_channel ]->IT_HW.preemptPriority;
    ITSettingsHW.subPriority     = hwConfig[ spi_channel ]->IT_HW.subPriority;

    ITSettings_DMA_TX.IRQn            = hwConfig[ spi_channel ]->dmaIT_TX.IRQn;
    ITSettings_DMA_TX.preemptPriority = hwConfig[ spi_channel ]->dmaIT_TX.preemptPriority;
    ITSettings_DMA_TX.subPriority     = hwConfig[ spi_channel ]->dmaIT_TX.subPriority;

    ITSettings_DMA_RX.IRQn            = hwConfig[ spi_channel ]->dmaIT_RX.IRQn;
    ITSettings_DMA_RX.preemptPriority = hwConfig[ spi_channel ]->dmaIT_RX.preemptPriority;
    ITSettings_DMA_RX.subPriority     = hwConfig[ spi_channel ]->dmaIT_RX.subPriority;

    /*------------------------------------
    DMA
    ------------------------------------*/
    hdma_spi_tx.Init = dflt_DMA_Init_TX;
    hdma_spi_rx.Init = dflt_DMA_Init_RX;

    hdma_spi_tx.Init.Channel = hwConfig[ spi_channel ]->dmaTX.channel;
    hdma_spi_rx.Init.Channel = hwConfig[ spi_channel ]->dmaRX.channel;
                               
    hdma_spi_tx.Instance = hwConfig[ spi_channel ]->dmaTX.Instance;
    hdma_spi_rx.Instance = hwConfig[ spi_channel ]->dmaRX.Instance;
    
#if defined( GMOCK_TEST )
    if ( !STM32HAL_Mock::spiMockObj )
    {
      STM32HAL_Mock::spiMockObj = std::make_shared<STM32HAL_Mock::SPINiceMock>();
    }
#endif /* GMOCK_TEST */
  }

  const SPIClass_sPtr SPIClass::create( const uint8_t channel )
  {
    /*------------------------------------------------
  Create a new object if none already there
  -------------------------------------------------*/
    if ( !spiObjects[ channel ] )
    {
      /*------------------------------------------------
  Forced to create the new object in this manner due to the
  private constructor not being accessible by Boost
  -------------------------------------------------*/
      boost::shared_ptr<SPIClass> newClass( new SPIClass( channel ) );
      spiObjects[ channel ] = newClass;
    }

    /*------------------------------------------------
  Intentionally force the shared_ptr reference count to be incremented.
  The internally managed object shall not be deleted!
  -------------------------------------------------*/
    return spiObjects[ channel ];
  }

  Chimera::Status_t SPIClass::init( const Chimera::SPI::Setup &setupStruct )
  {
    
  }

  Chimera::Status_t SPIClass::deInit()
  {
  }


  Chimera::Status_t SPIClass::setChipSelect( const Chimera::GPIO::State &value )
  {
  }

  Chimera::Status_t SPIClass::setChipSelectControlMode( const Chimera::SPI::ChipSelectMode &mode )
  {
    chipSelectMode = mode;
    return Chimera::CommonStatusCodes::OK;
  }

  Chimera::Status_t SPIClass::writeBytes( const uint8_t *const txBuffer, const size_t length, const uint32_t timeoutMS )
  {
  }

  Chimera::Status_t SPIClass::readBytes( uint8_t *const rxBuffer, const size_t length, const uint32_t timeoutMS )
  {
  }

  Chimera::Status_t SPIClass::readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, const size_t length,
                                    const uint32_t timeoutMS )
  {
    Chimera::Status_t error = Chimera::CommonStatusCodes::OK;

    if ( !hardwareStatus.gpio_enabled || !hardwareStatus.spi_enabled )
    {
      error = Chimera::CommonStatusCodes::NOT_INITIALIZED;
    }
    else if ( !length )
    {
      error = Chimera::CommonStatusCodes::INVAL_FUNC_PARAM;
    }
    else
    {
      switch ( txMode )
      {
        case Chimera::SPI::SubPeripheralMode::BLOCKING:
          error = transfer_blocking( txBuffer, rxBuffer, length, timeoutMS );
          break;

        case Chimera::SPI::SubPeripheralMode::INTERRUPT:
          error = transfer_interrupt( txBuffer, rxBuffer, length);
          break;

        case Chimera::SPI::SubPeripheralMode::DMA:
          error = transfer_dma( txBuffer, rxBuffer, length );
          break;

        default:
          error = Chimera::CommonStatusCodes::FAIL;
          break;
      }
    }

    return error;
  }

  Chimera::Status_t SPIClass::setPeripheralMode( const Chimera::SPI::SubPeripheral periph, const Chimera::SPI::SubPeripheralMode mode )
  {
  }

  Chimera::Status_t SPIClass::setClockFrequency( const uint32_t freq, const uint32_t tolerance )
  {
  }

  Chimera::Status_t SPIClass::getClockFrequency( uint32_t &freq )
  {
  }

  Chimera::Status_t SPIClass::onWriteCompleteCallback( const Chimera::Function::void_func_uint32_t func )
  {
  }

  Chimera::Status_t SPIClass::onReadCompleteCallback( const Chimera::Function::void_func_uint32_t func )
  {
  }

  Chimera::Status_t SPIClass::onReadWriteCompleteCallback( const Chimera::Function::void_func_uint32_t func )
  {
  }

  Chimera::Status_t SPIClass::onErrorCallback( const Chimera::Function::void_func_uint32_t func )
  {
  }

  Chimera::Status_t SPIClass::transfer_blocking( const uint8_t *const txBuffer, uint8_t *const rxBuffer, const size_t length,
                                                 const uint32_t timeoutMS )
  {
  }

  Chimera::Status_t SPIClass::transfer_interrupt( const uint8_t *const txBuffer, uint8_t *const rxBuffer, const size_t length )
  {
  }

  Chimera::Status_t SPIClass::transfer_dma( const uint8_t *const txBuffer, uint8_t *const rxBuffer, const size_t length )
  {
  }


  Thor::Status SPIClass::begin( const Config &settings, const bool &force )
  {
    /*------------------------------------
  Create the GPIO instances depending on what the user supplied
  ------------------------------------*/
    if ( settings.MOSI.pinNum == PinNum::NOT_A_PIN )
    {
      /* Default pin as described in defaults.cpp for this channel */
      MOSI = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      MOSI->initAdvanced( spi_cfg[ spi_channel ].MOSI.GPIOx, spi_cfg[ spi_channel ].MOSI.PinNum,
                          spi_cfg[ spi_channel ].MOSI.Speed, spi_cfg[ spi_channel ].MOSI.Alternate );
    }
    else
    {
      /* User supplied pin */
      MOSI = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      MOSI->initAdvanced( settings.MOSI.GPIOx, settings.MOSI.pinNum, settings.MOSI.speed, settings.MOSI.alternate );
    }

    if ( settings.MISO.pinNum == PinNum::NOT_A_PIN )
    {
      /* Default pin as described in defaults.cpp for this channel */
      MISO = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      MISO->initAdvanced( spi_cfg[ spi_channel ].MISO.GPIOx, spi_cfg[ spi_channel ].MISO.PinNum,
                          spi_cfg[ spi_channel ].MISO.Speed, spi_cfg[ spi_channel ].MISO.Alternate );
    }
    else
    {
      /* User supplied pin */
      MISO = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      MISO->initAdvanced( settings.MISO.GPIOx, settings.MISO.pinNum, settings.MISO.speed, settings.MISO.alternate );
    }

    if ( settings.SCK.pinNum == PinNum::NOT_A_PIN )
    {
      /* Default pin as described in defaults.cpp for this channel */
      SCK = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      SCK->initAdvanced( spi_cfg[ spi_channel ].SCK.GPIOx, spi_cfg[ spi_channel ].SCK.PinNum, spi_cfg[ spi_channel ].SCK.Speed,
                         spi_cfg[ spi_channel ].SCK.Alternate );
    }
    else
    {
      /* User supplied pin */
      SCK = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      SCK->initAdvanced( settings.SCK.GPIOx, settings.SCK.pinNum, settings.SCK.speed, settings.SCK.alternate );
    }

    if ( settings.CS.pinNum == PinNum::NOT_A_PIN )
    {
      /* Default pin as described in defaults.cpp for this channel */
      CS = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      CS->initAdvanced( spi_cfg[ spi_channel ].NSS.GPIOx, spi_cfg[ spi_channel ].NSS.PinNum, spi_cfg[ spi_channel ].NSS.Speed,
                        spi_cfg[ spi_channel ].NSS.Alternate );
    }
    else
    {
      /* User supplied pin */
      CS = boost::movelib::unique_ptr<GPIOClass>( new GPIOClass() );
      CS->initAdvanced( settings.CS.GPIOx, settings.CS.pinNum, settings.CS.speed, settings.CS.alternate );

      alternateCS = true;
    }

    spi_handle.Init = settings.settings;

    SPI_GPIO_Init();
    SPI_Init();

    setMode( SubPeripheral::TX, Modes::BLOCKING );
    setMode( SubPeripheral::RX, Modes::BLOCKING );

    return Thor::Status::PERIPH_OK;
  }

  Thor::Status SPIClass::end()
  {
    SPI_GPIO_DeInit();
    SPI_DisableInterrupts();
    SPI_DMA_DeInit( SubPeripheral::TX );
    SPI_DMA_DeInit( SubPeripheral::RX );

    txMode = Modes::MODE_UNDEFINED;
    rxMode = Modes::MODE_UNDEFINED;

    return Thor::Status::PERIPH_OK;
  }

  Thor::Status SPIClass::reserve( const uint32_t &timeout_ms )
  {
    Thor::Status status = Thor::Status::PERIPH_LOCKED;

    /*------------------------------------------------
  If not locked, grab immediately
  -------------------------------------------------*/
    if ( !isLocked() )
    {
      lock( true );
      status = Thor::Status::PERIPH_OK;
    }
    else if ( timeout_ms )
    {
/*------------------------------------------------
Otherwise, block the current task for a bit and check
again when unblocked.
-------------------------------------------------*/
#if defined( USING_FREERTOS )
      vTaskDelay( pdMS_TO_TICKS( timeout_ms ) );

      if ( !isLocked() )
      {
        lock( true );
        status = Thor::Status::PERIPH_OK;
      }
#endif
    }

    return status;
  }

  Thor::Status SPIClass::release( const uint32_t &timeout_ms )
  {
    Thor::Status status = Thor::Status::PERIPH_TIMEOUT;

    if ( isOwner( ownerID ) )
    {
      if ( isTransferComplete() )
      {
        unlock();
        status = Thor::Status::PERIPH_OK;
      }
      else if ( timeout_ms )
      {
#if defined( USING_FREERTOS )
        vTaskDelay( pdMS_TO_TICKS( timeout_ms ) );

        if ( isTransferComplete() )
        {
          unlock();
          status = Thor::Status::PERIPH_OK;
        }
#endif
      }
    }
    else
    {
      status = Thor::Status::NOT_OWNER;
    }

    return status;
  }

  Thor::Status SPIClass::attachChipSelect( const Thor::Peripheral::GPIO::GPIOClass_sPtr &chipSelect )
  {
    externalCS = chipSelect;
    return Thor::Status::PERIPH_OK;
  }

  Thor::Status SPIClass::detachChipSelect()
  {
    externalCS = nullptr;
    return Thor::Status::PERIPH_OK;
  }

  Thor::Status SPIClass::writeBytes( const uint8_t *const txBuffer, size_t length, const bool &autoDisableCS,
                                     const bool &autoRelease, uint32_t timeoutMS )
  {
    return readWriteBytes( txBuffer, nullptr, length, autoDisableCS, autoRelease, timeoutMS );
  }

  Thor::Status SPIClass::readBytes( uint8_t *const rxBuffer, size_t length, const bool &autoDisableCS, const bool &autoRelease,
                                    uint32_t timeoutMS )
  {
    return readWriteBytes( nullptr, rxBuffer, length, autoDisableCS, autoRelease, timeoutMS );
  }


  Thor::Status SPIClass::setMode( const SubPeripheral &periph, const Modes &mode )
  {
    if ( ( periph == SubPeripheral::TXRX ) || ( periph == SubPeripheral::TX ) )
    {
      switch ( mode )
      {
        case Modes::BLOCKING:
          txMode = mode;

          /*------------------------------------------------
    Disable interrupts as both periphs are now in blocking mode
    -------------------------------------------------*/
          if ( rxMode == Modes::BLOCKING )
          {
            SPI_DisableInterrupts();
          }

          SPI_DMA_DeInit( SubPeripheral::TX );
          break;

        case Modes::INTERRUPT:
          txMode = mode;

          SPI_EnableInterrupts();
          SPI_DMA_DeInit( SubPeripheral::TX );
          break;

        case Modes::DMA:
          txMode = mode;

          SPI_EnableInterrupts();
          SPI_DMA_Init( SubPeripheral::TX );
          break;

        default:
          txMode = Modes::MODE_UNDEFINED;
          return Status::PERIPH_ERROR;
          break;
      }
    }

    if ( ( periph == SubPeripheral::TXRX ) || ( periph == SubPeripheral::RX ) )
    {
      switch ( mode )
      {
        case Modes::BLOCKING:
          rxMode = mode;

          /*------------------------------------------------
    Disable interrupts as both periphs are now in blocking mode
    -------------------------------------------------*/
          if ( txMode == Modes::BLOCKING )
          {
            SPI_DisableInterrupts();
          }

          SPI_DMA_DeInit( SubPeripheral::RX );
          break;

        case Modes::INTERRUPT:
          rxMode = mode;

          SPI_EnableInterrupts();
          SPI_DMA_DeInit( SubPeripheral::RX );
          break;

        case Modes::DMA:
          rxMode = mode;

          SPI_EnableInterrupts();
          SPI_DMA_Init( SubPeripheral::RX );
          break;

        default:
          rxMode = Modes::MODE_UNDEFINED;
          return Status::PERIPH_ERROR;
          break;
      }
    }

    return Thor::Status::PERIPH_OK;
  }

  Thor::Status SPIClass::setChipSelect( const LogicLevel &state )
  {
    if ( externalCS )
    {
      /*------------------------------------
  External GPIOClass object supplied by the user
  ------------------------------------*/
      externalCS->write( state );
    }
    else if ( alternateCS )
    {
      /*------------------------------------
  Internal GPIOClass object that has different settings than
  the default for this channel.
  ------------------------------------*/
      CS->write( state );
    }
    else
    {
      /*------------------------------------
  Default pin for this channel. We can only control setting it HIGH.
  ------------------------------------*/

      /**
       *  This is only useful when the peripheral's dedicated NSS pin is used,
       *configured for hardware management, and the SSOE bit in CR1 is set. (By
       *default this is how Thor is set up) In this configuration, the NSS pin is
       *automatically driven low upon transmission start and will STAY low until
       *the SPI hardware is disabled. Rather quirky...
       *
       *  Note:   Don't bother trying to use software NSS management as described
       *in the datasheet. It will cause a nasty mode fault if you are in master
       *mode.
       */
      if ( static_cast<bool>( state ) )
      {
        __HAL_SPI_DISABLE( &spi_handle );
      }
    }

    return Thor::Status::PERIPH_OK;
  }


  Thor::Status SPIClass::setClockFrequency( const uint32_t &freq )
  {
    Thor::Status status = Thor::Status::PERIPH_ERROR;

    if ( isTransferComplete() && isAvailable( ownerID ) && ( reserve() == Thor::Status::PERIPH_OK ) )
    {
      // TODO: Figure out how to do this without resetting the channel
      // spi_handle.Init.BaudRatePrescaler = getPrescaler(spi_channel, freq);

      release();
    }
    else
    {
      status = Thor::Status::PERIPH_BUSY;
    }

    return status;
  }

  Thor::Status SPIClass::getClockFrequency( uint32_t *const freq )
  {
    *freq = getFrequency( spi_channel, spi_handle.Init.BaudRatePrescaler );
    return Thor::Status::PERIPH_OK;
  }

  
  
  
  
  
  
  void SPIClass::IRQHandler()
  {
    HAL_SPI_IRQHandler( &spi_handle );
  }

  void SPIClass::IRQHandler_TXDMA()
  {
    HAL_DMA_IRQHandler( spi_handle.hdmatx );
  }

  void SPIClass::IRQHandler_RXDMA()
  {
    HAL_DMA_IRQHandler( spi_handle.hdmarx );
  }

  uint32_t SPIClass::getPrescaler( const int &channel, const uint32_t &freq )
  {
    uint32_t busFreq            = 0u;
    uint32_t prescaler          = 0u;
    const uint8_t numPrescalers = 8;

    /*------------------------------------------------
  Figure out the APB bus frequency for this channel
  -------------------------------------------------*/
    if ( spi_cfg[ channel ].clockBus == Thor::ClockBus::APB1_PERIPH )
    {
      busFreq = HAL_RCC_GetPCLK1Freq();
    }
    else if ( spi_cfg[ channel ].clockBus == Thor::ClockBus::APB2_PERIPH )
    {
      busFreq = HAL_RCC_GetPCLK2Freq();
    }
    else
    {
      return Thor::Defaults::SPI::dflt_SPI_Init.BaudRatePrescaler;
    }

    /*------------------------------------------------
  Calculate the error between the resultant pre-scaled clock and the desired
  clock
  -------------------------------------------------*/
    int clockError[ numPrescalers ];
    memset( clockError, INT_MAX, numPrescalers * sizeof( clockError ) );

    for ( int i = 0; i < numPrescalers; i++ )
    {
      clockError[ i ] = abs( static_cast<int>( ( busFreq / ( 1 << ( i + 1 ) ) - freq ) ) );
    }

    /*------------------------------------------------
  Find the index of the element with lowest error
  -------------------------------------------------*/
    auto idx = std::distance( clockError, std::min_element( clockError, clockError + numPrescalers - 1 ) );

    /*------------------------------------------------
  Grab the correct prescaler
  -------------------------------------------------*/
    switch ( idx )
    {
      case 0:
        prescaler = SPI_BAUDRATEPRESCALER_2;
        break;

      case 1:
        prescaler = SPI_BAUDRATEPRESCALER_4;
        break;

      case 2:
        prescaler = SPI_BAUDRATEPRESCALER_8;
        break;

      case 3:
        prescaler = SPI_BAUDRATEPRESCALER_16;
        break;

      case 4:
        prescaler = SPI_BAUDRATEPRESCALER_32;
        break;

      case 5:
        prescaler = SPI_BAUDRATEPRESCALER_64;
        break;

      case 6:
        prescaler = SPI_BAUDRATEPRESCALER_128;
        break;

      case 7:
        prescaler = SPI_BAUDRATEPRESCALER_256;
        break;

      default:
        prescaler = Thor::Defaults::SPI::dflt_SPI_Init.BaudRatePrescaler;
        break;
    };

    return prescaler;
  }

  uint32_t SPIClass::getFrequency( const int &channel, const uint32_t &prescaler )
  {
    uint32_t apbFreq = 0u;
    uint32_t busFreq = 0u;

    /*------------------------------------------------
  Figure out the APB bus frequency for this channel
  -------------------------------------------------*/
    if ( spi_cfg[ channel ].clockBus == Thor::ClockBus::APB1_PERIPH )
    {
      apbFreq = HAL_RCC_GetPCLK1Freq();
    }
    else if ( spi_cfg[ channel ].clockBus == Thor::ClockBus::APB2_PERIPH )
    {
      apbFreq = HAL_RCC_GetPCLK2Freq();
    }
    else
    {
      return 0u;
    }

    /*------------------------------------------------
  Now get the SPI clock frequency
  -------------------------------------------------*/
    switch ( prescaler )
    {
      case SPI_BAUDRATEPRESCALER_2:
        busFreq = apbFreq / 2;
        break;

      case SPI_BAUDRATEPRESCALER_4:
        busFreq = apbFreq / 4;
        break;

      case SPI_BAUDRATEPRESCALER_8:
        busFreq = apbFreq / 8;
        break;

      case SPI_BAUDRATEPRESCALER_16:
        busFreq = apbFreq / 16;
        break;

      case SPI_BAUDRATEPRESCALER_32:
        busFreq = apbFreq / 32;
        break;

      case SPI_BAUDRATEPRESCALER_64:
        busFreq = apbFreq / 64;
        break;

      case SPI_BAUDRATEPRESCALER_128:
        busFreq = apbFreq / 128;
        break;

      case SPI_BAUDRATEPRESCALER_256:
        busFreq = apbFreq / 256;
        break;

      default:
        busFreq = 0u;
    };

    return busFreq;
  }

  Thor::Status SPIClass::transfer_blocking( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                            const bool &autoDisableCS, const bool &autoRelease, uint32_t timeoutMS )
  {
    HAL_StatusTypeDef error = HAL_OK;
    Thor::Status status     = Thor::Status::PERIPH_OK;

#if defined( USING_FREERTOS )
    timeoutMS = pdMS_TO_TICKS( timeoutMS );
#endif

    /*------------------------------------------------
  Ensure the previous transmission has completed
  -------------------------------------------------*/
    if ( isAvailable( ownerID ) )
    {
      /*------------------------------------------------
  Lock the hardware to make sure we don't get interrupted
  -------------------------------------------------*/
      if ( !isExternallyReserved() )
      {
        lock( false );
      }

      /*------------------------------------------------
  Activate the chip select line?
  -------------------------------------------------*/
      if ( ( chipSelectMode < Thor::SPI::ChipSelectMode::NUM_CS_MODES ) &&
           ( chipSelectMode != Thor::SPI::ChipSelectMode::MANUAL ) )
      {
        setChipSelect( LogicLevel::LOW );
      }

      /*------------------------------------------------
  Execute the transfer
  -------------------------------------------------*/
      if ( txBuffer && !rxBuffer )
      {
        /*------------------------------------------------
          The HAL operation doesn't modify the transmit buffer, but
  for some reason the designers did not add const to the function
  signature. This feels nasty, but it must be done.
          -------------------------------------------------*/
        error = HAL_SPI_Transmit( &spi_handle, const_cast<uint8_t *>( txBuffer ), length, timeoutMS );
      }
      else if ( rxBuffer && !txBuffer )
      {
        /*------------------------------------------------
  In order for TransmitReceive to work, both subperipherals must be in the
  same mode. This will silently clobber whatever settings were previously
  there.
  -------------------------------------------------*/
        if ( rxMode != txMode )
        {
          setMode( SubPeripheral::RX, txMode );
        }

        /*------------------------------------------------
          The HAL operation doesn't modify the transmit buffer, but
  for some reason the designers did not add const to the function
  signature. This feels nasty, but it must be done.
          -------------------------------------------------*/
        error = HAL_SPI_TransmitReceive( &spi_handle, const_cast<uint8_t *>( rxBuffer ), const_cast<uint8_t *>( rxBuffer ),
                                         length, timeoutMS );
      }
      else if ( txBuffer && rxBuffer )
      {
        /*------------------------------------------------
  In order for TransmitReceive to work, both subperipherals must be in the
  same mode. This will silently clobber whatever settings were previously
  there.
  -------------------------------------------------*/
        if ( rxMode != txMode )
        {
          setMode( SubPeripheral::RX, txMode );
        }

        /*------------------------------------------------
          The HAL operation doesn't modify the transmit buffer, but
  for some reason the designers did not add const to the function
  signature. This feels nasty, but it must be done.
          -------------------------------------------------*/
        error = HAL_SPI_TransmitReceive( &spi_handle, const_cast<uint8_t *>( txBuffer ), const_cast<uint8_t *>( rxBuffer ),
                                         length, timeoutMS );
      }
      else
      {
        status = Thor::Status::PERIPH_INVALID_PARAM;
      }

      /*------------------------------------------------
  De-activate the chip select line?
  -------------------------------------------------*/
      if ( autoDisableCS )
      {
        setChipSelect( LogicLevel::HIGH );
      }

      /*------------------------------------------------
  Unlock since we are done with the hardware
  -------------------------------------------------*/
      if ( !isExternallyReserved() || autoRelease )
      {
        unlock();
      }

      /*------------------------------------------------
  Handle any errors and exit gracefully
  -------------------------------------------------*/
      if ( error != HAL_OK )
      {
        status = Thor::Status::PERIPH_ERROR;
      }
    }
    else
    {
      status = Thor::Status::PERIPH_BUSY;
    }

    return status;
  }

  Thor::Status SPIClass::transfer_interrupt( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                             const bool &autoDisableCS, const bool &autoRelease )
  {
    HAL_StatusTypeDef error = HAL_OK;
    Thor::Status status     = Thor::Status::PERIPH_OK;

    /*------------------------------------------------
  Ensure the previous transmission has completed
  -------------------------------------------------*/
    if ( isAvailable( ownerID ) )
    {
      /*------------------------------------------------
  Lock the hardware to make sure we don't get interrupted
  -------------------------------------------------*/
      if ( !isExternallyReserved() )
      {
        lock( false );
      }

      /*------------------------------------------------
  Activate the chip select line?
  -------------------------------------------------*/
      if ( ( chipSelectMode < Thor::SPI::ChipSelectMode::NUM_CS_MODES ) &&
           ( chipSelectMode != Thor::SPI::ChipSelectMode::MANUAL ) )
      {
        setChipSelect( LogicLevel::LOW );
      }

      /*------------------------------------------------
  Start the transfer
  -------------------------------------------------*/
      channelState[ spi_channel ].transfer_complete = false;
      channelState[ spi_channel ].auto_unlock       = autoRelease;
      channelState[ spi_channel ].auto_disable_cs   = autoDisableCS;

      if ( txBuffer && !rxBuffer )
      {
        /*------------------------------------------------
  The HAL operation doesn't modify the transmit buffer, but for some reason
  the designers did not add const to the function signature. This feels
  nasty, but it must be done.
  -------------------------------------------------*/
        error  = HAL_SPI_Transmit_IT( &spi_handle, const_cast<uint8_t *>( txBuffer ), length );
        status = Thor::Status::PERIPH_TX_IN_PROGRESS;
      }
      else if ( txBuffer && rxBuffer )
      {
        /*------------------------------------------------
  In order for TransmitReceive to work, both subperipherals must be in the
  same mode. This will silently clobber whatever settings were previously
  there.
  -------------------------------------------------*/
        if ( rxMode != txMode )
        {
          setMode( SubPeripheral::RX, txMode );
        }

        status = Thor::Status::PERIPH_TXRX_IN_PROGRESS;

        /*------------------------------------------------
          The HAL operation doesn't modify the transmit buffer, but
  for some reason the designers did not add const to the function
  signature. This feels nasty, but it must be done.
          -------------------------------------------------*/
        error = HAL_SPI_TransmitReceive_IT( &spi_handle, const_cast<uint8_t *>( txBuffer ), const_cast<uint8_t *>( rxBuffer ),
                                            length );
      }
      else if ( rxBuffer && !txBuffer )
      {
        status = Thor::Status::PERIPH_ERROR;
      }
      else
      {
        resetISRFlags();
        status = Thor::Status::PERIPH_INVALID_PARAM;
      }

      /*------------------------------------------------
  Catch any errors and exit gracefully
  -------------------------------------------------*/
      if ( error != HAL_OK )
      {
        resetISRFlags();
        status = Thor::Status::PERIPH_ERROR;

        if ( autoDisableCS )
        {
          setChipSelect( LogicLevel::HIGH );
        }

        if ( !isExternallyReserved() )
        {
          unlock();
        }
      }
    }
    else
    {
      status = Thor::Status::PERIPH_BUSY;
    }

    return status;
  }

  Thor::Status SPIClass::transfer_dma( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                       const bool &autoDisableCS, const bool &autoRelease )
  {
    HAL_StatusTypeDef error = HAL_OK;
    Thor::Status status     = Thor::Status::PERIPH_OK;

    /*------------------------------------------------
  Ensure the previous transmission has completed
  -------------------------------------------------*/
    if ( isAvailable( ownerID ) )
    {
      /*------------------------------------------------
  Lock the hardware to make sure we don't get interrupted
  -------------------------------------------------*/
      if ( !isExternallyReserved() )
      {
        lock( false );
      }

      /*------------------------------------------------
  Activate the chip select line?
  -------------------------------------------------*/
      if ( ( chipSelectMode < Thor::SPI::ChipSelectMode::NUM_CS_MODES ) &&
           ( chipSelectMode != Thor::SPI::ChipSelectMode::MANUAL ) )
      {
        setChipSelect( LogicLevel::LOW );
      }

      /*------------------------------------------------
  Start the transfer
  -------------------------------------------------*/
      channelState[ spi_channel ].transfer_complete = false;
      channelState[ spi_channel ].auto_unlock       = autoRelease;
      channelState[ spi_channel ].auto_disable_cs   = autoDisableCS;

      if ( txBuffer && !rxBuffer )
      {
        status = Thor::Status::PERIPH_TX_IN_PROGRESS;

        /*------------------------------------------------
  The HAL operation doesn't modify the transmit buffer, but for some reason
  the designers did not add const to the function signature. This feels
  nasty, but it must be done.
  -------------------------------------------------*/
        error = HAL_SPI_Transmit_DMA( &spi_handle, const_cast<uint8_t *>( txBuffer ), length );
      }
      else if ( rxBuffer && !txBuffer )
      {
        status = Thor::Status::PERIPH_ERROR;
      }
      else if ( txBuffer && rxBuffer )
      {
        /*------------------------------------------------
  In order for TransmitReceive to work, both subperipherals must be in the
  same mode. This will silently clobber whatever settings were previously
  there.
  -------------------------------------------------*/
        if ( rxMode != txMode )
        {
          setMode( SubPeripheral::RX, txMode );
        }

        status = Thor::Status::PERIPH_TXRX_IN_PROGRESS;

        /*------------------------------------------------
  The HAL operation doesn't modify the transmit buffer, but for some reason
  the designers did not add const to the function signature. This feels
  nasty, but it must be done.
  -------------------------------------------------*/
        error = HAL_SPI_TransmitReceive_DMA( &spi_handle, const_cast<uint8_t *>( txBuffer ), const_cast<uint8_t *>( rxBuffer ),
                                             length );
      }
      else
      {
        resetISRFlags();
        status = Thor::Status::PERIPH_INVALID_PARAM;
      }

      /*------------------------------------------------
  Catch any errors and exit gracefully
  -------------------------------------------------*/
      if ( error != HAL_OK )
      {
        resetISRFlags();
        status = Thor::Status::PERIPH_ERROR;

        if ( autoDisableCS )
        {
          setChipSelect( LogicLevel::HIGH );
        }

        if ( !isExternallyReserved() )
        {
          unlock();
        }
      }
    }
    else
    {
      status = Thor::Status::PERIPH_BUSY;
    }

    return status;
  }

  bool SPIClass::isAvailable( const uint32_t &ownerID )
  {
    return ( !isLocked() || isOwner( ownerID ) );
  }

  bool SPIClass::isLocked()
  {
    if ( spi_channel < NUM_CHANNELS )
    {
      return channelState[ spi_channel ].lock;
    }
    else
    {
      return true;
    }
  }

  bool SPIClass::isOwner( const uint32_t &ownerID )
  {
    if ( ( spi_channel < NUM_CHANNELS ) && ( ownerID != NO_OWNER_ID ) )
    {
      return ( ownerID == channelState[ spi_channel ].owner );
    }
    else
    {
      return false;
    }
  }

  bool SPIClass::isTransferComplete()
  {
    if ( spi_channel < NUM_CHANNELS )
    {
      return channelState[ spi_channel ].transfer_complete;
    }
    else
    {
      // TODO: error out?
      return true;
    }
  }

  bool SPIClass::isExternallyReserved()
  {
    if ( spi_channel < NUM_CHANNELS )
    {
      return channelState[ spi_channel ].externally_reserved;
    }
    else
    {
      // TODO: I might want to throw an error instead of returning true/false?
      // There could be some very unintended consequences for this.
      return true;
    }
  }

  void SPIClass::resetISRFlags()
  {
    if ( spi_channel < NUM_CHANNELS )
    {
      channelState[ spi_channel ].auto_disable_cs   = false;
      channelState[ spi_channel ].auto_unlock       = false;
      channelState[ spi_channel ].transfer_complete = true;
      channelState[ spi_channel ].transfer_error    = false;
    }
  }

  void SPIClass::lock( const bool &external_lock )
  {
    if ( spi_channel < NUM_CHANNELS )
    {
      channelState[ spi_channel ].lock  = true;
      channelState[ spi_channel ].owner = rand();

      ownerID = channelState[ spi_channel ].owner;

      if ( external_lock )
      {
        channelState[ spi_channel ].externally_reserved = true;
      }
    }
  }

  void SPIClass::unlock()
  {
    if ( spi_channel < NUM_CHANNELS )
    {
      channelState[ spi_channel ].lock                = false;
      channelState[ spi_channel ].owner               = NO_OWNER_ID;
      channelState[ spi_channel ].externally_reserved = false;
    }
  }

  void SPIClass::SPI_GPIO_Init()
  {
    if ( spi_handle.Init.Mode == SPI_MODE_MASTER )
    {
      /*------------------------------------------------
  Setup GPIO. Mode must be ALT_PP rather than Input for reads to work.
  -------------------------------------------------*/
      MISO->mode( PinMode::ALT_PP, PinPull::PULLUP );
      MOSI->mode( PinMode::ALT_PP, PinPull::PULLUP );

      if ( spi_handle.Init.CLKPolarity )
      {
        SCK->mode( PinMode::ALT_PP, PinPull::PULLUP );
      }
      else
      {
        SCK->mode( PinMode::ALT_PP, PinPull::PULLDN );
      }

      /*------------------------------------------------
  Decide how to initialize the CS pin. There are a few options.
  -------------------------------------------------*/
      if ( externalCS )
      {
        /*------------------------------------------------
  User supplied pre-initialized GPIOClass_sPtr object. This is
  an externally created object we don't own.
  -------------------------------------------------*/
        externalCS->mode( Thor::GPIO::PinMode::OUTPUT_PP, Thor::GPIO::PinPull::NOPULL );
        externalCS->write( LogicLevel::HIGH );
      }
      else if ( alternateCS )
      {
        /*------------------------------------------------
  User supplied description of the GPIO pin to be initialized
  -------------------------------------------------*/
        CS->mode( Thor::GPIO::PinMode::OUTPUT_PP, Thor::GPIO::PinPull::NOPULL );
        CS->write( LogicLevel::HIGH );
      }
      else
      {
        /*------------------------------------------------
  User supplied nothing. This is the pin described in defaults.cpp
  -------------------------------------------------*/
        CS->mode( PinMode::ALT_PP, PinPull::PULLUP );
      }

      hardwareStatus.gpio_enabled = true;
    }
    else
    {
      MISO->mode( PinMode::ALT_PP, PinPull::PULLUP );
      MOSI->mode( PinMode::ALT_OD, PinPull::PULLUP );
      SCK->mode( PinMode::ALT_OD, PinPull::PULLUP );
      CS->mode( PinMode::ALT_OD, PinPull::PULLUP );

      hardwareStatus.gpio_enabled = true;
    }
  }

  void SPIClass::SPI_GPIO_DeInit()
  {
    SCK.reset( nullptr );
    MOSI.reset( nullptr );
    MISO.reset( nullptr );
    CS.reset( nullptr );

    if ( externalCS )
    {
      externalCS.reset();
    }
  }

  void SPIClass::SPI_Init()
  {
    SPI_EnableClock();

    if ( HAL_SPI_Init( &spi_handle ) != HAL_OK )
    {
      BasicErrorHandler( logError( "Failed SPI Init" ) );
    }

    hardwareStatus.spi_enabled = true;
  }

  void SPIClass::SPI_DeInit()
  {
    if ( HAL_SPI_DeInit( &spi_handle ) != HAL_OK )
    {
      BasicErrorHandler( logError( "Failed SPI DeInit" ) );
    }

    SPI_DisableClock();

    hardwareStatus.spi_enabled = false;
  }

  void SPIClass::SPI_EnableClock()
  {
    *spiClockRegister( spi_handle.Instance ) |= spiClockMask( spi_handle.Instance );
  }

  void SPIClass::SPI_DisableClock()
  {
    *spiClockRegister( spi_handle.Instance ) &= ~spiClockMask( spi_handle.Instance );
  }

  void SPIClass::SPI_DMA_EnableClock()
  {
    __DMA1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();
  }

  void SPIClass::SPI_EnableInterrupts()
  {
    HAL_NVIC_DisableIRQ( ITSettingsHW.IRQn );
    HAL_NVIC_SetPriority( ITSettingsHW.IRQn, ITSettingsHW.preemptPriority, ITSettingsHW.subPriority );
    HAL_NVIC_EnableIRQ( ITSettingsHW.IRQn );

    if ( rxMode == Modes::INTERRUPT )
    {
      __HAL_SPI_ENABLE_IT( &spi_handle, SPI_IT_RXNE );
    }

    hardwareStatus.spi_interrupts_enabled = true;
  }

  void SPIClass::SPI_DisableInterrupts()
  {
    __HAL_SPI_DISABLE_IT( &spi_handle, SPI_IT_RXNE );

    HAL_NVIC_ClearPendingIRQ( ITSettingsHW.IRQn );
    HAL_NVIC_DisableIRQ( ITSettingsHW.IRQn );

    hardwareStatus.spi_interrupts_enabled = false;
  }

  void SPIClass::SPI_DMA_Init( const SubPeripheral &periph )
  {
    if ( periph == SubPeripheral::TX )
    {
      SPI_DMA_EnableClock();

      if ( HAL_DMA_Init( &hdma_spi_tx ) != HAL_OK )
      {
        BasicErrorHandler( logError( "Failed TX Modes::DMA Init" ) );
      }

      __HAL_LINKDMA( &spi_handle, hdmatx, hdma_spi_tx );

      spiDMAManager.attachCallback_TXDMA( spi_channel, boost::bind( &SPIClass::IRQHandler_TXDMA, this ) );

      SPI_DMA_EnableInterrupts( periph );

      hardwareStatus.dma_enabled_tx = true;
    }
    else if ( periph == SubPeripheral::RX )
    {
      SPI_DMA_EnableClock();

      if ( HAL_DMA_Init( &hdma_spi_rx ) != HAL_OK )
      {
        BasicErrorHandler( logError( "Failed RX Modes::DMA Init. Check Init settings." ) );
      }

      __HAL_LINKDMA( &spi_handle, hdmarx, hdma_spi_rx );

      spiDMAManager.attachCallback_RXDMA( spi_channel, boost::bind( &SPIClass::IRQHandler_RXDMA, this ) );

      SPI_DMA_EnableInterrupts( periph );

      hardwareStatus.dma_enabled_rx = true;
    }
  }

  void SPIClass::SPI_DMA_DeInit( const SubPeripheral &periph )
  {
    if ( periph == SubPeripheral::TX )
    {
      if ( !hardwareStatus.dma_enabled_tx )
      {
        return;
      }

      HAL_DMA_Abort( spi_handle.hdmatx );
      HAL_DMA_DeInit( spi_handle.hdmatx );
      SPI_DMA_DisableInterrupts( periph );
      spiDMAManager.removeCallback_TXDMA( spi_channel );

      hardwareStatus.dma_enabled_tx = false;
    }
    else if ( periph == SubPeripheral::RX )
    {
      if ( !hardwareStatus.dma_enabled_rx )
      {
        return;
      }

      HAL_DMA_Abort( spi_handle.hdmarx );
      HAL_DMA_DeInit( spi_handle.hdmarx );
      SPI_DMA_DisableInterrupts( periph );
      spiDMAManager.removeCallback_RXDMA( spi_channel );

      hardwareStatus.dma_enabled_rx = false;
    }
  }

  void SPIClass::SPI_DMA_EnableInterrupts( const SubPeripheral &periph )
  {
    if ( periph == SubPeripheral::TX )
    {
      HAL_NVIC_DisableIRQ( ITSettings_DMA_TX.IRQn );
      HAL_NVIC_ClearPendingIRQ( ITSettings_DMA_TX.IRQn );
      HAL_NVIC_SetPriority( ITSettings_DMA_TX.IRQn, ITSettings_DMA_TX.preemptPriority, ITSettings_DMA_TX.subPriority );
      HAL_NVIC_EnableIRQ( ITSettings_DMA_TX.IRQn );

      hardwareStatus.dma_interrupts_enabled_tx = true;
    }
    else if ( periph == SubPeripheral::RX )
    {
      HAL_NVIC_DisableIRQ( ITSettings_DMA_RX.IRQn );
      HAL_NVIC_ClearPendingIRQ( ITSettings_DMA_RX.IRQn );
      HAL_NVIC_SetPriority( ITSettings_DMA_RX.IRQn, ITSettings_DMA_RX.preemptPriority, ITSettings_DMA_RX.subPriority );
      HAL_NVIC_EnableIRQ( ITSettings_DMA_RX.IRQn );

      hardwareStatus.dma_interrupts_enabled_rx = true;
    }
  }

  void SPIClass::SPI_DMA_DisableInterrupts( const SubPeripheral &periph )
  {
    if ( periph == SubPeripheral::TX )
    {
      HAL_NVIC_ClearPendingIRQ( ITSettings_DMA_TX.IRQn );
      HAL_NVIC_DisableIRQ( ITSettings_DMA_TX.IRQn );

      hardwareStatus.dma_interrupts_enabled_tx = false;
    }
    else if ( periph == SubPeripheral::RX )
    {
      HAL_NVIC_ClearPendingIRQ( ITSettings_DMA_RX.IRQn );
      HAL_NVIC_DisableIRQ( ITSettings_DMA_RX.IRQn );

      hardwareStatus.dma_interrupts_enabled_rx = false;
    }
  }

#ifdef USING_FREERTOS
  void SPIClass::attachThreadTrigger( const Thor::Interrupt::Trigger &trig, const SemaphoreHandle_t *const semphr )
  {
    // spiTaskTrigger.attachEventConsumer(trig, semphr);
  }

  void SPIClass::removeThreadTrigger( const SemaphoreHandle_t *const semphr )
  {
    // spiTaskTrigger.removeEventConsumer(trig);
  }
#endif

  ChimeraSPI::ChimeraSPI( const int &channel )
  {
    this->channel = channel;
    spi           = SPIClass::create( channel );

    conversionError = false;
  }

  Chimera::SPI::Status ChimeraSPI::init( const Chimera::SPI::Setup &setupStruct )
  {
    Chimera::SPI::Status initResult = Chimera::SPI::Status::ERROR;

    /*------------------------------------------------
  Convert the setup configuration and attempt to initialize the hardware
  -------------------------------------------------*/
    Thor::Peripheral::SPI::Config cfg = convertInitSettings( setupStruct );

    if ( conversionError )
    {
      initResult = Chimera::SPI::Status::FAILED_CONVERSION;
    }
    else if ( spi->begin( cfg, false ) == Thor::Status::PERIPH_OK )
    {
      initResult = Chimera::SPI::Status::OK;
    }
    else
    {
      initResult = Chimera::SPI::Status::FAILED_INITIALIZATION;
    }

    return initResult;
  }

  Thor::Peripheral::SPI::Config ChimeraSPI::convertInitSettings( const Chimera::SPI::Setup &setup )
  {
    bool error = false;
    Thor::Peripheral::SPI::Config cfg;

    /*------------------------------------------------
  Convert over all the GPIO configuration info
  -------------------------------------------------*/
    if ( setup.SCK.port != Chimera::GPIO::Port::UNKNOWN_PORT )
    {
      cfg.SCK = ChimeraGPIO::convertPinInit( setup.SCK );
    }

    if ( setup.MOSI.port != Chimera::GPIO::Port::UNKNOWN_PORT )
    {
      cfg.MOSI = ChimeraGPIO::convertPinInit( setup.MOSI );
    }

    if ( setup.MISO.port != Chimera::GPIO::Port::UNKNOWN_PORT )
    {
      cfg.MISO = ChimeraGPIO::convertPinInit( setup.MISO );
    }

    if ( setup.CS.port != Chimera::GPIO::Port::UNKNOWN_PORT )
    {
      cfg.CS = ChimeraGPIO::convertPinInit( setup.CS );
    }

    /*------------------------------------------------
  Convert the hardware parameters
  -------------------------------------------------*/
    cfg.settings                   = Defaults::SPI::dflt_SPI_Init;
    cfg.settings.Mode              = convertMode( setup.mode, &error );
    cfg.settings.DataSize          = convertDataSize( setup.dataSize, &error );
    cfg.settings.FirstBit          = convertBitOrder( setup.bitOrder, &error );
    cfg.settings.CLKPhase          = convertClockPhase( setup.clockMode, &error );
    cfg.settings.CLKPolarity       = convertClockPolarity( setup.clockMode, &error );
    cfg.settings.BaudRatePrescaler = convertBaudRatePrescaler( channel, setup.clockFrequency, &error );

    conversionError = error;

    return cfg;
  }

  uint32_t ChimeraSPI::convertMode( const Chimera::SPI::Mode &mode, bool *const error )
  {
    uint32_t result = 0u;

    switch ( mode )
    {
      case Chimera::SPI::Mode::MASTER:
        result = SPI_MODE_MASTER;
        break;

      case Chimera::SPI::Mode::SLAVE:
        result = SPI_MODE_SLAVE;
        break;

      default:
        result = 0u;
        *error = true;
        break;
    };

    return result;
  }

  uint32_t ChimeraSPI::convertDataSize( const Chimera::SPI::DataSize &size, bool *const error )
  {
    uint32_t result = 0u;

    switch ( size )
    {
      case Chimera::SPI::DataSize::SZ_8BIT:
        result = SPI_DATASIZE_8BIT;
        break;

      case Chimera::SPI::DataSize::SZ_16BIT:
        result = SPI_DATASIZE_16BIT;
        break;

      default:
        result = 0u;
        *error = true;
        break;
    };

    return result;
  }

  uint32_t ChimeraSPI::convertBitOrder( const Chimera::SPI::BitOrder &order, bool *const error )
  {
    uint32_t result = 0u;

    switch ( order )
    {
      case Chimera::SPI::BitOrder::LSB_FIRST:
        result = SPI_FIRSTBIT_LSB;
        break;

      case Chimera::SPI::BitOrder::MSB_FIRST:
        result = SPI_FIRSTBIT_MSB;
        break;

      default:
        result = 0u;
        *error = true;
        break;
    };

    return result;
  }

  uint32_t ChimeraSPI::convertClockPhase( const Chimera::SPI::ClockMode &mode, bool *const error )
  {
    uint32_t result = 0u;

    switch ( mode )
    {
      case Chimera::SPI::ClockMode::MODE0:
      case Chimera::SPI::ClockMode::MODE2:
        result = SPI_PHASE_1EDGE;
        break;

      case Chimera::SPI::ClockMode::MODE1:
      case Chimera::SPI::ClockMode::MODE3:
        result = SPI_PHASE_2EDGE;
        break;

      default:
        result = 0u;
        *error = true;
        break;
    };

    return result;
  }

  uint32_t ChimeraSPI::convertClockPolarity( const Chimera::SPI::ClockMode &mode, bool *const error )
  {
    uint32_t result = 0u;

    switch ( mode )
    {
      case Chimera::SPI::ClockMode::MODE0:
      case Chimera::SPI::ClockMode::MODE1:
        result = SPI_POLARITY_LOW;
        break;

      case Chimera::SPI::ClockMode::MODE2:
      case Chimera::SPI::ClockMode::MODE3:
        result = SPI_POLARITY_HIGH;
        break;

      default:
        result = 0u;
        *error = true;
        break;
    };

    return result;
  }

  uint32_t ChimeraSPI::convertBaudRatePrescaler( const int &channel, const uint32_t &freq, bool *const error )
  {
    return SPIClass::getPrescaler( channel, freq );
  }

  Thor::SPI::ChipSelectMode ChimeraSPI::convertChipSelectMode( const Chimera::SPI::ChipSelectMode &mode, bool *const error )
  {
    Thor::SPI::ChipSelectMode result = Thor::SPI::ChipSelectMode::UNKNOWN_CS_MODE;

    switch ( mode )
    {
      case Chimera::SPI::ChipSelectMode::MANUAL:
        result = Thor::SPI::ChipSelectMode::MANUAL;
        break;

      case Chimera::SPI::ChipSelectMode::AUTO_AFTER_TRANSFER:
        result = Thor::SPI::ChipSelectMode::AUTO_AFTER_TRANSFER;
        break;

      case Chimera::SPI::ChipSelectMode::AUTO_BETWEEN_TRANSFER:
        result = Thor::SPI::ChipSelectMode::AUTO_BETWEEN_TRANSFER;
        break;

      default:
        result = Thor::SPI::ChipSelectMode::UNKNOWN_CS_MODE;
        *error = true;
        break;
    };

    return result;
  }

}    // namespace Thor::SPI

void HAL_SPI_TxCpltCallback( SPI_HandleTypeDef *hspi )
{
  /*------------------------------------------------
Determine which object actually triggered this callback
-------------------------------------------------*/
  const SPIClass_sPtr &spi = getSPIClassRef( hspi->Instance );

  if ( spi && spi->hardwareStatus.spi_enabled && ( spi->spi_channel < NUM_CHANNELS ) )
  {
    const int channel = spi->spi_channel;

    channelState[ channel ].transfer_error    = false;
    channelState[ channel ].transfer_complete = true;

    if ( channelState[ channel ].auto_disable_cs )
    {
      spi->setChipSelect( LogicLevel::HIGH );
    }

    if ( channelState[ channel ].auto_unlock )
    {
      spi->unlock();
    }

    /*------------------------------------------------
Notify event occurred
-------------------------------------------------*/
#if defined( USING_FREERTOS )
    spiTaskTrigger.logEvent( TX_COMPLETE, &spiTaskTrigger );
#endif
  }
}

void HAL_SPI_RxCpltCallback( SPI_HandleTypeDef *hspi )
{
}

void HAL_SPI_TxRxCpltCallback( SPI_HandleTypeDef *hspi )
{
  /*------------------------------------------------
Determine which object actually triggered this callback
-------------------------------------------------*/
  const SPIClass_sPtr &spi = getSPIClassRef( hspi->Instance );

  if ( spi && spi->hardwareStatus.spi_enabled && ( spi->spi_channel < NUM_CHANNELS ) )
  {
    const int channel = spi->spi_channel;

    channelState[ channel ].transfer_error    = false;
    channelState[ channel ].transfer_complete = true;

    if ( channelState[ channel ].auto_disable_cs )
    {
      spi->setChipSelect( LogicLevel::HIGH );
    }

    if ( channelState[ channel ].auto_unlock )
    {
      spi->unlock();
    }

    /*------------------------------------------------
Notify event occurred
-------------------------------------------------*/
#if defined( USING_FREERTOS )
    spiTaskTrigger.logEvent( TXRX_COMPLETE, &spiTaskTrigger );
#endif
  }
}

void HAL_SPI_TxHalfCpltCallback( SPI_HandleTypeDef *hspi )
{
}

void HAL_SPI_RxHalfCpltCallback( SPI_HandleTypeDef *hspi )
{
}

void HAL_SPI_TxRxHalfCpltCallback( SPI_HandleTypeDef *hspi )
{
}

void HAL_SPI_ErrorCallback( SPI_HandleTypeDef *hspi )
{
  /*------------------------------------------------
Determine which object actually triggered this callback
-------------------------------------------------*/
  const SPIClass_sPtr &spi = getSPIClassRef( hspi->Instance );

  const bool enabled      = spi->hardwareStatus.spi_enabled;
  const bool validChannel = ( spi->spi_channel < NUM_CHANNELS );
  const int channel       = spi->spi_channel;

  if ( spi && enabled && validChannel )
  {
    channelState[ channel ].transfer_error    = true;
    channelState[ channel ].transfer_complete = true;

    if ( channelState[ channel ].auto_disable_cs )
    {
      spi->setChipSelect( LogicLevel::HIGH );
    }

    if ( channelState[ channel ].auto_unlock )
    {
      spi->unlock();
    }

    /*------------------------------------------------
Notify event occurred
-------------------------------------------------*/
#if defined( USING_FREERTOS )
    spiTaskTrigger.logEvent( TRANSFER_ERROR, &spiTaskTrigger );
#endif
  }
}

void SPI1_IRQHandler()
{
  if ( spiObjects[ 1 ] )
  {
    spiObjects[ 1 ]->IRQHandler();
  }
}

void SPI2_IRQHandler()
{
  if ( spiObjects[ 2 ] )
  {
    spiObjects[ 2 ]->IRQHandler();
  }
}

void SPI3_IRQHandler()
{
  if ( spiObjects[ 3 ] )
  {
    spiObjects[ 3 ]->IRQHandler();
  }
}

void SPI4_IRQHandler()
{
  if ( spiObjects[ 4 ] )
  {
    spiObjects[ 4 ]->IRQHandler();
  }
}

void SPI5_IRQHandler()
{
  if ( spiObjects[ 5 ] )
  {
    spiObjects[ 5 ]->IRQHandler();
  }
}

void SPI6_IRQHandler()
{
  if ( spiObjects[ 6 ] )
  {
    spiObjects[ 6 ]->IRQHandler();
  }
}

#endif 

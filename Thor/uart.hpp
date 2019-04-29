/********************************************************************************
 * File Name:
 *   uart.hpp
 *
 * Description:
 *   UART interface for Thor
 *
 * 2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_UART_HPP
#define THOR_UART_HPP

/* C++ Includes */
#include <cstdlib>
#include <cstdint>
#include <memory>

/* Boost Includes */
#include <boost/circular_buffer_fwd.hpp>

/* Chimera Includes */
#include <Chimera/buffer.hpp>
#include <Chimera/interface/serial_intf.hpp>

/* Thor Includes */
#include <Thor/gpio.hpp>
#include <Thor/types/interrupt_types.hpp>

#ifdef __cplusplus
extern "C"
{
#endif
  extern void UART4_IRQHandler();
  extern void UART5_IRQHandler();
  extern void UART7_IRQHandler();
  extern void UART8_IRQHandler();
#ifdef __cplusplus
}
#endif

namespace Thor
{
  namespace UART
  {
    class UARTClass;
    using UARTClass_sPtr = std::shared_ptr<UARTClass>;
    using UARTClass_uPtr = std::unique_ptr<UARTClass>;

    class UARTClass : public Chimera::Serial::Interface
    {
    public:
      UARTClass();
      ~UARTClass();

      Chimera::Status_t assignHW( const uint8_t channel, const Chimera::Serial::IOPins &pins ) final override;

      Chimera::Status_t begin( const Chimera::Serial::Modes txMode, const Chimera::Serial::Modes rxMode ) final override;

      Chimera::Status_t end() final override;

      Chimera::Status_t configure( const uint32_t baud, const Chimera::Serial::CharWid width,
                                   const Chimera::Serial::Parity parity, const Chimera::Serial::StopBits stop,
                                   const Chimera::Serial::FlowControl flow ) final override;

      Chimera::Status_t setBaud( const uint32_t baud ) final override;

      Chimera::Status_t setMode( const Chimera::Hardware::SubPeripheral periph,
                                 const Chimera::Serial::Modes mode ) final override;

      Chimera::Status_t write( const uint8_t *const buffer, const size_t length,
                               const uint32_t timeout_mS = 500 ) final override;

      Chimera::Status_t read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) final override;

      Chimera::Status_t flush( const Chimera::Hardware::SubPeripheral periph ) final override;

      void postISRProcessing() final override;

      Chimera::Status_t readAsync( uint8_t *const buffer, const size_t len ) final override;

      Chimera::Status_t attachNotifier( const Chimera::Event::Trigger_t event,
                                             SemaphoreHandle_t *const semphr ) final override;

      Chimera::Status_t detachNotifier( const Chimera::Event::Trigger_t event,
                                             SemaphoreHandle_t *const semphr ) final override;

      Chimera::Status_t enableBuffering( const Chimera::Hardware::SubPeripheral periph,
                                         boost::circular_buffer<uint8_t> *const userBuffer,
                                         uint8_t *const hwBuffer, const uint32_t hwBufferSize ) final override;

      Chimera::Status_t disableBuffering( const Chimera::Hardware::SubPeripheral periph ) final override;

      bool available( size_t *const bytes = nullptr ) final override;

    private:
      /*------------------------------------------------
      Allows the C STM32 HAL and ISR functions access this class
      ------------------------------------------------*/
      friend void( ::HAL_UART_TxCpltCallback )( UART_HandleTypeDef *UsartHandle );
      friend void( ::HAL_UART_RxCpltCallback )( UART_HandleTypeDef *UsartHandle );
      friend void( ::UART4_IRQHandler )( void );
      friend void( ::UART5_IRQHandler )( void );
      friend void( ::UART7_IRQHandler )( void );
      friend void( ::UART8_IRQHandler )( void );

      int uart_channel;              /**< Numerical representation of the UART instance, zero is invalid */
      bool tx_complete       = true; /**< Indicates if a transmission has been completed */
      bool rx_complete       = true; /**< Indicates if a reception has been completed */
      bool AUTO_ASYNC_RX     = true; /**< Enables/Disables asynchronous reception of data */
      bool hardware_assigned = false;

      volatile uint32_t event_bits = 0u;  /* Tracks ISR events so we can respond to them */

      Chimera::Serial::Modes txMode; /**< Logs which mode the TX peripheral is currently in */
      Chimera::Serial::Modes rxMode; /**< Logs which mode the RX peripheral is currently in */

      Chimera::Buffer::DoubleBuffer<UARTClass> txBuffers;
      Chimera::Buffer::DoubleBuffer<UARTClass> rxBuffers;

      uint32_t asyncRXDataSize = 0u;

      uint8_t dmaRXReqSig;
      uint8_t dmaTXReqSig;

      SemaphoreHandle_t *rxCompleteWakeup;
      SemaphoreHandle_t *txCompleteWakeup;
      

      UART_HandleTypeDef uart_handle;
      DMA_HandleTypeDef hdma_uart_tx;
      DMA_HandleTypeDef hdma_uart_rx;
      Thor::GPIO::GPIOClass_sPtr tx_pin;
      Thor::GPIO::GPIOClass_sPtr rx_pin;

      /* Local copy of interrupt settings */
      Thor::Interrupt::Initializer ITSettings_HW;
      Thor::Interrupt::Initializer ITSettings_DMA_TX;
      Thor::Interrupt::Initializer ITSettings_DMA_RX;

      /*------------------------------------------------
      Track states of the hardware to allow/disallow functionality
      ------------------------------------------------*/
      struct UARTClassStatus
      {
        bool gpio_enabled              = false;
        bool enabled                   = false;
        bool tx_buffering_enabled      = false;
        bool rx_buffering_enabled      = false;
        bool dma_enabled_tx            = false;
        bool dma_enabled_rx            = false;
        bool interrupts_enabled        = false;
        bool dma_interrupts_enabled_tx = false;
        bool dma_interrupts_enabled_rx = false;
      } PeripheralState;

      /*------------------------------------------------
      Mode Change Function Pointers 
      ------------------------------------------------*/
      std::array<void ( UARTClass::* )( const Chimera::Hardware::SubPeripheral ), 3> modeChangeFuncPtrs;
      void setBlockingMode( const Chimera::Hardware::SubPeripheral periph );
      void setInterruptMode( const Chimera::Hardware::SubPeripheral periph );
      void setDMAMode( const Chimera::Hardware::SubPeripheral periph );

      /*------------------------------------------------
      Read Type Function Pointers  
      ------------------------------------------------*/
      std::array<Chimera::Status_t ( UARTClass::* )( uint8_t *const, const size_t, const uint32_t ), 3> readFuncPtrs;
      Chimera::Status_t readBlocking( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS );
      Chimera::Status_t readInterrupt( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS );
      Chimera::Status_t readDMA( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS );

      /*------------------------------------------------
      Write Type Function Pointers 
      ------------------------------------------------*/
      std::array<Chimera::Status_t ( UARTClass::* )( const uint8_t *const, const size_t, const uint32_t ), 3> writeFuncPtrs;
      Chimera::Status_t writeBlocking( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS );
      Chimera::Status_t writeInterrupt( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS );
      Chimera::Status_t writeDMA( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS );

      /*------------------------------------------------
      Low Level Hardware Configuration Functions
      ------------------------------------------------*/
      bool setWordLength( UART_InitTypeDef &initStruct, const Chimera::Serial::CharWid width );
      bool setParity( UART_InitTypeDef &initStruct, const Chimera::Serial::Parity parity );
      bool setStopBits( UART_InitTypeDef &initStruct, const Chimera::Serial::StopBits stopBits );
      bool setFlowControl( UART_InitTypeDef &initStruct, const Chimera::Serial::FlowControl flow );
      
      void IRQHandler();
      void IRQHandler_TXDMA();
      void IRQHandler_RXDMA();

      void UART_GPIO_Init();
      void UART_GPIO_DeInit();

      Chimera::Status_t UART_Init();
      void UART_DeInit();
      void UART_EnableClock();
      void UART_DisableClock();
      void UART_DMA_EnableClock();
      void UART_EnableInterrupts();
      void UART_DisableInterrupts();

      void UART_DMA_Init( const Chimera::Hardware::SubPeripheral periph );
      void UART_DMA_DeInit( const Chimera::Hardware::SubPeripheral periph );
      void UART_DMA_EnableIT( const Chimera::Hardware::SubPeripheral periph );
      void UART_DMA_DisableIT( const Chimera::Hardware::SubPeripheral periph );

      void UART_OverrunHandler();
    };
  }    // namespace UART
}    // namespace Thor
#endif /* !UART_H_ */

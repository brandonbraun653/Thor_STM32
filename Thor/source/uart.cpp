/* Boost Includes */
#include <boost/bind.hpp>

/* Project Includes */
#include <Thor/include/uart.h>
#include <Thor/include/exceptions.h>
#include <Thor/include/interrupt.h>

using namespace Thor::Definitions::Serial;
using namespace Thor::Defaults::Serial;
using namespace Thor::Peripheral::UART;




namespace Thor
{
	namespace Definitions
	{
		namespace Serial
		{
			boost::container::flat_map<USART_TypeDef*, Thor::Peripheral::UART::UARTClass_sPtr> uart_instance_to_class =
			{
				#if defined(STM32F446xx)
					#if defined(ENABLE_UART4)
					{ UART4, uart4 },
					#endif
					#if defined(ENABLE_UART5)
					{ UART5, uart5 }
					#endif
				#endif

				#if defined(STM32F767xx)
					#if defined(ENABLE_UART4)
					{ UART4, uart4 },
					#endif
					#if defined(ENABLE_UART5)
					{ UART5, uart5 },
					#endif
					#if defined(ENABLE_UART7)
					{ UART7, uart7 },
					#endif
					#if defined(ENABLE_UART8)
					{ UART8, uart8 },
					#endif
				#endif
			};

			boost::container::flat_map<USART_TypeDef*, uint32_t> uart_clock_mask =
			{
				#if defined(STM32F446xx)
					#if defined(ENABLE_UART4)
					{ UART4, RCC_APB1ENR_UART4EN },
					#endif
					#if defined(ENABLE_UART5)
					{ UART5, RCC_APB1ENR_UART5EN }
					#endif
				#endif 

				#if defined(STM32F767xx)
					#if defined(ENABLE_UART4)
					{ UART4, RCC_APB1ENR_UART4EN },
					#endif
					#if defined(ENABLE_UART5)
					{ UART5, RCC_APB1ENR_UART5EN },
					#endif
					#if defined(ENABLE_UART7)
					{ UART7, RCC_APB1ENR_UART7EN },
					#endif
					#if defined(ENABLE_UART8)
					{ UART8, RCC_APB1ENR_UART8EN },
					#endif
				#endif 
			};
		}
	}


	namespace Peripheral
	{
		namespace UART
		{
			UARTClass::UARTClass(int channel)
			{
				uart_channel = channel;

				/* Assign external flag pointers */
				isInitialized = &UART_PeriphState.uart_enabled;

				txMode = TX_MODE_NONE;
				rxMode = RX_MODE_NONE;
				RX_ASYNC = true;

				/* Default handle settings for this instance */
				uart_handle.Init = Defaults::Serial::dflt_UART_Init;
				uart_handle.Instance = srl_cfg[uart_channel].instance;

				#if defined(STM32F7)
				uart_handle.AdvancedInit = Defaults::Serial::dflt_UART_AdvInit;
				#endif

				/* Copy over the interrupt settings information */
				ITSettings_HW = srl_cfg[uart_channel].IT_HW;
				ITSettings_DMA_TX = srl_cfg[uart_channel].dmaIT_TX;
				ITSettings_DMA_RX = srl_cfg[uart_channel].dmaIT_RX;

				/* Create the output gpio pin objects */
				tx_pin = boost::make_shared<Thor::Peripheral::GPIO::GPIOClass>(
					srl_cfg[uart_channel].txPin.GPIOx,
					srl_cfg[uart_channel].txPin.PinNum,
					srl_cfg[uart_channel].txPin.Speed,
					srl_cfg[uart_channel].txPin.Alternate);

				rx_pin = boost::make_shared<Thor::Peripheral::GPIO::GPIOClass>(
					srl_cfg[uart_channel].rxPin.GPIOx,
					srl_cfg[uart_channel].rxPin.PinNum,
					srl_cfg[uart_channel].rxPin.Speed,
					srl_cfg[uart_channel].rxPin.Alternate);

				/* Initialize the buffer memory */
				TXPacketBuffer.set_capacity(Thor::Definitions::Serial::UART_BUFFER_SIZE);
				RXPacketBuffer.set_capacity(Thor::Definitions::Serial::UART_BUFFER_SIZE);


				#if defined(USING_FREERTOS)
				uart_semphr = xSemaphoreCreateCounting(Thor::Definitions::Serial::UART_BUFFER_SIZE, 
													   Thor::Definitions::Serial::UART_BUFFER_SIZE);
				#endif 
			}

			UARTClass::~UARTClass()
			{
				end();
			}


			UART_Status UARTClass::begin()
			{
				return begin(115200, TX_MODE_BLOCKING, RX_MODE_BLOCKING);
			}

			UART_Status UARTClass::begin(uint32_t baud)
			{
				return begin(baud, TX_MODE_BLOCKING, RX_MODE_BLOCKING);
			}

			UART_Status UARTClass::begin(uint32_t baud, uint32_t tx_mode, uint32_t rx_mode)
			{
				UART_GPIO_Init();

				uart_handle.Init.BaudRate = baud;
				UART_Init();

				/* Configure the transmission mode */
				switch (tx_mode)
				{
				case TX_MODE_BLOCKING:
					setBlockMode(UARTPeriph::TX);
					break;

				case TX_MODE_INTERRUPT:
					setITMode(UARTPeriph::TX);
					break;

				case TX_MODE_DMA:
					setDMAMode(UARTPeriph::TX);
					break;

				default:
					txMode = TX_MODE_BLOCKING;
					setBlockMode(UARTPeriph::TX);
					break;
				}

				/* Configure the reception mode */
				switch (rx_mode)
				{
				case RX_MODE_BLOCKING:
					setBlockMode(UARTPeriph::RX);
					break;

				case RX_MODE_INTERRUPT:
					setITMode(UARTPeriph::RX);
					break;

				case RX_MODE_DMA:
					setDMAMode(UARTPeriph::RX);
					break;

				default:
					rxMode = RX_MODE_BLOCKING;
					setBlockMode(UARTPeriph::RX);
					break;
				}

				return UART_OK;
			}

			UART_Status UARTClass::write(std::string string)
			{
				return write((uint8_t*)string.data(), string.size());
			}

			UART_Status UARTClass::write(const char* string)
			{
				return write((uint8_t*)string, strlen(string));
			}

			UART_Status UARTClass::write(char* string, size_t length)
			{
				return write((uint8_t*)string, length);
			}

			UART_Status UARTClass::write(uint8_t* val, size_t length)
			{
				/*------------------------------------
				* NOTE:
				* This method does no real time checking of the TX length compared
				* to the length of data that is passed in. This is because the data
				* (val) decays to a pointer upon entry to the function and all information
				* regarding length is lost. As such, it is up to the programmer to ensure
				* that the length of the input data is not exceeded during TX or undefined
				* behavior will occur.
				*------------------------------------*/
				if (!UART_PeriphState.gpio_enabled || !UART_PeriphState.uart_enabled)
					return UART_NOT_INITIALIZED;

				#if defined(USING_FREERTOS)
				/* Requires the ISR version b/c it can be called from the 
				 * Interrupt or DMA handlers to send out queued packets. */
				if (xSemaphoreTakeFromISR(uart_semphr, NULL) != pdPASS)
					return UART_LOCKED;
				#endif


				switch (txMode)
				{
				case TX_MODE_BLOCKING:
					if (tx_complete)
					{
						tx_complete = false;
						HAL_UART_Transmit(&uart_handle, val, length, HAL_MAX_DELAY);
						tx_complete = true;
					}
					return UART_OK;
					break;

				case TX_MODE_INTERRUPT:
					if (UART_PeriphState.uart_interrupts_enabled)
					{
						if (tx_complete)
						{
							/* Starting a brand new IT transmission */
							tx_complete = false;
							HAL_UART_Transmit_IT(&uart_handle, val, length);
							return UART_TX_IN_PROGRESS;
						}
						else
						{
							/* A previous IT transmission is still going. Queue the data packet. */
							TX_tempPacket.data = 0;
							TX_tempPacket.data_ptr = val;
							TX_tempPacket.length = length;

							TXPacketBuffer.push_back(TX_tempPacket);
							return UART_NOT_READY;
						}
					}
					else
						return UART_ERROR;
					break;

				case TX_MODE_DMA:
					if (UART_PeriphState.dma_enabled_tx && UART_PeriphState.uart_interrupts_enabled)
					{
						if (tx_complete)
						{
							/* Starting a brand new DMA transmission */
							tx_complete = false;
							HAL_UART_Transmit_DMA(&uart_handle, val, length);
							return UART_TX_IN_PROGRESS;
						}
						else
						{
							/* A previous DMA transmission is still going. Queue the data packet. */
							TX_tempPacket.data = 0;
							TX_tempPacket.data_ptr = val;
							TX_tempPacket.length = length;

							TXPacketBuffer.push_back(TX_tempPacket);
							return UART_NOT_READY;
						}
					}
					else
						return UART_ERROR;
					break;

				default: return UART_ERROR;
				}
			}

			

			UART_Status UARTClass::readPacket(uint8_t* buff, size_t buff_length)
			{
				UARTPacket packet = RXPacketBuffer.front();

				size_t packetLength = packet.length;
				UART_Status error = UART_OK;

				/* Check if the received packet is too large for the buffer */
				if (packetLength > buff_length)
				{
					packetLength = buff_length;
					error = UART_PACKET_TOO_LARGE_FOR_BUFFER;
				}

				memcpy(buff, packet.data_ptr, packetLength);

				RXPacketBuffer.pop_front();

				totalWaitingPackets--;
				return error;
			}

			size_t UARTClass::nextPacketSize()
			{
				if (RXPacketBuffer.empty())
					return (size_t)0;
				else
					return RXPacketBuffer.front().length;
			}

			int UARTClass::availablePackets()
			{
				return totalWaitingPackets;
			}

			void UARTClass::flush()
			{

			}

			void UARTClass::end()
			{
				UART_DeInit();
				UART_GPIO_DeInit();
				UART_DisableInterrupts();
				UART_DMA_DeInit(UARTPeriph::TX);
				UART_DMA_DeInit(UARTPeriph::RX);

				txMode = TX_MODE_NONE;
				rxMode = RX_MODE_NONE;
			}

			void UARTClass::attachSettings(UART_InitTypeDef config)
			{
				uart_handle.Init = config;

				if (UART_PeriphState.uart_enabled)
					UART_DeInit();

				UART_Init();
			}

			void UARTClass::setBlockMode(const UARTPeriph& periph)
			{
				if (periph == UARTPeriph::TX)
				{
					txMode = TX_MODE_BLOCKING;

					/* Make sure RX side isn't using interrupts before disabling */
					if (rxMode = RX_MODE_BLOCKING)
						UART_DisableInterrupts();

					UART_DMA_DeInit(periph);
				}
				else
				{
					rxMode = RX_MODE_BLOCKING;

					/* Make sure TX side isn't using interrupts before disabling */
					if (txMode = TX_MODE_BLOCKING)
						UART_DisableInterrupts();

					UART_DMA_DeInit(periph);
				}
			}

			void UARTClass::setITMode(const UARTPeriph& periph)
			{
				UART_EnableInterrupts();
				UART_DMA_DeInit(periph);

				if (periph == UARTPeriph::TX)
					txMode = TX_MODE_INTERRUPT;
				else
					rxMode = RX_MODE_INTERRUPT;
			}

			void UARTClass::setDMAMode(const UARTPeriph& periph)
			{
				UART_EnableInterrupts();
				UART_DMA_Init(periph);

				if (periph == UARTPeriph::TX)
					txMode = TX_MODE_DMA;
				else
				{
					rxMode = RX_MODE_DMA;

					/* Instruct the DMA hardware to start listening for packets.
					* Set the idle line bit for triggering the end of packet interrupt. */
					HAL_UART_Receive_DMA(&uart_handle, packetQueue[currentQueuePacket], Thor::Definitions::Serial::UART_BUFFER_SIZE);
					__HAL_UART_ENABLE_IT(&uart_handle, UART_IT_IDLE);

					#if defined(STM32F7)
					__HAL_UART_CLEAR_IT(&uart_handle, UART_CLEAR_IDLEF);
					#endif

					#if defined(STM32F4)
					__HAL_UART_CLEAR_FLAG(&uart_handle, UART_FLAG_IDLE);
					#endif
				}
			}

			void UARTClass::UART_Init()
			{
				UART_EnableClock();

				if (HAL_UART_Init(&uart_handle) != HAL_OK)
					BasicErrorHandler(logError("Failed UART Init. Check settings."));

				setBlockMode(UARTPeriph::TX);
				setBlockMode(UARTPeriph::RX);

				UART_PeriphState.uart_enabled = true;
			}

			void UARTClass::UART_DeInit()
			{
				HAL_UART_DeInit(&uart_handle);
				UART_PeriphState.uart_enabled = false;
			}

			void UARTClass::UART_EnableClock()
			{
				using namespace Thor::Definitions::Serial;

				#if defined(TARGET_STM32F7) || defined(TARGET_STM32F4)
				RCC->APB1ENR |= (uart_clock_mask[uart_handle.Instance]);
				#endif 
			}

			void UARTClass::UART_DisableClock()
			{
				using namespace Thor::Definitions::Serial;

				#if defined(TARGET_STM32F7) || defined(TARGET_STM32F4)
				RCC->APB1ENR &= ~(uart_clock_mask[uart_handle.Instance]);
				#endif 
			}


			void UARTClass::UART_DMA_EnableClock()
			{
				/* Global DMA Clock options. Only turn on capability is
				provided due to other peripherals possibly using DMA. */
				if (__DMA1_IS_CLK_DISABLED())
					__DMA1_CLK_ENABLE();


				if (__DMA2_IS_CLK_DISABLED())
					__DMA2_CLK_ENABLE();
			}

			void UARTClass::UART_EnableInterrupts()
			{
				HAL_NVIC_DisableIRQ(ITSettings_HW.IRQn);
				HAL_NVIC_SetPriorityGrouping(ITSettings_HW.groupPriority);
				HAL_NVIC_SetPriority(ITSettings_HW.IRQn, ITSettings_HW.preemptPriority, ITSettings_HW.subPriority);
				HAL_NVIC_EnableIRQ(ITSettings_HW.IRQn); //Triggers an interrupt immediately. Why?
				HAL_NVIC_ClearPendingIRQ(ITSettings_HW.IRQn);

				/* Specific interrupts to enable */
				if (rxMode == RX_MODE_INTERRUPT)
					__HAL_UART_ENABLE_IT(&uart_handle, UART_IT_RXNE);	//RX Data Register not Empty

				UART_PeriphState.uart_interrupts_enabled = true;
			}

			void UARTClass::UART_DisableInterrupts()
			{
				__HAL_UART_DISABLE_IT(&uart_handle, UART_IT_IDLE);
				__HAL_UART_DISABLE_IT(&uart_handle, UART_IT_RXNE);

				HAL_NVIC_ClearPendingIRQ(ITSettings_HW.IRQn);
				HAL_NVIC_DisableIRQ(ITSettings_HW.IRQn);

				UART_PeriphState.uart_interrupts_enabled = false;
			}

			void UARTClass::UART_GPIO_Init()
			{
				/* These should be configured as ALT_PP with PULLUPs */
				if (tx_pin != NULL)
					tx_pin->mode(srl_cfg[uart_channel].txPin.Mode, srl_cfg[uart_channel].rxPin.Pull);

				if (rx_pin != NULL)
					rx_pin->mode(srl_cfg[uart_channel].rxPin.Mode, srl_cfg[uart_channel].rxPin.Pull);

				UART_PeriphState.gpio_enabled = true;
			}

			void UARTClass::UART_GPIO_DeInit()
			{
				//TODO: Implement GPIO DeInit
			}

			void UARTClass::UART_DMA_Init(const UARTPeriph& periph)
			{
				if (periph == UARTPeriph::TX)
				{
					UART_DMA_EnableClock();
					hdma_uart_tx.Instance = srl_cfg[uart_channel].dmaTX.Instance;

					/* Grab the default init settings and modify for the specific hardware */
					hdma_uart_tx.Init = Defaults::Serial::dflt_DMA_Init_TX;
					hdma_uart_tx.Init.Channel = Defaults::Serial::srl_cfg[uart_channel].dmaTX.channel;
					hdma_uart_tx.Init.Direction = Defaults::Serial::srl_cfg[uart_channel].dmaTX.direction;

					/* Hard error if initialization fails. */
					if (HAL_DMA_Init(&hdma_uart_tx) != HAL_OK)
						BasicErrorHandler(logError("Failed UART DMA TX Init. Check handle settings."));

					__HAL_LINKDMA(&uart_handle, hdmatx, hdma_uart_tx);

					uart_dma_manager.attachCallbackFunction_TXDMA(uart_channel, boost::bind(&UARTClass::IRQHandler_TXDMA, this));

					UART_DMA_EnableIT(periph);

					UART_PeriphState.dma_enabled_tx = true;
				}
				else
				{
					UART_DMA_EnableClock();
					hdma_uart_rx.Instance = srl_cfg[uart_channel].dmaRX.Instance;

					/* Grab the default init settings and modify for the specific hardware */
					hdma_uart_rx.Init = Defaults::Serial::dflt_DMA_Init_RX;
					hdma_uart_rx.Init.Channel = Defaults::Serial::srl_cfg[uart_channel].dmaRX.channel;
					hdma_uart_rx.Init.Direction = Defaults::Serial::srl_cfg[uart_channel].dmaRX.direction;

					/* Hard error if initialization fails. */
					if (HAL_DMA_Init(&hdma_uart_rx) != HAL_OK)
						BasicErrorHandler(logError("Failed UART DMA TX Init. Check handle settings."));

					__HAL_LINKDMA(&uart_handle, hdmarx, hdma_uart_rx);

					uart_dma_manager.attachCallbackFunction_RXDMA(uart_channel, boost::bind(&UARTClass::IRQHandler_RXDMA, this));

					UART_DMA_EnableIT(periph);

					UART_PeriphState.dma_enabled_rx = true;
				}
			}

			void UARTClass::UART_DMA_DeInit(const UARTPeriph& periph)
			{
				if (periph == UARTPeriph::TX)
				{
					if (!UART_PeriphState.dma_enabled_tx)
						return;

					HAL_DMA_Abort(uart_handle.hdmatx);
					HAL_DMA_DeInit(uart_handle.hdmatx);
					UART_DMA_DisableIT(periph);
					uart_dma_manager.removeCallbackFunction_TXDMA(uart_channel);

					UART_PeriphState.dma_enabled_tx = false;
				}
				else
				{
					if (!UART_PeriphState.dma_enabled_rx)
						return;

					HAL_DMA_Abort(uart_handle.hdmarx);
					HAL_DMA_DeInit(uart_handle.hdmarx);
					UART_DMA_DisableIT(periph);
					uart_dma_manager.removeCallbackFunction_RXDMA(uart_channel);

					UART_PeriphState.dma_enabled_rx = false;
				}
			}

			void UARTClass::UART_DMA_EnableIT(const UARTPeriph& periph)
			{
				if (periph == UARTPeriph::TX)
				{
					HAL_NVIC_DisableIRQ(ITSettings_DMA_TX.IRQn);
					HAL_NVIC_ClearPendingIRQ(ITSettings_DMA_TX.IRQn);
					HAL_NVIC_SetPriorityGrouping(ITSettings_DMA_TX.groupPriority);
					HAL_NVIC_SetPriority(ITSettings_DMA_TX.IRQn, ITSettings_DMA_TX.preemptPriority, ITSettings_DMA_TX.subPriority);
					HAL_NVIC_EnableIRQ(ITSettings_DMA_TX.IRQn);

					UART_PeriphState.dma_interrupts_enabled_tx = true;
				}
				else
				{
					HAL_NVIC_DisableIRQ(ITSettings_DMA_RX.IRQn);
					HAL_NVIC_ClearPendingIRQ(ITSettings_DMA_RX.IRQn);
					HAL_NVIC_SetPriorityGrouping(ITSettings_DMA_RX.groupPriority);
					HAL_NVIC_SetPriority(ITSettings_DMA_RX.IRQn, ITSettings_DMA_RX.preemptPriority, ITSettings_DMA_RX.subPriority);
					HAL_NVIC_EnableIRQ(ITSettings_DMA_RX.IRQn);

					UART_PeriphState.dma_interrupts_enabled_rx = true;
				}
			}

			void UARTClass::UART_DMA_DisableIT(const UARTPeriph& periph)
			{
				if (periph == UARTPeriph::TX)
				{
					HAL_NVIC_ClearPendingIRQ(ITSettings_DMA_TX.IRQn);
					HAL_NVIC_DisableIRQ(ITSettings_DMA_TX.IRQn);

					UART_PeriphState.dma_interrupts_enabled_tx = false;
				}
				else
				{
					HAL_NVIC_ClearPendingIRQ(ITSettings_DMA_RX.IRQn);
					HAL_NVIC_DisableIRQ(ITSettings_DMA_RX.IRQn);

					UART_PeriphState.dma_interrupts_enabled_rx = false;
				}
			}


			void UARTClass::IRQHandler()
			{
				#if defined(STM32F7)
				bool RX_DATA_READY = __HAL_UART_GET_FLAG(&uart_handle, UART_FLAG_RXNE);
				bool RX_LINE_IDLE = __HAL_UART_GET_FLAG(&uart_handle, UART_FLAG_IDLE);

				/*------------------------------------
				* Handle Asynchronous RX (Interrupt and DMA Mode)
				*------------------------------------*/
				/* RX In Progress */
				if (RX_ASYNC && RX_DATA_READY && uart_handle.gState != HAL_UART_STATE_BUSY_TX)
				{
					uint32_t isrflags = READ_REG(uart_handle.Instance->ISR);
					uint32_t errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));

					if (errorflags == RESET)
					{
						/* Detected start of a new frame of unknown size. Enable the
						* IDLE interrupt bit to detect end of frame. */
						if (rxAsyncPacketSize == 0)
						{
							memset(packetQueue[currentQueuePacket], 0, Thor::Definitions::Serial::UART_BUFFER_SIZE);
							__HAL_UART_ENABLE_IT(&uart_handle, UART_IT_IDLE);
						}

						/* Buffer the new data */
						if (rxMode == RX_MODE_INTERRUPT && (rxAsyncPacketSize < Thor::Definitions::Serial::UART_BUFFER_SIZE))
						{
							packetQueue[currentQueuePacket][rxAsyncPacketSize] = (uint8_t)(uart_handle.Instance->RDR & (uint8_t)0xFF);
							rxAsyncPacketSize += 1u;
						}
						else
						{
							/* Forced to read data without being able to store it.
							* Really need to put some kind of error thing here...*/
							uart_handle.Instance->RDR;
						}
					}
					else
					{
						//Do something more useful later
						__HAL_UART_CLEAR_IT(&uart_handle, UART_CLEAR_PEF);
						__HAL_UART_CLEAR_IT(&uart_handle, UART_CLEAR_FEF);
						__HAL_UART_CLEAR_IT(&uart_handle, UART_CLEAR_NEF);
						__HAL_UART_CLEAR_IT(&uart_handle, UART_CLEAR_OREF);
					}
				}

				/* RX Complete */
				if (RX_ASYNC && RX_LINE_IDLE)
				{
					/* Disable Idle Line Interrupt */
					__HAL_UART_DISABLE_IT(&uart_handle, UART_IT_IDLE);
					__HAL_UART_CLEAR_IT(&uart_handle, UART_CLEAR_IDLEF);

					/* Copy packets received to the internal buffer */
					if (rxMode == RX_MODE_INTERRUPT)
					{
						/* Store the address of the new data packet */
						RX_tempPacket.data		= 0;
						RX_tempPacket.data_ptr	= packetQueue[currentQueuePacket];
						RX_tempPacket.length	= rxAsyncPacketSize;
						RXPacketBuffer.push_back(RX_tempPacket);

						rxAsyncPacketSize = 0;	/* Reset the internal packet counter so we know when a new frame starts */
						currentQueuePacket++;	/* Go to the next buffer location */

						if (currentQueuePacket == Thor::Definitions::Serial::UART_PACKET_QUEUE_SIZE)
							currentQueuePacket = 0;
					}
					else if (rxMode == RX_MODE_DMA)
					{
						/* Force a hard reset of the DMA to trigger the DMA RX Complete handler */
						if (uart_handle.hdmarx->Instance->NDTR != 0)
							__HAL_DMA_DISABLE(uart_handle.hdmarx);
					}

					rx_complete = true;
					totalWaitingPackets++;
				}

				/*------------------------------------
				* Handle Synchronous TX-RX (Interrupt)
				*------------------------------------*/
				if (!RX_ASYNC || uart_handle.gState == HAL_UART_STATE_BUSY_TX)
				{
					HAL_UART_IRQHandler(&uart_handle);

					//Trace to make sure this gets hit after buffered TXs
					#if defined(USING_FREERTOS)
					if (tx_complete)
					{
						xSemaphoreGiveFromISR(uart_semphr, NULL);
					}
					#endif 
				}
					
				#endif 


				#if defined(STM32F4)
				/* Reading these two in the order of SR then DR ends up
				* clearing all flags, so it's best to store the returned
				* contents for further processing. */
				volatile uint32_t isrflags = READ_REG(uart_handle.Instance->SR);
				volatile uint32_t data_reg = READ_REG(uart_handle.Instance->DR);

				/*------------------------------------
				* Handle Async RX of unknown length(Interrupt and DMA Mode)
				*------------------------------------*/
				if (RX_ASYNC)
				{
					volatile uint32_t cr1 = READ_REG(uart_handle.Instance->CR1);

					bool RX_DATA_READY = ((isrflags & UART_FLAG_RXNE) == UART_FLAG_RXNE);
					bool RX_DATA_READY_IE = ((cr1 & USART_CR1_RXNEIE) == USART_CR1_RXNEIE);
					bool RX_LINE_IDLE = ((isrflags & UART_FLAG_IDLE) == UART_FLAG_IDLE);
					bool RX_LINE_IDLE_IE = ((cr1 & USART_CR1_IDLEIE) == USART_CR1_IDLEIE);

					if (RX_DATA_READY && RX_DATA_READY_IE)
					{
						uint32_t errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));

						/* No Errors Found */
						if (errorflags == 0u)
						{
							/* Detected start of a new frame of unknown size. Enable the
							* IDLE interrupt bit to detect end of frame. */
							if (rxAsyncPacketSize == 0)
							{
								memset(packetQueue[currentQueuePacket], 0, Thor::Definitions::Serial::UART_BUFFER_SIZE);
								__HAL_UART_ENABLE_IT(&uart_handle, UART_IT_IDLE);
							}

							/* Buffer the new data and increase packet size count */
							if (rxMode == RX_MODE_INTERRUPT && (rxAsyncPacketSize < Thor::Definitions::Serial::UART_BUFFER_SIZE))
							{
								packetQueue[currentQueuePacket][rxAsyncPacketSize] = (uint8_t)(data_reg & (uint8_t)0xFF);
								rxAsyncPacketSize += 1u;
							}
							else
							{
								/* Somehow inform the user that data is being tossed out */
							}
						}

						/* Error Handling */
						else
						{
							// TODO
						}
					}

					/* RX Complete */
					if (RX_LINE_IDLE && RX_LINE_IDLE_IE)
					{
						/* Disable Idle Line Interrupt */
						__HAL_UART_DISABLE_IT(&uart_handle, UART_IT_IDLE);

						/* Copy packets received to the internal buffer */
						if (rxMode == RX_MODE_INTERRUPT)
						{
							/* Store the address of the the new data packet */
							RX_tempPacket.data = 0;
							RX_tempPacket.data_ptr = packetQueue[currentQueuePacket];
							RX_tempPacket.length = rxAsyncPacketSize;
							RXPacketBuffer.push_back(RX_tempPacket);

							rxAsyncPacketSize = 0;	/* Reset the internal packet counter so we know when a new frame starts */
							currentQueuePacket++;	/* Go to the next buffer location */

							if (currentQueuePacket == Thor::Definitions::Serial::UART_PACKET_QUEUE_SIZE)
								currentQueuePacket = 0;


							/*------------------------------------
							* Signal Waiting Threads
							*------------------------------------*/
							#if defined(USING_FREERTOS)

							/* Inform the semaphore task manager that a particular event has occured
							* on the given source and periph instance. If a semaphore is tied to this,
							* 1 semaphore will be given in the EXTI0 interrupt. */
							EXTI0_TaskMGR->logEventGenerator(SRC_UART, uart_channel);
							#endif 
						}

						else if (rxMode == RX_MODE_DMA)
						{
							/* Force a hard reset of the DMA to trigger the DMA RX Complete handler */
							if (uart_handle.hdmarx->Instance->NDTR != 0)
								__HAL_DMA_DISABLE(uart_handle.hdmarx);
						}

						rx_complete = true;
						totalWaitingPackets++;
					}
				}

				/*------------------------------------
				* Handle Explicit User Requests:
				* Only run the normal IRQHandler if an explicit RX packet request
				* was generated or if TX-ing some data.
				*------------------------------------*/

				//TODO: Better fix this later to support usart handler in uart mode
				if (!RX_ASYNC || uart_handle.gState == HAL_UART_STATE_BUSY_TX)
					HAL_UART_IRQHandler(&uart_handle);
				#endif
			}

			void UARTClass::IRQHandler_TXDMA()
			{
				HAL_DMA_IRQHandler(uart_handle.hdmatx);
			}

			void UARTClass::IRQHandler_RXDMA()
			{
				HAL_DMA_IRQHandler(uart_handle.hdmarx);
			}
		}
	}
}


/************************************************************************/
/*						   Callback Functions                           */
/************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	auto uart = Thor::Definitions::Serial::uart_instance_to_class[UartHandle->Instance];

	if (uart && uart->isInitialized)
	{
		uart->tx_complete = true;

		/* Check if we have more data to send out */
		if (!uart->TXPacketBuffer.empty())
		{
			auto packet = uart->TXPacketBuffer.front();

			uart->write(packet.data_ptr, packet.length);

			uart->TXPacketBuffer.pop_front();
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	using PacketType = Thor::Peripheral::UART::UARTClass::UARTPacket;
	auto uart = Thor::Definitions::Serial::uart_instance_to_class[UartHandle->Instance];

	if (uart->rxMode == RX_MODE_DMA)
	{
		uart->rx_complete = true;

		/* Calculate how many bytes were received by looking at remaining RX buffer space
		 * num_received = bufferMaxSize - bufferSizeRemaining */
		size_t num_received = (size_t)(UartHandle->RxXferSize - UartHandle->hdmarx->Instance->NDTR);

		/* Construct the received packet and push into the receive queue */
		PacketType tempPacket;
		tempPacket.data		= 0;
		tempPacket.data_ptr = uart->packetQueue[uart->currentQueuePacket];
		tempPacket.length	= num_received;
		uart->RXPacketBuffer.push_back(tempPacket);

		/* Start the listening process again for a new packet */
		__HAL_UART_ENABLE_IT(UartHandle, UART_IT_IDLE);
		HAL_UART_Receive_DMA(UartHandle, uart->packetQueue[uart->currentQueuePacket], Thor::Definitions::Serial::UART_BUFFER_SIZE);

		/* Signal Waiting Threads */
		#if defined(USING_FREERTOS)
		EXTI0_TaskMGR->logEventGenerator(Thor::Interrupt::SRC_UART, uart->uart_channel);
		#endif
	}

	/* Only runs if the user explicitly requests RX in blocking or interrupt mode */
	else if (!uart->rx_complete)
	{
		uart->rx_complete = true;

		//Revert back to asynchronous listening for RX
		if(uart->rxMode == RX_MODE_INTERRUPT)
			uart->RX_ASYNC = true; 
	}
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *UartHandle)
{
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *UartHandle)
{
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
}



#if defined(ENABLE_UART4)
// #ifdef USING_CHIMERA
// UARTClass_sPtr uart4;
// #else
// UARTClass_sPtr uart4 = boost::make_shared<UARTClass>(4);
// #endif

UARTClass_sPtr uart4 = boost::make_shared<UARTClass>(4);
void UART4_IRQHandler(void)
{
	if (uart4)
	{
		uart4->IRQHandler();
	}
}
#endif

#if defined(ENABLE_UART5)
#ifdef USING_CHIMERA
UARTClass_sPtr uart5;
#else
UARTClass_sPtr uart5 = boost::make_shared<UARTClass>(5);
#endif

void UART5_IRQHandler(void)
{
	if (uart5)
	{
		uart5->IRQHandler();
	}
}
#endif

#if defined(ENABLE_UART7)
#ifdef USING_CHIMERA
UARTClass_sPtr uart7;
#else
UARTClass_sPtr uart7 = boost::make_shared<UARTClass>(7);
#endif

void UART7_IRQHandler(void)
{
	if (uart7)
	{
		uart7->IRQHandler();
	}
}
#endif

#if defined(ENABLE_UART8)
#ifdef USING_CHIMERA
UARTClass_sPtr uart8;
#else
UARTClass_sPtr uart8 = boost::make_shared<UARTClass>(8);
#endif

void UART8_IRQHandler(void)
{
	if (uart8)
	{
		uart8->IRQHandler();
	}
}
#endif

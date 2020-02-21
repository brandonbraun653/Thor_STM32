/********************************************************************************
 *  File Name:
 *    uart.hpp
 *  
 *  Description:
 *    Common header for Thor UART that configures the driver based on which
 *    chip family is being compiled against.
 *  
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_UART_CONFIG_HPP
#define THOR_UART_CONFIG_HPP

#include <Thor/preprocessor.hpp>

/*-------------------------------------------------
Using the custom STM32 driver
-------------------------------------------------*/
#if defined( THOR_CUSTOM_DRIVERS ) && ( THOR_CUSTOM_DRIVERS == 1 )

#if defined( TARGET_STM32F4 )
#include <Thor/drivers/f4/uart/hw_uart_driver.hpp>
#endif

#if defined( TARGET_STM32F7 )
#include <Thor/drivers/f7/uart/hw_uart_driver.hpp>
#endif

/*-------------------------------------------------
Using the STM32HAL
-------------------------------------------------*/
#elif defined( THOR_STM32HAL_DRIVERS ) && ( THOR_STM32HAL_DRIVERS == 1 )
#if defined( TARGET_STM32F4 )
#include <stm32f4/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h>
#endif

#if defined( TARGET_STM32F7 )
#include <stm32f7/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_uart.h>
#endif

/*-------------------------------------------------
Using an unsupported driver?
-------------------------------------------------*/
#else
#error Unknown Thor UART driver implementation
#endif /* THOR_CUSTOM_DRIVERS */

#endif /* !THOR_UART_CONFIG_HPP */
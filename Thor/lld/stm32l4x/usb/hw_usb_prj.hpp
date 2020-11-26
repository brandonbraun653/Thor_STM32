/********************************************************************************
 *  File Name:
 *    hw_usb_prj.hpp
 *
 *  Description:
 *    Pulls in target specific definitions and resources used in the actual driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_HW_USB_PROJECT_HPP
#define THOR_HW_USB_PROJECT_HPP

/*------------------------------------------------
All STM32L4 devices
------------------------------------------------*/
#include <Thor/lld/stm32l4x/usb/variant/hw_usb_register_stm32l4xxxx.hpp>

/*------------------------------------------------
Chip specific STM32L4 devices
------------------------------------------------*/
#if defined( STM32L432xx )
#include <Thor/lld/stm32l4x/usb/variant/hw_usb_register_stm32l432kc.hpp>
#endif

#endif /* !THOR_HW_USB_PROJECT_HPP */

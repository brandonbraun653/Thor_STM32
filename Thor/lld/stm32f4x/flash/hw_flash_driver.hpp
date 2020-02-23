/********************************************************************************
 *  File Name:
 *    hw_flash_driver.hpp
 *
 *  Description:
 *    Implements the STM32F4 Flash driver interface
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_HW_DRIVER_FLASH_HPP
#define THOR_HW_DRIVER_FLASH_HPP

/* C++ Includes */
#include <cstdint>

/* Driver Includes */
#include <Thor/lld/stm32f4x/flash/hw_flash_types.hpp>

namespace Thor::LLD::FLASH
{
  void initialize();
}

#endif /* !THOR_HW_DRIVER_FLASH_HPP */
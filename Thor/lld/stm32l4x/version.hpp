/********************************************************************************
 *  File Name:
 *    version.hpp
 *
 *  Description:
 *    Low level driver versioning information for STM32L4 chips
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef THOR_LLD_STM32L4_VERSION_HPP
#define THOR_LLD_STM32L4_VERSION_HPP

/* STL Includes */
#include <string>

namespace Thor::LLD
{
  static const std::string_view VersionSTM32L4 = "1.0#";
}

#endif  /* !THOR_LLD_STM32L4_VERSION_HPP */

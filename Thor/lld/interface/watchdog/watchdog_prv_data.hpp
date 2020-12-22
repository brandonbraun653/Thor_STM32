/********************************************************************************
 *  File Name:
 *    watchdog_prv_data.hpp
 *
 *  Description:
 *    Declaration of data that must be defined by the LLD implementation or is
 *    shared among all possible drivers.
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#pragma once
#ifndef THOR_LLD_WATCHDOG_DATA
#define THOR_LLD_WATCHDOG_DATA

/* STL Includes */
#include <cstddef>

/* Chimera Includes */
#include <Chimera/watchdog>

/* Thor Includes */
#include <Thor/hld/dma/hld_dma_intf.hpp>
#include <Thor/lld/common/macros.hpp>
#include <Thor/lld/common/types.hpp>
#include <Thor/lld/interface/watchdog/watchdog_detail.hpp>
#include <Thor/lld/interface/interrupt/interrupt_detail.hpp>

namespace Thor::LLD::Watchdog
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/

  /*-------------------------------------------------------------------------------
  Project Defined Constants
  -------------------------------------------------------------------------------*/

  /*-------------------------------------------------------------------------------
  Peripheral Instances:
    Memory mapped structures that allow direct access to peripheral registers
  -------------------------------------------------------------------------------*/
#if defined( STM32_IWDG1_PERIPH_AVAILABLE )
  extern RegisterMap *IWDG1_PERIPH;
#endif

#if defined( STM32_WWDG1_PERIPH_AVAILABLE )
  extern RegisterMap *WWDG1_PERIPH;
#endif


  /*-------------------------------------------------------------------------------
  Configuration Maps:
    These convert high level configuration options into low level register config
    options. The idea is to allow the user to specify some general options, then
    convert that over to what the peripheral understands during config/init steps.
  -------------------------------------------------------------------------------*/
  namespace ConfigMap
  {
    // extern LLD_CONST <some_map>
  }    // namespace ConfigMap


  /*-------------------------------------------------------------------------------
  Peripheral Resources
  -------------------------------------------------------------------------------*/
  namespace Resource
  {
  }    // namespace Resource
}    // namespace Thor::LLD::Watchdog

#endif /* !THOR_LLD_WATCHDOG_DATA */
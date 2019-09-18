/********************************************************************************
 *   File Name:
 *    hw_iwdg_driver.hpp
 *
 *   Description:
 *    Declares the low level hardware watchdog driver interface
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#ifndef THOR_HW_IWDG_DRIVER_HPP
#define THOR_HW_IWDG_DRIVER_HPP

/* Driver Includes */
#include <Thor/headers.hpp>
#include <Thor/drivers/model/watchdog_model.hpp>

#if defined( TARGET_STM32F4 ) && ( THOR_DRIVER_WATCHDOG == 1 )
namespace Thor::Driver::IWDG
{
  class Driver : public Thor::Driver::Watchdog::Basic
  {
  public:
    void kick() final override;

    size_t maxDelay( const uint32_t prescaler ) final override;

    size_t minDelay( const uint32_t prescaler ) final override;

    
    void unlock();

  private:

  };
}    // namespace Thor::Driver::IWDG

#endif /* TARGET_STM32F4 && THOR_DRIVER_WATCHDOG */
#endif /* !THOR_HW_IWDG_DRIVER_HPP */
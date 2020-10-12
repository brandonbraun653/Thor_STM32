/********************************************************************************
 *  File Name:
 *    watchdog.hpp
 *
 *  Description:
 *    Thor interface to the STM32 watchdog hardware.
 *
 *  2019-2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C++ Includes */
#include <cstdint>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/thread>
#include <Chimera/watchdog>

namespace Thor::Watchdog
{
  /*-------------------------------------------------------------------------------
  Public Functions
  -------------------------------------------------------------------------------*/
  Chimera::Status_t reset();
  Chimera::Watchdog::Driver_sPtr getDriver( const Chimera::Watchdog::Channel channel );

  /**
   *  Initialize the WWDG driver
   *
   *  @return Chimera::Status_t
   */
  Chimera::Status_t initializeWWDG();

  /**
   *  Initialize the WWDG driver
   *
   *  @return Chimera::Status_t
   */
  Chimera::Status_t initializeIWDG();


  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /**
   *   A high resolution Watchdog peripheral driven by PCLK1 off the AHB bus. This
   *   watchdog is intended to protect against software faults and has more advanced
   *   capabilities than the Independent Watchdog.
   */
  class Window : virtual public Chimera::Watchdog::HWInterface, public Chimera::Threading::Lockable
  {
  public:
    Window();
    ~Window();

    Chimera::Status_t initialize( const uint32_t timeout_mS, const uint8_t windowPercent = 100 ) final override;

    Chimera::Status_t start() final override;

    Chimera::Status_t stop() final override;

    Chimera::Status_t kick() final override;

    Chimera::Status_t pauseOnDebugHalt( const bool enable ) final override;

    size_t getTimeout() final override;

    size_t maxTimeout() final override;

    size_t minTimeout() final override;

  private:
    uint32_t currentPrescaler;
  };

  /**
   *   A low resolution Watchdog peripheral driven by the LSI clock, which is
   *   independent from the main system clock. This particular watchdog is intended
   *   to protect against issues deriving from a faulty system clock that would not
   *   trip the window watchdog.
   */
  class Independent : virtual public Chimera::Watchdog::HWInterface, public Chimera::Threading::Lockable
  {
  public:
    Independent();
    ~Independent();

    Chimera::Status_t initialize( const uint32_t timeout_mS, const uint8_t windowPercent = 0 ) final override;

    Chimera::Status_t start() final override;

    Chimera::Status_t stop() final override;

    Chimera::Status_t kick() final override;

    Chimera::Status_t pauseOnDebugHalt( const bool enable ) final override;

    size_t getTimeout() final override;

    size_t maxTimeout() final override;

    size_t minTimeout() final override;

  private:
    uint32_t currentPrescaler;
  };

}    // namespace Thor::Watchdog

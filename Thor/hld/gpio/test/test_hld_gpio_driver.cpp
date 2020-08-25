/********************************************************************************
 *  File Name:
 *    test_hld_gpio_driver.cpp
 *
 *	 Description:
 *    Tests the Thor GPIO high level driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#if defined( THOR_HLD_TEST ) || defined( THOR_HLD_TEST_GPIO )

/* GTest Includes */
#include "gtest/gtest.h"

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/gpio>

/* Thor Includes */
#include <Thor/cfg>
#include <Thor/gpio>
#include <Thor/hld/gpio/hld_gpio_chimera.hpp>
#include <Thor/hld/gpio/test/test_fixture_gpio.hpp>
#include <Thor/lld/interface/gpio/mock/gpio_mock.hpp>

namespace Thor::HLD::GPIO
{

  TEST_F(TestFixture, DriverConstructor)
  {
    auto DoIFailCreation = Thor::GPIO::Driver();
    SUCCEED();
  }

  TEST_F(TestFixture, InitFromConfig)
  {
    auto driver = Thor::GPIO::Driver();
    Chimera::GPIO::PinInit pinInit;

    EXPECT_EQ( 0, 0 );
  }

  TEST_F(TestFixture, FreeFunc_Initialize)
  {
    using ::testing::Return;

    Thor::LLD::GPIO::Mock::ModuleMock& lld = Thor::LLD::GPIO::Mock::getMockObject();

    EXPECT_CALL( lld, initialize() ).Times(1);

    Thor::GPIO::initialize();
  }
}

#endif

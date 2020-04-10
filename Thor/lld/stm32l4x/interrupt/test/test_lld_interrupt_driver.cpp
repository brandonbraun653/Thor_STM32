/********************************************************************************
 *  File Name:
 *    test_lld_interrupt_driver.cpp
 *
 *	 Description:
 *    Tests the Thor INTERRUPT low level driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 *******************************************************************************/

#if defined( THOR_LLD_TEST_INTERRUPT )
/* GTest Includes */
#include "gtest/gtest.h"

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/interrupt>

/* Thor Includes */
#include <Thor/cfg>
#include <Thor/lld/interface/interrupt/interrupt.hpp>

TEST(Compiler, CanCompile)
{
  EXPECT_EQ(0, 0);
  Thor::LLD::INTERRUPT::initialize();
}

#endif  /* THOR_LLD_TEST_INTERRUPT */

/*
 * Generated by erpcgen 1.6.0 on Fri Jun  1 15:06:24 2018.
 *
 * AUTOGENERATED - DO NOT EDIT
 */


#if !defined(_uart_rpc_h_)
#define _uart_rpc_h_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "erpc_version.h"
#include "erpc_crc16.h"

#if 10600 != ERPC_VERSION_NUMBER
#error "The generated shim code version is different to the rest of eRPC code."
#endif


#if !defined(ERPC_GENERATED_CRC) || ERPC_GENERATED_CRC != 46132
#error "Macro 'ERPC_GENERATED_CRC' should be defined with value 46132."
#endif

#if !defined(ERPC_TYPE_DEFINITIONS)
#define ERPC_TYPE_DEFINITIONS

#endif // ERPC_TYPE_DEFINITIONS

/*! @brief TEST_UART identifiers */
enum _TEST_UART_ids
{
    kTEST_UART_service_id = 1,
    kTEST_UART_turnGreenLEDON_id = 1,
    kTEST_UART_turnGreenLEDOFF_id = 2,
    kTEST_UART_setupSerialUnderTest_id = 3,
    kTEST_UART_testStart_blockingTX_id = 4,
    kTEST_UART_testEnd_blockingTX_id = 5,
};

#if defined(__cplusplus)
extern "C" {
#endif

//! @name TEST_UART
//@{
void turnGreenLEDON(void);

void turnGreenLEDOFF(void);

void setupSerialUnderTest(void);

void testStart_blockingTX(void);

void testEnd_blockingTX(void);
//@} 

#if defined(__cplusplus)
}
#endif

#endif // _uart_rpc_h_

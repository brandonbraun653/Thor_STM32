#
# Generated by erpcgen 1.6.0 on Fri Jun  1 15:06:24 2018.
#
# AUTOGENERATED - DO NOT EDIT
#

import erpc

# Abstract base class for TEST_UART
class ITEST_UART(object):
    SERVICE_ID = 1
    TURNGREENLEDON_ID = 1
    TURNGREENLEDOFF_ID = 2
    SETUPSERIALUNDERTEST_ID = 3
    TESTSTART_BLOCKINGTX_ID = 4
    TESTEND_BLOCKINGTX_ID = 5

    def turnGreenLEDON(self):
        raise NotImplementedError()

    def turnGreenLEDOFF(self):
        raise NotImplementedError()

    def setupSerialUnderTest(self):
        raise NotImplementedError()

    def testStart_blockingTX(self):
        raise NotImplementedError()

    def testEnd_blockingTX(self):
        raise NotImplementedError()



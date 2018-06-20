#include <Thor/include/config.hpp>
#include <Thor/include/macro.hpp>
#include <Thor/include/print.hpp>

/* VERY basic float->string with 4 decimal places of precision */
std::string float2String(float number)
{
	char buff[50];
	std::string out;
	
	const char *tmpSign = (number < 0) ? "-" : "";
	float tmpVal = (number < 0) ? -number : number;

	int tmpInt1 = tmpVal;                  // Get the integer
	float tmpFrac = tmpVal - tmpInt1;      // Get fraction
	int tmpInt2 = trunc(tmpFrac * 10000);  // Turn into integer

    // Print as parts, note that you need 0-padding for fractional bit.
	std::sprintf(buff, "%s%d.%04d", tmpSign, tmpInt1, tmpInt2);
	
	out = buff;
	return out;
}


/* Unable to overload the Newlib Nano _write _read stubs if using the Visual GDB
 * Sysprogs Profiler. The profiler contains its own implementation of _write. */
 //&& !defined(USING_VGDB_PROFILER)
#if USE_SERIAL_DEBUG_OUTPUT 
#include <Thor/include/serial.hpp>
using namespace Thor::Peripheral::Serial;
using namespace Thor::Definitions::Serial;
using namespace Thor::Definitions::GPIO;

static SerialClass_uPtr serial;

extern "C" SerialPins serialDebugPinConfig;

void setupSTDIO()
{
	//Note that serialDebugPinConfig is used with C linkage! If getting errors here, you probably didn't instantiate it right.
	#if USE_SERIAL_DEBUG_EXT_PINS
		serial = boost::movelib::unique_ptr<SerialClass>(new SerialClass(SERIAL_DEBUG_CHANNEL, &serialDebugPinConfig));
	#else
		serial = boost::movelib::unique_ptr<SerialClass>(new SerialClass(SERIAL_DEBUG_CHANNEL, nullptr));
	#endif

	if (serial)
	{
		serial->begin(SERIAL_DEBUG_BAUDRATE);
		serial->setMode(SubPeripheral::TXRX, Modes::BLOCKING);
	}
	else
	{
		INSERT_BREAKPOINT;
	}	
}


#ifdef __cplusplus
extern "C" {
#endif
	int _write(int file, const char* buf, int len)
	{
		if (serial)
		{
			serial->write((uint8_t*)buf, len);
		}
		return len;
	}

	int _read(int file, const char* buf, int len)
	{
		if (serial)
		{
			serial->readSync((uint8_t*)buf, len);
			return len;
		}
		else
		{
			return 0;
		}
	}

#ifdef __cplusplus
}
#endif

#endif /* !USE_SERIAL_DEBUG_OUTPUT */
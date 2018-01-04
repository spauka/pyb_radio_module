/*
The MIT License (MIT)

Copyright (c) 2017 Sebastian Pauka
Copyright (c) 2016 British Broadcasting Corporation.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef NCSS_PYB_RADIO_H
#define NCSS_PYB_RADIO_H

#include "mbed.h"

#include "MicroBitSystemTimer.h"
#include "MicroBitCompat.h"
#include "ManagedString.h"
#include "MicroBitEvent.h"
#include "NotifyEvents.h"

#include "MicroBitThermometer.h"
#include "MicroBitSerial.h"
#include "MicroBitIO.h"

#include "MicroBitFiber.h"
#include "MicroBitMessageBus.h"

#include "MicroBitRadio.h"

// Module::flags
#define MODULE_INITIALIZED                    0x01

/**
  * Class definition for a NCSS PyBoard Radio device.
  *
  * Represents the device as a whole, and includes member variables that represent various device drivers
  * used to control aspects of the device.
  *
  * It is basically a very cut-down version of the MicroBit class.
  */
class NCSSPybRadio
{
    private:

    uint8_t                     status;

    public:

    // Serial Interface
    MicroBitSerial              serial;

    // Persistent key value store
    MicroBitStorage             storage;

    // Device level Message Bus abstraction
    MicroBitMessageBus          messageBus;

    // Member variables to represent each of the core components on the device.
    MicroBitThermometer         thermometer;

    // A reference to the LED pin on the radio module
    MicroBitPin                 led_io;

    // Bluetooth related member variables.
    MicroBitRadio               radio;
    // Various functions to query the radio state
    uint8_t radio_enabled(void);
    uint8_t radio_channel(void);
    uint8_t radio_power(void);

    /**
      * Constructor.
      *
      * Create a representation of a NCSS Radio device, which includes member variables
      * that represent various device drivers.
      *
      * This is basically a very cut down version of the MicroBit class.
      */
    NCSSPybRadio();

    /**
      * Post constructor initialisation method.
      *
      * This call will initialised the scheduler.
      *
      * @note This method must be called before user code utilises any MicroBit library
      * code.
      */
    void init();

    /**
      * Return the friendly name for this device.
      *
      * @return A ManagedString representing the friendly name of this device.
      *
      * @code
      * ManagedString name = module.getName();
      * @endcode
      */
    static ManagedString getName();

    /**
      * Return the serial number of this device.
      *
      * @return A ManagedString representing the serial number of this device.
      *
      * @code
      * ManagedString serialNumber = module.getSerial();
      * @endcode
      */
    static ManagedString getSerial();

    /**
      * Will reset the module when called.
      *
      * @code
      * module.reset();
      * @endcode
      */
    void reset();

    /**
      * Delay execution for the given amount of time.
      *
      * If the scheduler is running, this will deschedule the current fiber and perform
      * a power efficient, concurrent sleep operation.
      *
      * If the scheduler is disabled or we're running in an interrupt context, this
      * will revert to a busy wait.
      *
      * Alternatively: wait, wait_ms, wait_us can be used which will perform a blocking sleep
      * operation.
      *
      * @param milliseconds the amount of time, in ms, to wait for. This number cannot be negative.
      *
      * @return MICROBIT_OK on success, MICROBIT_INVALID_PARAMETER milliseconds is less than zero.
      *
      * @code
      * module.sleep(20); //sleep for 20ms
      * @endcode
      *
      * @note This operation is currently limited by the rate of the system timer, therefore
      *       the granularity of the sleep operation is limited to 6 ms unless the rate of
      *       the system timer is modified.
      */
    void sleep(uint32_t milliseconds);

    /**
      * Determine the time since this microcontroller was last reset.
      *
      * @return The time since the last reset, in milliseconds.
      *
      * @note This will value overflow after 1.6 months.
      */
      //TODO: handle overflow case.
    unsigned long systemTime();

    /**
      * Determine the version of the micro:bit runtime currently in use.
      *
      * @return A textual description of the version of the micro:bit runtime that
      *         is currently running on this device.
      */
    const char *systemVersion();

};

/**
  * Return the friendly name for this device.
  *
  * @return A ManagedString representing the friendly name of this device.
  *
  * @code
  * ManagedString name = module.getName();
  * @endcode
  */
inline ManagedString NCSSPybRadio::getName()
{
    return ManagedString(microbit_friendly_name());
}

/**
  * Return the serial number of this device.
  *
  * @return A ManagedString representing the serial number of this device.
  *
  * @code
  * ManagedString serialNumber = module.getSerial();
  * @endcode
  */
inline ManagedString NCSSPybRadio::getSerial()
{
    // We take to 16 bit numbers here, as we want the full range of ID bits, but don't want negative numbers...
    int n1 = microbit_serial_number() & 0xffff;
    int n2 = (microbit_serial_number() >> 16) & 0xffff;

    // Simply concat the two numbers.
    ManagedString s1(n1);
    ManagedString s2(n2);

    return s1 + s2;
}

/**
  * Will reset the module when called.
  *
  * @code
  * module.reset();
  * @endcode
  */
inline void NCSSPybRadio::reset()
{
    microbit_reset();
}

/**
  * Delay execution for the given amount of time.
  *
  * If the scheduler is running, this will deschedule the current fiber and perform
  * a power efficient, concurrent sleep operation.
  *
  * If the scheduler is disabled or we're running in an interrupt context, this
  * will revert to a busy wait.
  *
  * Alternatively: wait, wait_ms, wait_us can be used which will perform a blocking sleep
  * operation.
  *
  * @param milliseconds the amount of time, in ms, to wait for. This number cannot be negative.
  *
  * @return MICROBIT_OK on success, MICROBIT_INVALID_PARAMETER milliseconds is less than zero.
  *
  * @code
  * module.sleep(20); //sleep for 20ms
  * @endcode
  *
  * @note This operation is currently limited by the rate of the system timer, therefore
  *       the granularity of the sleep operation is limited to 6 ms unless the rate of
  *       the system timer is modified.
  */
inline void NCSSPybRadio::sleep(uint32_t milliseconds)
{
    fiber_sleep(milliseconds);
}

/**
  * Determine the time since this module was last reset.
  *
  * @return The time since the last reset, in milliseconds.
  *
  * @note This will value overflow after 1.6 months.
  */
inline unsigned long NCSSPybRadio::systemTime()
{
    return system_timer_current_time();
}


/**
  * Determine the version of the micro:bit runtime currently in use.
  *
  * @return A textual description of the version of the micro:bit runtime that
  *         is currently running on this device.
  */
inline const char *NCSSPybRadio::systemVersion()
{
    return microbit_dal_version();
}
#endif

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

/*
 * The underlying Nordic libraries that support BLE do not compile cleanly with the stringent GCC settings we employ
 * If we're compiling under GCC, then we suppress any warnings generated from this code (but not the rest of the DAL)
 * The ARM cc compiler is more tolerant. We don't test __GNUC__ here to detect GCC as ARMCC also typically sets this
 * as a compatability option, but does not support the options used...
 */
#if !defined(__arm)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "NCSSPybRadio.h"

#include "nrf_soc.h"

/*
 * Return to our predefined compiler settings.
 */
#if !defined(__arm)
#pragma GCC diagnostic pop
#endif

/**
  * Constructor.
  *
  * Create a representation of the NCSS Radio device, which includes member variables
  * that represent various device drivers used to control portions of the board.
  *
  * This is basically a very cut down version of a microbit with pins remapped based
  * on the build of the radio.
  */
NCSSPybRadio::NCSSPybRadio() :
    serial(P0_8, P0_9),
    storage(),
    messageBus(),
    thermometer(storage),
    led_io(MICROBIT_ID_IO_P0, P0_21, PIN_CAPABILITY_STANDARD),
    radio()
{
    // Clear our status
    status = 0;

    // Radio starts disabled
    radio_enabled = 0;
}

/**
  * Post constructor initialisation method.
  *
  * This call will initialize the scheduler.
  *
  * @code
  * module.init();
  * @endcode
  *
  * @note This method must be called before user code utilises any microbit modules.
  */
void NCSSPybRadio::init()
{
    if (status & MODULE_INITIALIZED)
        return;

    // Bring up fiber scheduler.
    scheduler_init(messageBus);

    status |= MODULE_INITIALIZED;
}


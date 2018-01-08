/*
The MIT License (MIT)

Copyright (c) 2017 Sebastian Pauka

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

#include "mbed.h"

#include "NCSSPybRadio.h"
#include "SPIRadio.h"
#include "SPISlaveExt.h"

#include YOTTA_BUILD_INFO_HEADER
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/**
 * Get version info
 */
const char * version_info(void) {
    return "Quokka Radio r." TOSTRING(YOTTA_BUILD_VCS_DESCRIPTION);
}

NCSSPybRadio module;
// For the quokka
//SPISlave spi(P0_22, P0_23, P0_21, P0_24); // MOSI, MISO, SCLK, CS
// For the test board
SPISlaveExt spi(P0_13, P0_12, P0_9, P0_8); // MOSI, MISO, SCLK, CS

// Allocate space for a message buffer
uint8_t radio_buffer_len;
uint8_t radio_buffer[64];
uint8_t io_buffer[64];

void onRadioMsg(MicroBitEvent e) {
    ManagedString s = module.radio.datagram.recv();

    // Save the data to the radio buffer
    // Note: we sholdn't have concurrency issues here
    // as this should only run at the end of the main event
    // loop.
    memcpy(radio_buffer, s.toCharArray(), s.length()+1);
    radio_buffer_len = s.length();

    // Let the message get handled in the main loop.
    return;
}

int main()
{
    uint8_t pin_state = 0;

    // Initialise the module and radio
    module.init();
    module.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onRadioMsg);
    module.radio.enable();

    //led.period_us(100);

    // Initialize SPI slave settings
    spi.format(8, 0); // 8bits per frame, default polarity+phase
    // Prime with a default response
    spi.reply(0x00);

    int r = 0;
    while (true) {
        // Check whether we've received a message on SPI
        r = spi.receive();
        // If we have, handle it
        if (r) {
            spi_op_status_t success = spi.read_buffer(io_buffer, 64, 0);
            if (success != SPI_OP_SUCCESS)
                continue;
            spi_radio_cmds_t cmd = (spi_radio_cmds_t) io_buffer[0];
            spi_cmd_switch(cmd, io_buffer, r);
            //led.pulsewidth_us(1* (pin_state ^= 1));
            module.led_io.setAnalogValue(5 * (pin_state ^= 1));
        }

        r = module.radio.dataReady();

        // Run any waiting events (i.e. a message has arrived?)
        schedule();

        fiber_sleep(1);
    }

    // We will never get here, but this would put us in a waiting loop.
    // Use if we don't use main.
    release_fiber();
}


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
#include "SPIRadioCmds.h"

// We need access to the module/spi instances
extern NCSSPybModule module;
extern SPISlave spi;

// Loop over the command buffer and 
void spi_cmd_switch(spi_radio_cmds_t cmd) {
    switch(cmd) {
        case SPI_NOOP:
            // NOOP
            spi.reply(SPI_SUCCESS);
            break;

        // Radio State
        case SPI_RADIO_STATE_ENABLE:
            module.radio.enable(); // TODO: Check success
            module.radio_enabled = 1;
            spi.reply(SPI_SUCCESS);
            break;
        case SPI_RADIO_STATE_DISABLE:
            module.radio.disable(); // TODO: Check success
            module.radio_enabled = 0;
            spi.reply(SPI_SUCCESS);
            break;
        case SPI_RADIO_STATE_QUERY:
            spi.reply(SPI_SUCCESS); // TODO: Multiple responses
            spi.reply(module.radio_enabled);
            break;
    }
}

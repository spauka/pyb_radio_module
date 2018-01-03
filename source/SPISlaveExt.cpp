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
#include "SPIRadioCmds.h"
#include "SPISlaveExt.h"

/**
 * Constructor: initialize empty buffers and set up pins
 */
SPISlaveExt::SPISlaveExt(PinName mosi, PinName miso, PinName sclk, PinName ssel) :
    SPISlave(mosi, miso, sclk, ssel)
{
    memset(inputBuf, 0x00, SPI_IOBUF_SIZE);
    memset(outputBuf, 0x00, SPI_IOBUF_SIZE);
}

/**
 * read multiple bytes into the buffer
 */
void SPISlaveExt::read_buffer(uint8_t len, uint8_t c) {
    if (len == 0 || len > SPI_IOBUF_SIZE)
        return;

    // Set the output characters
    memset(outputBuf, c, SPI_IOBUF_SIZE);

    // Do the read and write
    do_read_write(len);
}

/**
 * read_write buffer
 */
void SPISlaveExt::read_write_buffer(uint8_t *outBuf, uint8_t len) {
    // First let's check that the parameters are valid
    if (outBuf == NULL)
        return;
    if (len == 0 || len > SPI_IOBUF_SIZE)
        return;

    // Copy data into buffers
    memcpy(outputBuf, outBuf, len);

    // do the reading and writing
    do_read_write(len);
}

/**
 * do the actual reading and writing
 */
void SPISlaveExt::do_read_write(uint8_t len) {
    // Save the old read/write destination pointers/SPI state
    uint8_t *old_rx_buffer = (uint8_t*) _spi.spis->RXDPTR;
    uint8_t old_maxrx = _spi.spis->MAXRX;
    uint8_t *old_tx_buffer = (uint8_t*)_spi.spis->TXDPTR;
    uint8_t old_maxtx = _spi.spis->MAXTX;
    uint8_t old_shorts = _spi.spis->SHORTS;

    // Acquire the SPI semaphore to change the destination pointers
    _spi.spis->TASKS_ACQUIRE = 1;
    while (_spi.spis->EVENTS_ACQUIRED == 0);

    // Disable the short on transaction end
    _spi.spis->SHORTS &= ~(SPIS_SHORTS_END_ACQUIRE_Enabled << SPIS_SHORTS_END_ACQUIRE_Pos);

    // Set up new pointers
    _spi.spis->RXDPTR = (uint32_t) inputBuf;
    _spi.spis->MAXRX = len;
    _spi.spis->TXDPTR = (uint32_t) outputBuf;
    _spi.spis->MAXTX = len;
    _spi.spis->ORC = (uint8_t) SPI_OVERFLOW;

    // Release the semaphore
    _spi.spis->EVENTS_END = 0;
    _spi.spis->EVENTS_ACQUIRED = 0;
    _spi.spis->TASKS_RELEASE = 1;

    // Then, check whether we're done reading each time we hit the end event
    do {
        while(_spi.spis->EVENTS_END == 0);
        _spi.spis->EVENTS_END = 0;
    } while (_spi.spis->AMOUNTRX != len);

    // Once we're done, restore pointers and return
    _spi.spis->TASKS_ACQUIRE = 1;
    while (_spi.spis->EVENTS_ACQUIRED == 0);

    _spi.spis->SHORTS = old_shorts;
    _spi.spis->RXDPTR = (uint32_t) old_rx_buffer;
    _spi.spis->MAXRX = old_maxrx;
    _spi.spis->TXDPTR = (uint32_t) old_tx_buffer;
    _spi.spis->MAXTX = old_maxtx;

    // Release the semaphore
    _spi.spis->EVENTS_END = 0;
    _spi.spis->EVENTS_ACQUIRED = 0;
    _spi.spis->TASKS_RELEASE = 1;
}



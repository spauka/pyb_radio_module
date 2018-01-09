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
 * We leverage the work that SPISlave does here, but then overwrite
 * the RXD/TXD buffers with our own that can hold more bits
 */
SPISlaveExt::SPISlaveExt(PinName mosi, PinName miso, PinName sclk, PinName ssel) :
    SPISlave(mosi, miso, sclk, ssel)
{
    // At this point, we have an SPISlave object set up to recieve single bytes
    // per transaction. Let's overwrite this with a longer buffer.
    acquire_sem();
    _spi.spis->TXDPTR = (uint32_t) outputBuf;
    _spi.spis->MAXTX = SPI_IOBUF_SIZE;
    _spi.spis->RXDPTR = (uint32_t) inputBuf;
    _spi.spis->MAXRX = SPI_IOBUF_SIZE;

    // Set up overread and busy characters (used when semaphore is locked)
    _spi.spis->DEF = SPI_PERIPH_BUSY;
    _spi.spis->ORC = SPI_OVERFLOW;

    // Set up a short to automatically acquire the semaphore after a message is
    // receive. This is to allow us to do something with the message before
    // it is overwritten.
    _spi.spis->SHORTS |= (SPIS_SHORTS_END_ACQUIRE_Enabled << SPIS_SHORTS_END_ACQUIRE_Pos);

    // Release the semaphore, the device is ready to work
    release_sem();
}

/**
 * Acquire the semaphore to allow safely changing buffers
 */
void SPISlaveExt::acquire_sem(void) {
    // Check we don't already hold the semaphore
    if (_spi.spis->SEMSTAT == 1)
        return;
    // Start the acquisition task
    _spi.spis->EVENTS_ACQUIRED = 0;
    _spi.spis->TASKS_ACQUIRE = 1;
    // Wait until it's ours
    while (_spi.spis->EVENTS_ACQUIRED == 0) sleep();
    // Then we can return
    return;
}

/**
 * Release the semaphore, prepare for next operation
 */
void SPISlaveExt::release_sem(void) {
    _spi.spis->EVENTS_ACQUIRED = 0;
    _spi.spis->TASKS_RELEASE = 1;
    return;
}

/**
 * Return the number of available bytes in the buffer
 */
int SPISlaveExt::receive(void) {
    if (SPISlave::receive())
        return _spi.spis->AMOUNTRX;
    return 0;
}

/**
 * read multiple bytes into a buffer
 */
spi_op_status_t SPISlaveExt::read_buffer(uint8_t *buffer, uint8_t maxLen, uint8_t release) {
    int recv = receive();
    // Check we have a new receive message and we can safely access the buffers
    if (recv == 0 || _spi.spis->SEMSTAT != 1)
        return SPI_OP_NOT_READY;
    // Check that the input buffer is of sufficient length
    if (maxLen < recv)
        return SPI_OP_INSUFFICIENT_BUFFER;

    // Copy output into buffers
    memcpy(buffer, inputBuf, recv);

    // Zero the input buffer and unset the receive bit
    memset(inputBuf, 0x00, recv);
    _spi.spis->EVENTS_END = 0;

    // Release the semaphore if we think we are done
    if (release)
        release_sem();

    // Done
    return SPI_OP_SUCCESS;
}

/**
 * prepare a response message
 * outBuffer is the response that we want to send
 * len is the length of the response
 * release tells us whether we are ready immediately after this operation.
 *   If we still want to oparate on this data then we should set this to 0,
 *   but we then MUST call release before the device will send this message.
 */
spi_op_status_t SPISlaveExt::reply_buffer(uint8_t *buffer, uint8_t len, uint8_t release) {
    // Check that the length of the message is valid
    if (len == 0 || len > SPI_IOBUF_SIZE)
        return SPI_OP_INSUFFICIENT_BUFFER;

    // Make sure we can safely access the buffers
    if (_spi.spis->SEMSTAT != 1)
        return SPI_OP_NOT_READY;

    // copy output into buffer and set up sender
    memcpy(outputBuf, buffer, len);
    _spi.spis->MAXTX = len;

    // if we are ready to release the semaphore do it
    if (release)
        release_sem();

    // Done
    return SPI_OP_SUCCESS;
}

/**
 * Single byte response shortcut
 */
spi_op_status_t SPISlaveExt::reply(int d) {
    // Convert to a byte
    uint8_t byte = (uint8_t) (d & 0xFF);
    // Send a single byte, releasing semaphore at the same time
    return reply_buffer(&byte, 1, 1);
}

/**
 * Return the semaphore state
 */
uint32_t SPISlaveExt::sem_state(void) {
  return _spi.spis->SEMSTAT;
}

/**
 * Release semaphore once buffers are prepared for the next operation
 */
spi_op_status_t SPISlaveExt::release(void) {
    // Check we actually hold the semaphore
    if (_spi.spis->SEMSTAT != 1)
        return SPI_OP_OTHER_FAIL;
    // Release and succeed :)
    release_sem();
    return SPI_OP_SUCCESS;
}

/**
 * do the actual reading and writing
 */
//void SPISlaveExt::do_read_write(uint8_t len) {
    //// Save the old read/write destination pointers/SPI state
    //uint8_t *old_rx_buffer = (uint8_t*) _spi.spis->RXDPTR;
    //uint8_t old_maxrx = _spi.spis->MAXRX;
    //uint8_t *old_tx_buffer = (uint8_t*)_spi.spis->TXDPTR;
    //uint8_t old_maxtx = _spi.spis->MAXTX;
    //uint8_t old_shorts = _spi.spis->SHORTS;

    //// Acquire the SPI semaphore to change the destination pointers
    //_spi.spis->TASKS_ACQUIRE = 1;
    //while (_spi.spis->EVENTS_ACQUIRED == 0);

    //// Enable the short on transaction end
    //_spi.spis->SHORTS |= (SPIS_SHORTS_END_ACQUIRE_Enabled << SPIS_SHORTS_END_ACQUIRE_Pos);

    //// Set up new pointers
    //_spi.spis->RXDPTR = (uint32_t) inputBuf;
    //_spi.spis->MAXRX = len;
    //_spi.spis->TXDPTR = (uint32_t) outputBuf;
    //_spi.spis->MAXTX = len;
    //_spi.spis->ORC = (uint8_t) SPI_OVERFLOW;

    //// Release the semaphore
    //_spi.spis->EVENTS_END = 0;
    //_spi.spis->EVENTS_ACQUIRED = 0;
    //_spi.spis->TASKS_RELEASE = 1;

    //// Then, check whether we're done reading each time we hit the end event
    //uint32_t amount_rx = 0;
    //do {
        //while(_spi.spis->EVENTS_END == 0);
        //_spi.spis->EVENTS_END = 0;
        //_spi.spis->EVENTS_ACQUIRED = 0;
        //amount_rx += _spi.spis->AMOUNTRX;
    //} while (amount_rx < len);

    //// Once we're done, restore pointers and return
    //_spi.spis->TASKS_ACQUIRE = 1;
    //while (_spi.spis->EVENTS_ACQUIRED == 0);

    //_spi.spis->SHORTS = old_shorts;
    //_spi.spis->RXDPTR = (uint32_t) old_rx_buffer;
    //_spi.spis->MAXRX = old_maxrx;
    //_spi.spis->TXDPTR = (uint32_t) old_tx_buffer;
    //_spi.spis->MAXTX = old_maxtx;

    //// Release the semaphore
    //_spi.spis->EVENTS_END = 0;
    //_spi.spis->EVENTS_ACQUIRED = 0;
    //_spi.spis->TASKS_RELEASE = 1;
//}



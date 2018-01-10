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

#ifndef SPI_SLAVE_EXT_H
#define SPI_SLAVE_EXT_H

#include "mbed.h"

const uint16_t SPI_IOBUF_SIZE = 255;

typedef enum {
    SPI_OP_SUCCESS,
    SPI_OP_NOT_READY,
    SPI_OP_INSUFFICIENT_BUFFER,
    SPI_OP_OTHER_FAIL
} spi_op_status_t;

class SPISlaveExt : public SPISlave {
    private:
        // Allocate space for input and output
        uint8_t inputBuf[SPI_IOBUF_SIZE] = {0};
        uint8_t intputBuf_size;
        uint8_t outputBuf[SPI_IOBUF_SIZE] = {0};
        uint8_t outputBuf_size;

        // SPI Device Commands
        // Acquire and release semaphores for modifying buffers
        void acquire_sem(void);
        void release_sem(void);

        // Disable single-byte read/reply operation. They no longer make sense...
        int read(void);
    public:
        /**
         * Constructor: initialize empty buffers and set up pins
         */
        SPISlaveExt(PinName mosi, PinName miso, PinName sclk, PinName ssel);

        /**
         * number of received bytes
         */
        int receive(void);

        /**
         * read multiple bytes
         *
         * buffer is where we should place bytes
         * maxLen is the size of the buffer
         * release tells us whether to release the CPU semaphore after operating.
         *   If we want to respond we should keep hold of the semaphore, however
         *   we MUST then call release before the device will send any messages.
         *
         */
        spi_op_status_t read_buffer(uint8_t* buffer, uint8_t maxLen, uint8_t release = 1);

        /**
         * prepare a response message
         * outBuffer is the response that we want to send
         * len is the length of the response
         * release tells us whether we are ready immediately after this operation.
         *   If we still want to oparate on this data then we should set this to 0,
         *   but we then MUST call release before the device will send this message.
         */
        spi_op_status_t reply_buffer(uint8_t* outBuffer, uint8_t len, uint8_t release = 1);

        /**
         * Shortcut for a single byte response
         */
        spi_op_status_t reply(int d);

        /**
         * Check semaphore state
         */
        uint32_t sem_state(void);

        /**
         * Release CPU semaphore for next operation
         */
        spi_op_status_t release(void);
};

#endif

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

const uint8_t SPI_IOBUF_SIZE = 64;

class SPISlaveExt : public SPISlave {
    private:
        // Allocate space for input and output
        uint8_t inputBuf[SPI_IOBUF_SIZE];
        uint8_t intputBuf_size;
        uint8_t outputBuf[SPI_IOBUF_SIZE];
        uint8_t outputBuf_size;
    public:
        /**
         * Constructor: initialize empty buffers and set up pins
         */
        SPISlaveExt(PinName mosi, PinName miso, PinName sclk, PinName ssel);

        /**
         * read multiple bytes
         *
         * c is the character written out during read (defaults to 0x00)
         * len is the number of characters to read
         */
        void read_buffer(uint8_t len, uint8_t c=0x00);

        /**
         * read and write multiple bytes.
         */
        void read_write_buffer(uint8_t *outBuf, uint8_t len);

    private:
        /**
         * Internal function to read and write into our own buffers
         */
        void do_read_write(uint8_t len);
};

#endif

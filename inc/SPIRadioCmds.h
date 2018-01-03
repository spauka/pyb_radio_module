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

#ifndef SPI_RADIO_CMDS_H
#define SPI_RADIO_CMDS_H

// States
static const uint8_t SPI_STATE_ON = 0x01;
static const uint8_t SPI_STATE_OFF = 0x00;
static const uint8_t SPI_QUERY = 0x02;

// Peripherals and commands
static const uint8_t SPI_RADIO_STATE = 0x01 << 2;
static const uint8_t SPI_RADIO_CHAN = 0x02 << 2;
static const uint8_t SPI_RADIO_POWER = 0x03 << 2;
static const uint8_t SPI_MSG_AVAIL = 0x04 << 2;
static const uint8_t SPI_SEND_MSG = 0x05 << 2;
static const uint8_t SPI_RECV_MSG = 0x06 << 2;

// Cmds from master
typedef enum {
    SPI_NOOP = 0x00,
    // Radio State
    SPI_RADIO_STATE_DISABLE = SPI_RADIO_STATE | SPI_STATE_OFF,
    SPI_RADIO_STATE_ENABLE = SPI_RADIO_STATE | SPI_STATE_ON,
    SPI_RADIO_STATE_QUERY = SPI_RADIO_STATE | SPI_QUERY,
    // Radio Frequency
    SPI_RADIO_CHAN_SET = SPI_RADIO_CHAN,
    SPI_RADIO_CHAN_QUERY = SPI_RADIO_CHAN | SPI_QUERY,
    // Radio Transmit Power
    SPI_RADIO_POWER_SET = SPI_RADIO_POWER,
    SPI_RADIO_POWER_QUERY = SPI_RADIO_POWER | SPI_QUERY,
    // Message Available Query
    SPI_MSG_QUERY = SPI_MSG_AVAIL | SPI_QUERY,
    // Send and recieve commands
    SPI_SEND_CMD = SPI_SEND_MSG,
    SPI_RECV_CMD = SPI_RECV_MSG
} spi_radio_cmds_t;

// Responses
typedef enum {
    SPI_NOCMD = 0x00,
    SPI_SUCCESS = 0x01,
    SPI_OUT_OF_RANGE = 0x02,
    SPI_SUCCESS_AND_ENABLED = 0x03,
    SPI_SUCCESS_AND_DISABLED = 0x04,
    SPI_PERIPH_BUSY = 0xF0,
    SPI_FAIL = 0xFF
} spi_radio_responses_t;

// Message format looks like: (here represented as a comment since this would
// require variable length arrays...)
//typedef struct {
//    uint8_t msg_length;
//    uint8_t msg[0..64];
//    uint8_t chksum; // XOR of all previous bits excluding msg_length
//    spi_radio_response_t success;
//} __attribute__((packed)) msg_format;
//
//Note: The above message format is only sent for commands that have data
//      so the SPI_RADIO_STATE_* commands only send a response.
//
#endif

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
#include "SPISlaveExt.h"
#include "SPIRadio.h"
#include "SPIRadioCmds.h"

// We need access to the module/spi instances
extern NCSSPybRadio module;
extern SPISlaveExt spi;

// Radio messages buffer
extern uint8_t radio_buffer_len;
extern uint8_t radio_buffer[64];
// Version info prototype
const char* version_info(void);

/**
 * Calculate string checksum
 */
uint8_t calc_checksum(const uint8_t *buffer, const uint32_t length) {
    uint8_t chksum = buffer[0];
    for (uint32_t i = 1; i < length; i += 1)
        chksum ^= buffer[i];
    return chksum;
}

/**
 * Validate that a buffer has the correct format and length.
 * Return 0 if failed else return length
 */
uint8_t validate_packet(const uint8_t *io_buffer, const uint32_t length) {
    // First check the length is valid
    // It should be the length of the message plus the command, length and checksum
    if (length != (uint8_t) (io_buffer[1]+3))
        return 0;
    // Then calculate a checksum of the message
    uint8_t chksum = calc_checksum(io_buffer+2, io_buffer[1]);
    if (chksum != io_buffer[length-1])
        return 0;
    return io_buffer[1];
}

spi_op_status_t craft_packet(uint8_t *io_buffer, spi_radio_responses_t resp,
        const uint8_t *msg, const uint32_t length) {
    // Check that our message isn't too long
    if (length > 61)
        return SPI_OP_INSUFFICIENT_BUFFER;

    // Calculate our checksum
    uint8_t chksum = calc_checksum(msg, length);

    // Fill in our buffer
    io_buffer[0] = (uint8_t) resp;
    io_buffer[1] = (uint8_t) length;
    memcpy(io_buffer+2, msg, length);
    io_buffer[length+2] = chksum;

    return SPI_OP_SUCCESS;
}

// Loop over the command buffer and reply
void spi_cmd_switch(spi_radio_cmds_t cmd, uint8_t *io_buffer, const uint32_t length) {
    uint8_t check, response;
    uint32_t len;
    const char* version;
    // If the packet contains a payload, validate that the packet is not corrupt
    if (length > 1) {
        check = validate_packet(io_buffer, length);
        if (check == 0) {
            spi.reply(SPI_CHECKSUM_FAIL);
            return;
        }
    } else
        check = 0;
    // Choose the correct action
    switch(cmd) {
        case SPI_NOOP:
            // NOOP
            spi.reply(SPI_NOCMD);
            break;
        case SPI_VERSION:
            version = version_info();
            len = strlen(version_info())+1; // not forgetting 0 terminator
            craft_packet(io_buffer, SPI_SUCCESS, (const uint8_t *)version, len);
            spi.reply_buffer(io_buffer, len+3);
            break;
        // Radio State
        case SPI_RADIO_STATE_ENABLE:
            module.radio.enable(); // TODO: Check success
            spi.reply(SPI_SUCCESS);
            break;
        case SPI_RADIO_STATE_DISABLE:
            module.radio.disable(); // TODO: Check success
            spi.reply(SPI_SUCCESS);
            break;
        case SPI_RADIO_STATE_QUERY:
            if (module.radio_enabled())
                spi.reply(SPI_SUCCESS_AND_ENABLED);
            else
                spi.reply(SPI_SUCCESS_AND_DISABLED);
            break;
        // Radio channel
        case SPI_RADIO_CHAN_SET:
            if (check != 1) { // length must be 1
                spi.reply(SPI_INVALID_LENGTH);
                break;
            }
            if (io_buffer[2] > 100) { // Out of range
                spi.reply(SPI_OUT_OF_RANGE);
                break;
            }
            response = module.radio.setFrequencyBand(io_buffer[2]);
            if (response == MICROBIT_OK)
                spi.reply(SPI_SUCCESS);
            else
                spi.reply(SPI_OTHER_FAIL);
            break;
        case SPI_RADIO_CHAN_QUERY:
            response = module.radio_channel();
            craft_packet(io_buffer, SPI_SUCCESS, &response, 1);
            spi.reply_buffer(io_buffer, 4);
            break;
        // Radio Power
        case SPI_RADIO_POWER_SET:
            if (check != 1) { // length must be 1
                spi.reply(SPI_INVALID_LENGTH);
                break;
            }
            if (io_buffer[2] > 7) { // Out of range
                spi.reply(SPI_OUT_OF_RANGE);
                break;
            }
            response = module.radio.setTransmitPower(io_buffer[2]);
            if (response == MICROBIT_OK)
                spi.reply(SPI_SUCCESS);
            else
                spi.reply(SPI_OTHER_FAIL);
            break;
        case SPI_RADIO_POWER_QUERY:
            response = module.radio_power();
            craft_packet(io_buffer, SPI_SUCCESS, &response, 1);
            spi.reply_buffer(io_buffer, 4);
            break;
        // Message Queries
        case SPI_MSG_QUERY:
            if (radio_buffer_len > 0)
                spi.reply(SPI_MESSAGE);
            else
                spi.reply(SPI_NO_MESSAGE);
            break;
        case SPI_SEND_CMD:
            if (check == 0) {
                spi.reply(SPI_INVALID_LENGTH);
                break;
            }
            if (check > 61) {
                spi.reply(SPI_REPLY_OVERFLOW);
                break;
            }
            module.radio.datagram.send(io_buffer+2, io_buffer[1]);
            spi.reply(SPI_SUCCESS);
            break;
        case SPI_RECV_CMD:
            if (radio_buffer_len == 0) {
                spi.reply(SPI_NO_MESSAGE);
                break;
            }
            craft_packet(io_buffer, SPI_SUCCESS, radio_buffer, radio_buffer_len);
            spi.reply_buffer(io_buffer, radio_buffer_len+3);
            break;
        default:
            spi.reply(SPI_INVALID_COMMAND);
            break;
    }
}

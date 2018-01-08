# boot.py -- run on boot-up
# can run arbitrary Python, but best to keep it minimal

from pyb import delay
from machine import Pin, SPI

# States
SPI_STATE_ON = 0x01
SPI_STATE_OFF = 0x00
SPI_QUERY = 0x02

# Peripherals and commands
SPI_RADIO_STATE = 0x01 << 2
SPI_RADIO_CHAN = 0x02 << 2
SPI_RADIO_POWER = 0x03 << 2
SPI_MSG_AVAIL = 0x04 << 2
SPI_SEND_MSG = 0x05 << 2
SPI_RECV_MSG = 0x06 << 2

# Cmds from master
SPI_NOOP = 0x00
SPI_VERSION = SPI_QUERY
# Radio State
SPI_RADIO_STATE_DISABLE = SPI_RADIO_STATE | SPI_STATE_OFF
SPI_RADIO_STATE_ENABLE = SPI_RADIO_STATE | SPI_STATE_ON
SPI_RADIO_STATE_QUERY = SPI_RADIO_STATE | SPI_QUERY
# Radio Frequency
SPI_RADIO_CHAN_SET = SPI_RADIO_CHAN
SPI_RADIO_CHAN_QUERY = SPI_RADIO_CHAN | SPI_QUERY
# Radio Transmit Power
SPI_RADIO_POWER_SET = SPI_RADIO_POWER
SPI_RADIO_POWER_QUERY = SPI_RADIO_POWER | SPI_QUERY
# Message Available Query
SPI_MSG_QUERY = SPI_MSG_AVAIL | SPI_QUERY
# Send and recieve commands
SPI_SEND_CMD = SPI_SEND_MSG
SPI_RECV_CMD = SPI_RECV_MSG

# Responses
SPI_NOCMD = 0x00
SPI_SUCCESS = 0x01
SPI_OUT_OF_RANGE = 0x02
SPI_SUCCESS_AND_ENABLED = 0x03
SPI_SUCCESS_AND_DISABLED = 0x04
SPI_INVALID_LENGTH = 0x05
SPI_REPLY_OVERFLOW = 0x06
SPI_CHECKSUM_FAIL = 0x07
SPI_INVALID_COMMAND = 0x08
SPI_NO_MESSAGE = 0x10
SPI_MESSAGE = 0x11
SPI_PERIPH_BUSY = 0xF0
SPI_OVERFLOW = 0xF1
SPI_OTHER_FAIL = 0xFF

class Radio:
    def __init__(self, slave_select, spi):
        self.slave_select = slave_select
        self.spi = spi

    def version(self):
        response = write([SPI_VERSION])
        return bytes(read_packet(response)).decode('ascii').strip('\x00')

    def enable(self):
        write([SPI_RADIO_STATE_ENABLE])

    def disable(self):
        write([SPI_RADIO_STATE_DISABLE])

    def is_enabled(self):
        response = write([SPI_RADIO_STATE_QUERY])
        if response[0] == SPI_SUCCESS_AND_ENABLED:
            return True
        elif response[0] == SPI_SUCCESS_AND_DISABLED:
            return False
        return False

    def set_channel(self, channel):
        assert 0 <= channel <= 100, 'Channel must be between 1 and 100'
        write([SPI_RADIO_CHAN_SET, 1, channel&0xff, channel&0xff])

    def get_channel(self):
        response = write([SPI_RADIO_CHAN_QUERY])
        return read_packet(response)[0]

    def set_power(self, power):
        assert 0 <= power <= 7, 'Power must be between 0 and 7'
        write([SPI_RADIO_POWER_SET, 1, power, power])

    def get_power(self):
        response = write([SPI_RADIO_POWER_QUERY])
        return read_packet(response)[0]

    def is_message_available(self):
        response = write([SPI_MSG_QUERY])
        if response[0] == SPI_MESSAGE:
            return True
        elif response[0] == SPI_NO_MESSAGE:
            return False
        return False

    def send(self, message):
        # TODO: Write this!!!
        write([SPI_SEND_CMD])

    def receive(self):
        r = write([SPI_RECV_CMD])
        return read_packet(r)

    def write(self, data):
        data = bytearray(data)

        self.slave_select.value(0)
        self.spi.write(data)
        self.slave_select.value(1)

        # Wait until the radio is ready to respond
        self.slave_select.value(0)
        resp = self.spi.read(1, 0x00)
        while resp == SPI_PERIPH_BUSY:
            self.slave_select.value(1)
            pyb.udelay(100)
            self.slave_select.value(0)
            resp = self.spi.read(1, 0x00)

        data = bytearray(64)
        self.spi.readinto(data, 0x00)
        self.slave_select.value(1)

        return data.strip(bytes((0xf1,)))

    def read_packet(self, packet):
        status_code = packet[0]

        # Check for error codes
        if status_code not in (SPI_SUCCESS, SPI_SUCCESS_AND_ENABLED, SPI_SUCCESS_AND_DISABLED):
            raise RuntimeError("Radio Error. Status Code 0x%x" % status_code)

        # Get the data out of the packet
        length = packet[1]
        if length == 0:
            return bytearray()
        data = packet[2:2 + length]
        expected_checksum = packet[2 + length]

        # Calculate & compare checksum
        checksum = 0
        for d in data:
            checksum ^= d
        if checksum != expected_checksum:
            raise RuntimeError('Checksum did not match (actual: %d, expected: %d)' % (checksum, expected_checksum))

        return data

slave_select = Pin("Y5", Pin.OUT)
spi = SPI(2, baudrate=10000000)

slave_select.value(1)

radio = Radio(slave_select, spi)

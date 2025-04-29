/*
  This is a library written for the ST VL53L5CX Time-of-flight sensor
  SparkFun sells these at its website:
  https://www.sparkfun.com/products/18642

  Do you like this library? Help support open source hardware. Buy a board!

  Written by Ricardo Ramos  @ SparkFun Electronics, October 22nd, 2021
  This file implements the VL53L5CX I2C driver class.

  This library uses ST's VL53L5CX driver and parts of Simon Levy's VL53L5CX
  Arduino library available at https://github.com/simondlevy/VL53L5/tree/main/src/st

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SparkFun_VL53L5CX_IO.h"
#include "SparkFun_VL53L5CX_Library_Constants.h"

#include <stdio.h>

#define wireMaxPacketSize    32

#define lowByte(b)  (b & 0xff)
#define highByte(b) ((b >> 8) & 0xff)

bool SparkFun_VL53L5CX_IO::begin(byte address, I2C_Dev &wirePort)
{
    _address = address;
    _i2cPort = &wirePort;
    return isConnected();
}

bool SparkFun_VL53L5CX_IO::isConnected()
{
    // _i2cPort->beginTransmission(_address);
    // if (_i2cPort->endTransmission() != 0)
    //     return (false);
    return (true);
}

void SparkFun_VL53L5CX_IO::setAddress(uint8_t newAddress)
{
    _address = newAddress;
}

// Must be able to write 32,768 bytes at a time
uint8_t SparkFun_VL53L5CX_IO::writeMultipleBytes(uint16_t registerAddress, uint8_t *buffer, uint16_t bufferSize)
{
    // // Chunk I2C transactions into limit of 32 bytes (or wireMaxPacketSize)
    // uint8_t i2cError = 0;
    // uint32_t startSpot = 0;
    // uint32_t bytesToSend = bufferSize;
    // while (bytesToSend > 0)
    // {
    //     uint32_t len = bytesToSend;
    //     if (len > (wireMaxPacketSize - 2)) // Allow 2 byte for register address
    //         len = (wireMaxPacketSize - 2);

    //     _i2cPort->beginTransmission((uint8_t)_address);
    //     _i2cPort->write(highByte(registerAddress));
    //     _i2cPort->write(lowByte(registerAddress));

    //     // TODO write a subsection of the buffer rather than byte wise
    //     for (uint16_t x = 0; x < len; x++)
    //         _i2cPort->write(buffer[startSpot + x]); // Write a portion of the payload to the bus

    //     i2cError = _i2cPort->endTransmission(); // Release bus because we are writing the address each time
    //     if (i2cError != 0)
    //         return (i2cError); // Sensor did not ACK

    //     startSpot += len; // Move the pointer forward
    //     bytesToSend -= len;
    //     registerAddress += len; // Move register address forward
    // }
    // return (i2cError);

    uint8_t i2c_buffer[wireMaxPacketSize];

    uint32_t startSpot = 0;
    uint32_t bytesToSend = bufferSize;
    int i2cError;
    while (bytesToSend > 0)
    {
        uint32_t len = bytesToSend;
        if (len > (wireMaxPacketSize - 2)) // Allow 2 byte for register address
            len = (wireMaxPacketSize - 2);

        i2c_buffer[0] = highByte(registerAddress);
        i2c_buffer[1] = lowByte(registerAddress);

        // TODO write a subsection of the buffer rather than byte wise
        for (uint16_t x = 0; x < len; x++)
            i2c_buffer[x + 2] = (buffer[startSpot + x]); // Write a portion of the payload to the bus

        //printf("Write Buf @ %x, size %ld\n", _i2cPort->get_address(), len + 2);
        i2cError = _i2cPort->write_buf(i2c_buffer, len + 2); // Release bus because we are writing the address each time
        if (!i2cError)
            return (-1); // Sensor did not ACK

        startSpot += len; // Move the pointer forward
        bytesToSend -= len;
        registerAddress += len; // Move register address forward
    }
    return (0);
}

uint8_t SparkFun_VL53L5CX_IO::readMultipleBytes(uint16_t registerAddress, uint8_t *buffer, uint16_t bufferSize)
{
    uint8_t i2c_buffer[wireMaxPacketSize];
    uint8_t i2c_reg[2];

    // Read bytes up to max transaction size
    uint16_t bytesToReadRemaining = bufferSize;
    uint16_t offset = 0;
    while (bytesToReadRemaining > 0)
    {
        // Limit to 32 bytes or whatever the buffer limit is for given platform
        uint16_t bytesToRead = bytesToReadRemaining;
        if (bytesToRead > wireMaxPacketSize)
            bytesToRead = wireMaxPacketSize;

    	i2c_reg[0] = highByte(registerAddress);
    	i2c_reg[1] = lowByte(registerAddress);
        _i2cPort->read_buf_multi(i2c_reg, 2, i2c_buffer, bytesToRead);

        for (uint16_t x = 0; x < bytesToRead; x++)
            buffer[offset + x] = i2c_buffer[x];

        offset += bytesToRead;
        bytesToReadRemaining -= bytesToRead;
	registerAddress += bytesToRead;
    }

    return (0); // Success
}

uint8_t SparkFun_VL53L5CX_IO::readSingleByte(uint16_t registerAddress)
{
    uint8_t i2cregbuf[2], i2cvalue;
    i2cregbuf[0] = highByte(registerAddress);
    i2cregbuf[1] = lowByte(registerAddress);
    _i2cPort->read_buf_multi(i2cregbuf, 2, &i2cvalue, 1);
    //printf("Readbyte Byte @ %x, reg %x, value %x\n", _i2cPort->get_address(), registerAddress, i2cvalue);
    return i2cvalue;
}

uint8_t SparkFun_VL53L5CX_IO::writeSingleByte(uint16_t registerAddress, uint8_t const value)
{
    uint8_t i2cbuf[3];
    i2cbuf[0] = highByte(registerAddress);
    i2cbuf[1] = lowByte(registerAddress);
    i2cbuf[2] = value;
    //printf("Write Byte @ %x, reg %x, value %x\n", _i2cPort->get_address(), registerAddress, value);
    int ret = _i2cPort->write_buf(i2cbuf, 3);
    //printf("Write Byte @ %x, reg %x, value %x, Status %d\n", _i2cPort->get_address(), registerAddress, value, ret);
    return (ret ? 0 : -1);
}

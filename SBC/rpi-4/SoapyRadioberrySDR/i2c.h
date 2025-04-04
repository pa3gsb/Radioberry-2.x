/* 
	Title --- driver/i2c.hpp

	Copyright (C) 2013 Giacomo Trudu - wicker25[at]gmail[dot]com

	This file is part of Rpi-hw.

	Rpi-hw is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation version 3 of the License.

	Rpi-hw is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with Rpi-hw. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _RPI_HW_DRIVER_I2C_HPP_
#define _RPI_HW_DRIVER_I2C_HPP_

#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdexcept>

namespace rpihw { // Begin main namespace
	
namespace driver { // Begin drivers namespace

/*!
	@class i2c
	@brief Inter Integrated Circuit.
*/
class i2c {

public:

	/*!
		@brief Constructor method.
		@param[in] dev_path The device path.
		@param[in] addr The I2C slave address.
	*/
	i2c( const std::string &dev_path);
	void addr(uint8_t addr);

	//! Destructor method.
	virtual ~i2c();

	/*!
		@brief Writes data on the device.
		@param[in] data The data to write on the device.
		@param[in] size Size of the data to write.
	*/
	void write( uint8_t *data, uint8_t size );

	/*!
		@brief Reads data from the device.
		@param[in] data The buffer to store the data.
		@param[in] size Size of the data to read.
	*/
	void read( uint8_t *data, uint8_t size );

	/*!
		@brief Writes a byte data to a register on the device.
		@param[in] reg The device register.
		@param[in] data The data to write to the register.
	*/
	void writeReg8( uint8_t reg, uint8_t data );

	/*!
		@brief Reads a byte from a register on the device.
		@param[in] reg The device register.
		@return The data read from the register.
	*/
	uint8_t readReg8( uint8_t reg );

	/*!
		@brief Writes a word data to a register on the device.
		@param[in] reg The device register.
		@param[in] data The data to write to the register.
	*/
	void writeReg16( uint8_t reg, uint16_t data );

	/*!
		@brief Reads a word from a register on the device.
		@param[in] reg The device register.
		@return The data read from the register.
	*/
	uint16_t readReg16( uint8_t reg );

private:

	//! The device path.
	std::string m_dev_path;

	//! The I2C slave address.
	uint32_t m_addr;

	//! File descriptor of the device.
	int m_dev_fd;

	//! Data buffer used for I2C transmission.
	uint8_t m_buffer[3];
};

} // End of drivers namespace

} // End of main namespace


// Include inline methods 
//#include <rpi-hw/driver/i2c-inl.hpp>

#endif /* _RPI_HW_DRIVER_I2C_HPP_ */

/* 
    Title --- driver/i2c.cpp

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

#include "i2c.h"

namespace rpihw { // Begin main namespace

namespace driver { // Begin drivers namespace

i2c::i2c( const std::string &dev_path)
	: m_dev_path	( dev_path )
{

	// Open device file
	m_dev_fd = open( m_dev_path.c_str(), O_RDWR );

	if ( m_dev_fd < 0 )
		throw std::runtime_error( "(Fatal) `i2c`: can't open I2C device \n" );

}

void i2c::addr(uint8_t addr)
	{
		m_addr = addr;
		// Select slave device
		if(ioctl(m_dev_fd, I2C_SLAVE, addr) < 0)
			throw std::runtime_error( "(Fatal) `i2c`: can't select slave device \n" );
	}
	
i2c::~i2c() {

	// Close the device file
	close( m_dev_fd );
}

void
i2c::write( uint8_t *data, uint8_t size ) {

	if ( ::write( m_dev_fd, data, size ) != size )
		throw std::runtime_error( "(Fatal) `i2c`: failed to write to the bus\n" );
}

void
i2c::read( uint8_t *data, uint8_t size ) {

	if ( ::read( m_dev_fd, data, size ) != size )
		throw "(Fatal) `i2c`: failed to read from the bus\n";
}

void
i2c::writeReg8( uint8_t reg, uint8_t data ) {

	// Build the buffer to send
	m_buffer[0] = reg;
	m_buffer[1] = data;

	// Write the data on the device
	write( m_buffer, 2 );
}

uint8_t
i2c::readReg8( uint8_t reg ) {

	// Select the register on the device
	write( &reg, 1 );

	// Read the data from the device
	read( m_buffer, 1 );

	return m_buffer[0];
}

void
i2c::writeReg16( uint8_t reg, uint16_t data ) {

	// Build the buffer to send
	m_buffer[0] = reg;
	m_buffer[1] = data & 0xFF;
	m_buffer[2] = (data >> 8) & 0xFF;

	// Write the data on the device
	write( m_buffer, 2 );
}

uint16_t
i2c::readReg16( uint8_t reg ) {

	// Select the register on the device
	write( &reg, 1 );

	// Read the data from the device
	read( m_buffer, 2 );

	// Merge the 16 bit data
	return (uint16_t) m_buffer[0] | ( (uint16_t) m_buffer[1] << 8 );
}

} // End of drivers namespace

} // End of main namespace



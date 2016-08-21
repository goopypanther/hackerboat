/******************************************************************************
 * Hackerboat Beaglebone I2C module
 * hal/drivers/i2c.hpp
 * This module provides an interface for I2C
 * see the Hackerboat documentation for more details
 * Code is derived from the Arduino Wire library and Derek Molloy's I2C tutorial
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef I2C_H
#define I2C_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "hal/config.h"
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

class i2cClass {
	public:
		i2cClass () = default;							
		i2cClass (int bus);						/**< Creates an i2cClass object pointed at the numbered I2C bus */ 
		bool setBus (int bus);					/**< Set the bus to the desired bus. Numbers other than 1, 2, or 3 are invalid and will return false. */
		bool open (uint8_t address);			/**< Open a connection to the given address. Returns false if this fails. */
		int write (std::ostringstream output);	/**< Write the given output stream to the open device. Returns the number of bytes written, or -1 on any error. */
		int read (std::istringstream response, int minBytes = 0);	/**< Reads the response. Returns the number of bytes read. Returns -1 if there's an error and 0 if less than minBytes were returned */
		bool close ();							/**< Close the current connection. */
		bool isOpen ();							/**< returns true if the connection is open */
		
	private:
		int 		i2cfile;	/**< File handle of the open device */
		std::string devName;	/**< Name of the device file. */
};

#endif

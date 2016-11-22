/******************************************************************************
 * Hackerboat RC input module
 * RCinput.cpp
 * This module reads incoming data over the S.BUS input.
 * Operational code lifted from https://github.com/zendes/SBUS
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <atomic>
#include <thread>
#include <vector>
#include <inttypes.h>
#include <cmath>
#include "enumdefs.hpp"
#include "enumtable.hpp"
#include "hal/config.h"
#include "hal/inputThread.hpp"
#include "hal/RCinput.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>

// nicked from Arduino and changed to take doubles
double RCInput::map(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

RCInput::RCInput (std::string devpath) : 
	_path(devpath) {
		rawChannels.assign(RC_CHANNEL_COUNT, 0);
		period = RC_READ_PERIOD;
	}

int RCInput::getThrottle () {
	return round(map(static_cast<double>(rawChannels[RC_THROTTLE_CH]), RC_MIN, RC_MAX, THROTTLE_MIN, THROTTLE_MAX));
}

double RCInput::getRudder () {
	return map(static_cast<double>(rawChannels[RC_RUDDER_CH]), RC_MIN, RC_MAX, RUDDER_MIN, RUDDER_MAX); 
}

double RCInput::getCourse () {
	return map(static_cast<double>(rawChannels[RC_COURSE_SELECTOR]), RC_MIN, RC_MAX, COURSE_MIN, COURSE_MAX); 
}

int RCInput::getChannel (int channel) {
	return rawChannels[channel];
}

RCModeEnum RCInput::getMode() {
	if (this->isFailSafe()) return RCModeEnum::FAILSAFE;
	if (this->getChannel(RC_MODE_SWITCH) < (RC_MIDDLE_POSN-100)) return RCModeEnum::RUDDER;
	if (this->getChannel(RC_MODE_SWITCH) > (RC_MIDDLE_POSN+100)) return RCModeEnum::COURSE;
	return RCModeEnum::IDLE;
}

bool RCInput::begin() {
	// the serial port is opened in a pretty distinctly C-ish way. 
	struct termios2 attrib;
	devFD = open(_path.c_str(), O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (devFD < 0) return false;
	if (ioctl(devFD, TCGETS2, &attrib) < 0) return false;

	// These commands set the serial speed to 100 kbps
	attrib.c_cflag &= ~CBAUD;
	attrib.c_cflag |= BOTHER;
	attrib.c_ispeed = 100000;
	attrib.c_ospeed = 100000;
	
	// set the port to raw mode
	attrib.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	attrib.c_cflag &= ~(CSIZE);	
	attrib.c_cflag |= (PARENB | CSTOPB);		// even parity, two stop bits
	attrib.c_cflag |= (CLOCAL | CREAD | CS8);	// ignore modem status lines, enable receiver, 8 bits per byte
	attrib.c_iflag &= ~(IXON | IXOFF | IXANY);	// turn off all flow control
	attrib.c_iflag &= ~(ICRNL);					// turn off line ending translation					
	attrib.c_oflag &= ~(OPOST);					// turn off post processing of output
	attrib.c_cc[VMIN] = 0;						// this sets the timeouts for the read() operation to minimum
	attrib.c_cc[VTIME] = 1;
	if (ioctl(devFD, TCSETS2, &attrib) < 0) return false;
	
	// fire off the thread
	this->myThread = new std::thread (InputThread::InputThreadRunner(this));
	myThread->detach();
	return true;
}

bool RCInput::execute() {
	if (!lock && (!lock.try_lock_for(RC_LOCK_TIMEOUT))) return false;
	if (devFD < 0) return false;
	char buf[SBUS_BUF_LEN];
	ssize_t bytesRead = read(devFD, buf, SBUS_BUF_LEN);
	if (bytesRead > 0) {
		for (int i = 0; i < bytesRead; i++) inbuf.push_back(buf[i]); // we have to do this in order to copy /0 correctly
		memset(buf, 0, SBUS_BUF_LEN);
	}
	if (inbuf.size() >= SBUS_BUF_LEN) {
		if ((inbuf[0] != SBUS_STARTBYTE) || (inbuf[(SBUS_BUF_LEN - 1)] != SBUS_ENDBYTE)) {
			inbuf.clear();
			_errorFrames++;
			_valid = false;
			return false;
		} else {
			_goodFrames++;
			_valid = true;
			setLastInputTime();

			rawChannels[0]  = ((inbuf[1]    |inbuf[2]<<8)					& 0x07FF);
			rawChannels[1]  = ((inbuf[2]>>3 |inbuf[3]<<5)					& 0x07FF);
			rawChannels[2]  = ((inbuf[3]>>6 |inbuf[4]<<2 |inbuf[5]<<10)		& 0x07FF);
			rawChannels[3]  = ((inbuf[5]>>1 |inbuf[6]<<7)					& 0x07FF);
			rawChannels[4]  = ((inbuf[6]>>4 |inbuf[7]<<4)					& 0x07FF);
			rawChannels[5]  = ((inbuf[7]>>7 |inbuf[8]<<1 |inbuf[9]<<9)		& 0x07FF);
			rawChannels[6]  = ((inbuf[9]>>2 |inbuf[10]<<6)					& 0x07FF);
			rawChannels[7]  = ((inbuf[10]>>5|inbuf[11]<<3)                & 0x07FF);
			rawChannels[8]  = ((inbuf[12]   |inbuf[13]<<8)                & 0x07FF);
			rawChannels[9]  = ((inbuf[13]>>3|inbuf[14]<<5)                & 0x07FF);
			rawChannels[10] = ((inbuf[14]>>6|inbuf[15]<<2|inbuf[16]<<10) & 0x07FF);
			rawChannels[11] = ((inbuf[16]>>1|inbuf[17]<<7)                & 0x07FF);
			rawChannels[12] = ((inbuf[17]>>4|inbuf[18]<<4)                & 0x07FF);
			rawChannels[13] = ((inbuf[18]>>7|inbuf[19]<<1|inbuf[20]<<9)  & 0x07FF);
			rawChannels[14] = ((inbuf[20]>>2|inbuf[21]<<6)                & 0x07FF);
			rawChannels[15] = ((inbuf[21]>>5|inbuf[22]<<3)                & 0x07FF);

			((inbuf[23])      & 0x0001) ? rawChannels[16] = 2047: rawChannels[16] = 0;
			((inbuf[23] >> 1) & 0x0001) ? rawChannels[17] = 2047: rawChannels[17] = 0;

			if ((inbuf[23] >> 3) & 0x0001) {
				failsafe = true;
			} else {
				failsafe = false;
			}
			inbuf.clear();			
		}
	}
	return true;
}

RCInput::~RCInput() {
	this->kill();
	close(devFD);
	//if (myThread) delete myThread;
}
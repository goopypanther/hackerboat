/******************************************************************************
 * Hackerboat Beaglebone configuration
 * configuration.hpp
 * This is a library for loading the configuration from a json file
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Apr 2017
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include "configuration.hpp"
#include <string>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <tuple>
#include <vector>
#include "util.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/filereadstream.h"
#include "private-config.h"

using namespace std;

int Conf::Fetch(const string& name, string& target) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsString()) {
		target = d[name.c_str()].GetString();
		return 1;
	}
	return 0;
}

int Conf::Fetch(const string& name, unsigned int& target) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsUint()) {
		target = d[name.c_str()].GetUint();
		return 1;
	}
	return 0;
}

int Conf::Fetch(const string& name, uint8_t& target) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsUint()) {
		unsigned int tmp = d[name.c_str()].GetUint();
		if (tmp > 255) return 0;
		target = tmp;
		return 1;
	}
	return 0;
}

int Conf::Fetch(const string& name, float& target) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsFloat()) {
		target = d[name.c_str()].GetFloat();
		return 1;
	}
	return 0;
}

int Conf::Fetch(const string& name, int& target) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsInt()) {
		target = d[name.c_str()].GetInt();
		return 1;
	}
	return 0;
}

int Conf::Fetch(const string& name, sysdur& duration) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsString()) {
		if (parseDuration(d[name.c_str()].GetString(), duration)) return 1;
	}
	return 0;
}

int Conf::Fetch(const string& name, Value& v) {
	if (d.HasMember(name.c_str()) && d[name.c_str()].IsObject()) {
		v = d[name.c_str()].GetObject();
		return 1;
	}
	return 0;
}

Conf::Conf () {
	_gpsdAddress 		= "127.0.0.1";
	_gpsdPort			= (3001);
	_adcUpperAddress	= (0x1f);
	_adcLowerAddress	= (0x1d);
	_adcI2Cbus			= (2);
	_imuI2Cbus			= (1);
	_throttleMax	 	= (5);
	_throttleMin	 	= (-5);
	_rudderMax			= (100);
	_rudderMin			= (-100);
	_rudderDir			= (-1);
	_rudderPeriod		= (100);
	_courseMax			= (360.0);
	_courseMin			= (0.0);
	_lightCount			= (72);
	_aisMaxTime			= (600s);
	_configPinPath		= "/usr/local/bin/config-pin";
	_disarmInputPort	= (8);
	_disarmInputPin		= (22);
	_armInputPort 		= (8);
	_armInputPin 		= (20);
	_servoEnbPort 		= (8);
	_servoEnbPin 		= (19);
	_rudderPort 		= (9);
	_rudderPin			= (16);
	_pidKp				= (10.0);
	_pidKi 				= (0.1);
	_pidKd				= (0.0);
	_gpsBufSize			= (2000);
	_gpsAvgLen			= (10);
	_imuMagOffset		= {330, -86, 386};
	_imuMagScale		= {0.15835, 0.18519, 0.18149};
	_RCchannelMap 		= {	{"throttle", (0)},
							{"rudder", (3)},
							{"auto", (4)},
							{"mode", (5)},
							{"courseSelect", (6)},
							{"horn", (7)},
							{"channelCount", (18)} };
	_RClimits			= {	{"min", (171)},
							{"max", (1811)},
							{"middlePosn", (991)},
							{"middleTol", (200)} }; 						
	_RCserialPath		= "/dev/ttyS5";
	_autoDefaultThrottle = (5);
	_autoWaypointTol	= (50.0);
	_anchorDeadband		= (5.0);
	_anchorThrottleGain	= (1.0);
	_hornTime 			= (2s);
	_disarmPulseLen		= (50ms);
	_armPulseLen		= (50ms);
	_imuReadPeriod		= (10ms);
	_adcReadPeriod		= (10ms);
	_rcReadPeriod		= (8ms);
	_gpsReadPeriod		= (20ms);
	_startBatMinVolt	= (12.0);
	_lowBatCutoffVolt 	= (10.0);
	_adcUpperChanList	= {"RED", "DIR", "YLWWHT", "REDWHT", "YLW", "WHT", "DISARM", "ENABLE"};
	_adcLowerChanList	= {"HORN", "mot_i", "mot_v", "charge_v", "charge_i", "aux_0", "aux_1", "servo_i"};
	_adcExternRefVolt	= (5.0);
	_batmonPath			= "/sys/devices/platform/ocp/44e0d000.tscadc/TI-am335x-adc/iio:device0/in_voltage1_raw";
	_batmonName			= "battery_mon";
	_restConf			= {	{"key_header", "X-AIO-Key:"},
							{"host", "https://io.adafruit.com/api/v2/"},
							{"username", restUsername},
							{"key", restKey},
							{"group", ""},
							{"datatype", "json"} };
	_restPubPeriod		= (1000ms);
	_restSubPeriod		= (200ms);
	_restDelay			= (1ms);
	_restTimeout		= (200ms);
	_restMaxBuf			= (50000);
	_restMaxCount		= (5000);
	_wdFile				= "/tmp/watchdog";
	_wdTimeout			= (30s);
	_relayInit			= { { "RED", { "RED", 8, 3, 8, 4 } },
							{ "DIR", { "DIR", 8, 5, 8, 6 } }, 
							{ "YLWWHT", { "YLWWHT", 8, 7, 8, 8 } }, 
							{ "REDWHT", { "REDWHT", 8, 9, 8, 10 } }, 
							{ "YLW", { "YLW", 8, 11, 8, 12 } }, 
							{ "WHT", { "WHT", 8, 13, 8, 14 } }, 
							{ "DISARM", { "DISARM", 8, 15, 8, 16 } }, 
							{ "HORN", { "HORN", 8, 17, 8, 18 } }, 
							{ "ENABLE", { "ENABLE", 8, 24, 8, 26 } } };
	_RCchannelCount		= (18);
	_aisMaxDistance		= (10000);
	_selfTestDelay		= (30s);
}

int Conf::load (const string& file) {
	FILE* fp = fopen(file.c_str(), "r");
	if (fp != nullptr) {
		confFilePath = file;
	} else return 0;
	char readbuf[65536];
	FileReadStream is(fp, readbuf, sizeof(readbuf));
	d.ParseStream(is);
	Value v;

	int result = 0;

	result += Fetch("GPSd Address", _gpsdAddress);
	result += Fetch("GPSd Port", _gpsdPort);
	result += Fetch("ADC Upper Address", _adcUpperAddress);
	result += Fetch("ADC Lower Address", _adcLowerAddress);
	result += Fetch("ADC I2C Bus", _adcI2Cbus);
	result += Fetch("IMU I2C Bus", _imuI2Cbus);
	result += Fetch("Throttle Max", _throttleMax);
	result += Fetch("Throttle Min", _throttleMin);
	result += Fetch("Rudder Max", _rudderMax);
	result += Fetch("Rudder Min", _rudderMin);
	result += Fetch("Rudder Direction", _rudderDir);
	result += Fetch("Rudder Period", _rudderPeriod);
	result += Fetch("Course Max", _courseMax);
	result += Fetch("Course Min", _courseMin);
	result += Fetch("Light Count", _lightCount);
	result += Fetch("AIS Max Time", _aisMaxTime);
	result += Fetch("Config-Pin Path", _configPinPath);
	result += Fetch("Disarm Input Port", _disarmInputPort);
	result += Fetch("Disarm Input Pin", _disarmInputPin);
	result += Fetch("Arm Input Port", _armInputPort);
	result += Fetch("Arm Input Pin", _armInputPin);
	result += Fetch("Servo Enable Port", _servoEnbPort);
	result += Fetch("Servo Enable Pin", _servoEnbPin);
	result += Fetch("Rudder Port", _rudderPort);
	result += Fetch("Rudder Pin", _rudderPin);
	result += Fetch("PID Kp", _pidKp);
	result += Fetch("PID Ki", _pidKi);
	result += Fetch("PID Kd", _pidKd);
	result += Fetch("GPS Buf Size", _gpsBufSize);
	result += Fetch("GPS Avg Length", _gpsAvgLen);
	result += Fetch("RC Serial Path", _RCserialPath);
	result += Fetch("Default Throttle", _autoDefaultThrottle);
	result += Fetch("Waypoint Tolerance", _autoWaypointTol);
	result += Fetch("Anchor Deadband", _anchorDeadband);
	result += Fetch("Anchor Throttle Gain", _anchorThrottleGain);
	result += Fetch("Horn Time", _hornTime);
	result += Fetch("Disarm Pulse Length", _disarmPulseLen);
	result += Fetch("Arm Pulse Length", _armPulseLen);
	result += Fetch("IMU Read Period", _imuReadPeriod);
	result += Fetch("RC Read Period", _rcReadPeriod);
	result += Fetch("ADC Read Period", _adcReadPeriod);
	result += Fetch("GPS Read Period", _gpsReadPeriod);
	result += Fetch("Min Starting Battery Voltage", _startBatMinVolt);
	result += Fetch("Low Battery Cutoff Voltage", _lowBatCutoffVolt);
	result += Fetch("ADC External Reference Voltage", _adcExternRefVolt);
	result += Fetch("Battery Monitor Path", _batmonPath);
	result += Fetch("Battery Monitor Name", _batmonName);
	result += Fetch("REST Subscription Period", _restSubPeriod);
	result += Fetch("REST Publication Period", _restPubPeriod);
	result += Fetch("REST Delay", _restDelay);
	result += Fetch("REST Timeout", _restTimeout);
	result += Fetch("REST Max Buffer Size", _restMaxBuf);
	result += Fetch("REST Max Count", _restMaxCount);
	result += Fetch("Watchdog File", _wdFile);
	result += Fetch("Watchdog Timeout", _wdTimeout);
	result += Fetch("RC Channel Count", _RCchannelCount);
	result += Fetch("AIS Max Distance", _aisMaxDistance);
	result += Fetch("Self Test Period", _selfTestDelay);
	if (Fetch("IMU Magnetic Offset", v) && v.IsArray() && (v.Size() >= 3)) {
		_imuMagOffset = make_tuple(v[0].GetInt(), v[1].GetInt(), v[2].GetInt());
		result++;
	}
	if (Fetch("IMU Magnetic Scale", v) && v.IsArray() && (v.Size() >= 3)) {
		_imuMagScale = make_tuple(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat());
		result++;
	}
	if (Fetch("RC Channel Map", v) && v.IsObject()) {
		for (auto& itr : v.GetObject()) {
			if (itr.value.IsUint()) {
				_RCchannelMap.emplace((itr.name.GetString()),
										itr.value.GetUint());
			}	
		}
		result++;
	}
	if (Fetch("RC Limits", v) && v.IsObject()) {
		for (auto& itr : v.GetObject()) {
			if (itr.value.IsUint()) {
				_RClimits.emplace((itr.name.GetString()),
									itr.value.GetUint());
			}	
		}
		result++;
	}
	if (Fetch("ADC Upper Channel List", v) && v.IsArray()) {
		for (auto& itr : v.GetArray()) {
			if (itr.IsString()) _adcUpperChanList.emplace_back(itr.GetString());
		}
		result++;
	}
	if (Fetch("ADC Lower Channel List", v) && v.IsArray()) {
		for (auto& itr : v.GetArray()) {
			if (itr.IsString()) _adcLowerChanList.emplace_back(itr.GetString());
		}
		result++;
	}
	if (Fetch("REST Configuration", v) && v.IsObject()) {
		for (auto& itr : v.GetObject()) {
			if (itr.value.IsString()) {
				_restConf.emplace((itr.name.GetString()),
									itr.value.GetString());
			}	
		}
		result++;
	}

	return result;
}

bool Conf::parseDuration (const string& dur, sysdur& output) {
	long double mydur = stold(dur);
	if (dur.find("h") != std::string::npos) {
		output = duration_cast<sysdur>(duration<long double>(mydur/3600));
	} else if (dur.find("min") != std::string::npos) {
		output = duration_cast<sysdur>(duration<long double>(mydur/60));
	} else if (dur.find("ms") != std::string::npos) {
		output = duration_cast<sysdur>(duration<long double, milli>(mydur));
	} else if (dur.find("us") != std::string::npos) {
		output = duration_cast<sysdur>(duration<long double, micro>(mydur));
	} else if (dur.find("ns") != std::string::npos) {
		output = duration_cast<sysdur>(duration<long double, nano>(mydur));
	} else if (dur.find("s") != std::string::npos) {
		output = duration_cast<sysdur>(duration<long double>(mydur));
	} else return false;
	return true;
}

Conf* Conf::_instance = new Conf();
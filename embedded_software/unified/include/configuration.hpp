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

#include <string>
#include <list>
#include <map>
#include <chrono>
#include <iostream>
#include <tuple>
#include <vector>
#include "util.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

using namespace rapidjson;
using namespace std;
using namespace std::chrono;

typedef chrono::system_clock::duration sysdur;
typedef tuple<string, uint8_t, uint8_t, uint8_t, uint8_t> RelaySpec;

class Conf {
	public:
		int load(const string& file);
		int load () {return load(confFilePath);};
		static Conf* get() {return _instance;};

		inline const string& 		gpsdAddress () 			{return _gpsdAddress;};
		inline const unsigned int&  gpsdPort () 			{return _gpsdPort;};
		inline const uint8_t& 		adcUpperAddress () 		{return _adcUpperAddress;};
		inline const uint8_t& 		adcLowerAddress () 		{return _adcLowerAddress;};
		inline const uint8_t& 		adcI2Cbus () 			{return _adcI2Cbus;};
		inline const uint8_t& 		imuI2Cbus () 			{return _imuI2Cbus;};
		inline const float& 		throttleMax () 			{return _throttleMax;};
		inline const float& 		throttleMin () 			{return _throttleMin;};
		inline const int&  			rudderMax () 			{return _rudderMax;};
		inline const int&  			rudderMin () 			{return _rudderMin;};
		inline const uint8_t& 		rudderDir () 			{return _rudderDir;};
		inline const int&  			rudderPeriod () 		{return _rudderPeriod;};
		inline const float& 		courseMax () 			{return _courseMax;};
		inline const float&  		courseMin () 			{return _courseMin;};
		inline const int&  			lightCount () 			{return _lightCount;};
		inline const sysdur&  		aisMaxTime () 			{return _aisMaxTime;};
		inline const string& 		configPinPath () 		{return _configPinPath;};
		inline const int&  			disarmInputPort () 		{return _disarmInputPort;};
		inline const int&  			disarmInputPin () 		{return _disarmInputPin;};
		inline const int&  			armInputPort () 		{return _armInputPort;};
		inline const int&  			armInputPin () 			{return _armInputPin;};
		inline const int&  			servoEnbPort () 		{return _servoEnbPort;};
		inline const int&  			servoEnbPin () 			{return _servoEnbPin;};
		inline const int&  			rudderPort () 			{return _rudderPort;};
		inline const int&  			rudderPin () 			{return _rudderPin;};
		inline const float& 		pidKp () 				{return _pidKp;};
		inline const float& 		pidKi () 				{return _pidKi;};
		inline const float& 		pidKd () 				{return _pidKd;};
		inline const int&  			gpsBufSize () 			{return _gpsBufSize;};
		inline const unsigned int&	gpsAvgLen () 			{return _gpsAvgLen;};
		inline const tuple<int, int, int>& 	imuMagOffset () {return _imuMagOffset;};
		inline const tuple<float, float, float>&  imuMagScale () 	{return _imuMagScale;};
		inline const map<string, int>&  RCchannelMap () 	{return _RCchannelMap;};
		inline const map<string, int>&  RClimits () 		{return _RClimits;};
		inline const string& 		RCserialPath () 		{return _RCserialPath;};
		inline const int&  			autoDefaultThrottle () 	{return _autoDefaultThrottle;};
		inline const float& 		autoWaypointTol () 		{return _autoWaypointTol;};
		inline const float& 		anchorDeadband () 		{return _anchorDeadband;};
		inline const float& 		anchorThrottleGain () 	{return _anchorThrottleGain;};
		inline const sysdur&   		hornTime () 			{return _hornTime;};
		inline const sysdur&  		disarmPulseLen () 		{return _disarmPulseLen;};
		inline const sysdur&  		armPulseLen () 			{return _armPulseLen;};
		inline const sysdur&  		imuReadPeriod () 		{return _imuReadPeriod;};
		inline const sysdur&  		adcReadPeriod () 		{return _adcReadPeriod;};
		inline const sysdur&  		rcReadPeriod () 		{return _rcReadPeriod;};
		inline const sysdur&  		gpsReadPeriod () 		{return _gpsReadPeriod;};
		inline const float&  		startBatMinVolt () 		{return _startBatMinVolt;};
		inline const float&  		lowBatCutoffVolt () 	{return _lowBatCutoffVolt;};
		inline const vector<string>&	adcUpperChanList () {return _adcUpperChanList;};
		inline const vector<string>& 	adcLowerChanList () {return _adcLowerChanList;};
		inline const float& 		adcExternRefVolt ()		{return _adcExternRefVolt;};
		inline const string& 		batmonPath () 			{return _batmonPath;};
		inline const string& 		batmonName () 			{return _batmonName;};
		inline const map<string, string>&  	restConf () 	{return _restConf;};
		inline const sysdur& 		restPubPeriod () 		{return _restPubPeriod;};
		inline const sysdur& 		restSubPeriod () 		{return _restSubPeriod;};
		inline const sysdur& 		restDelay () 			{return _restDelay;};
		inline const sysdur& 		restTimeout () 			{return _restTimeout;};
		inline const int&  			restMaxBuf () 			{return _restMaxBuf;};
		inline const int&  			restMaxCount () 		{return _restMaxCount;};
		inline const string& 		wdFile () 				{return _wdFile;};
		inline const sysdur& 		wdTimeout () 			{return _wdTimeout;};
		inline const unsigned int&	RCchannelCount ()		{return _RCchannelCount;};
		inline const map<string, RelaySpec>& 	relayInit()	{return _relayInit;};
		inline const unsigned int&	aisMaxDistance ()		{return _aisMaxDistance;};
		inline const sysdur&		selfTestDelay ()		{return _selfTestDelay;};

	private:
		Conf ();						
		Conf (Conf const&) = delete;					/**< Hark, a singleton! */
		Conf& operator=(Conf const&) = delete;			/**< Hark, a singleton! */
		static Conf *_instance;
		bool parseDuration (const string& dur, sysdur& duration);

		int Fetch(const string& name, string& target);
		int Fetch(const string& name, unsigned int& target);
		int Fetch(const string& name, uint8_t& target);
		int Fetch(const string& name, float& target);
		int Fetch(const string& name, int& target);
		int Fetch(const string& name, sysdur& target);
		int Fetch(const string& name, Value& target);

		Document d;
		string			confFilePath		= "/home/debian/hackerboat/embedded_software/unified/setup/hackerboatconf.json";

		string 			_gpsdAddress;
		unsigned int 	_gpsdPort;
		uint8_t			_adcUpperAddress;
		uint8_t			_adcLowerAddress;
		uint8_t			_adcI2Cbus;
		uint8_t			_imuI2Cbus;
		float			_throttleMax;
		float			_throttleMin;
		int 			_rudderMax;
		int 			_rudderMin;
		uint8_t			_rudderDir;
		int 			_rudderPeriod;
		float			_courseMax;
		float 			_courseMin;
		int 			_lightCount;
		sysdur 			_aisMaxTime;
		string 			_configPinPath;
		int 			_disarmInputPort;
		int 			_disarmInputPin;
		int 			_armInputPort;
		int 			_armInputPin;
		int 			_servoEnbPort;
		int 			_servoEnbPin;
		int 			_rudderPort;
		int 			_rudderPin;
		float			_pidKp;
		float			_pidKi;
		float			_pidKd;
		int 			_gpsBufSize;
		unsigned int 	_gpsAvgLen;
		tuple<int, int, int> 		_imuMagOffset;
		tuple<float, float, float> 	_imuMagScale;
		map<string, int> _RCchannelMap;
		map<string, int> _RClimits;
		string			_RCserialPath;
		int 			_autoDefaultThrottle;
		float			_autoWaypointTol;
		float			_anchorDeadband;
		float			_anchorThrottleGain;
		sysdur  		_hornTime;
		sysdur 			_disarmPulseLen;
		sysdur 			_armPulseLen;
		sysdur 			_imuReadPeriod;
		sysdur 			_adcReadPeriod;
		sysdur 			_rcReadPeriod;
		sysdur 			_gpsReadPeriod;
		float 			_startBatMinVolt;
		float 			_lowBatCutoffVolt;
		vector<string>	_adcUpperChanList;
		vector<string>	_adcLowerChanList;
		float			_adcExternRefVolt;
		string			_batmonPath;
		string 			_batmonName;
		map<string, string> _restConf;
		sysdur			_restPubPeriod;
		sysdur			_restSubPeriod;
		sysdur			_restDelay;
		sysdur			_restTimeout;
		int 			_restMaxBuf;
		int 			_restMaxCount;
		string			_wdFile;
		sysdur			_wdTimeout;
		map<string, RelaySpec>	_relayInit;
		unsigned int 	_RCchannelCount;
		unsigned int 	_aisMaxDistance;
		sysdur			_selfTestDelay;
};

#endif /* CONFIGURATION_H */